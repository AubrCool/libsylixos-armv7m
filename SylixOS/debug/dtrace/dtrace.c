/*********************************************************************************************************
**
**                                    �й������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: dtrace.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2011 �� 11 �� 18 ��
**
** ��        ��: SylixOS ���Ը�����, GDB server ����ʹ�ô˵��Ը���������װ�ص�ģ����߽���.

** BUG:
2012.09.05  �����賿, ������� dtrace �ĵȴ���ӿڻ���, ��ʼΪ GDB server �ı�дɨƽһ���ϰ�.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
#include "signal.h"
/*********************************************************************************************************
  dtrace ����ԭ��
  
  ÿһ�� DTRACE_NODE ������һ�� ptrace �ڵ������Ϣ. GDB server ���ȴ���һ�� dtrace �ڵ�, Ȼ����Դ����ϵ�
  
  ����һ���ϵ�����ó������е�ָ����λ��(һ����ַ)ʱ�����쳣, ����, ͨ�� dtrace_read �Ƚ�ԭʼ��ָ�������,
  
  Ȼ������ dtrace_write д��һ�����Ե����쳣��ָ��, ���������е�����, �ͻ�����쳣. ����ϵͳ����һ����ÿ��
  
  �����˵Ľڵ�, Ȼ����� DTRACE_pfuncTrap() ���ж�����һ�� dtrace �ڵ㴴���Ľڵ� (�� GDB server ʵ��), 
  
  ����ҵ�, GDB server Ӧ���жϴ˶ϵ��Ƿ���Ч, �����Ч, ��Ӧ��¼�����Ϣ, Ȼ�󷵻� 0, DTRACE_pfuncTrap() 
  
  ���� 0 ˵������һ�������Ķϵ�, ��ʱ, dtrace_trap() �Ὣ "�ϵ�" ���߳�ֹͣ, ������ GDB server ���� SIGTRAP
  
  �ź�, Ȼ�� GDB server ��ȡ�ղ� DTRACE_pfuncTrap() ��¼����Ϣ, ��֪ͨ gdb ���¼�����. 
  
  ע��: ����쳣�������������� (������ vmm.h ��), GDB server ����Ҫ�� gdb ���ʵ�����Ϣ����!
  
  ���ϵ�ִ�������Ҫ����ִ��ʱ, GDB server ������Ҫ��ԭ�ϵ��ָ��, ����һ�λ�൥��, Ȼ�����ڸղŵ�λ������
  
  �ϵ㲢�����ǰ�ϵ�, Ȼ����ִ��, �����ϵ�ṹ֧��Ӳ������, ����: x86 PowerPC �ȴ�����, ��ʹ��Ӳ����������, 
  
  �����֧���� ARM MIPS ����Ҫ GDB server �����������������, (��Ҫ��������ķ�֧Ԥ��)
  
  ע��: ���ͳһ�ϵ����˶���߳�, DTRACE_pfuncTrap() ���ᱻ��ε���! ��ֻҪ GDB server ���� 
  
  dtrace_continue() ��ʹ���е��̼߳���ִ��.
*********************************************************************************************************/

/*********************************************************************************************************
  dtrace �ṹ
  
  ������Ϣ�ɲ���ϵͳ�쳣�����������д
  DTRACE_ulTraceThread
  DTRACE_pvFrame
  DTRACE_ulAddress
*********************************************************************************************************/
typedef struct {
    LW_LIST_LINE        DTRACE_lineManage;                              /*  ��������                    */
    LW_OBJECT_HANDLE    DTRACE_ulHostThread;                            /*  �����߳�                    */
    
    BOOL              (*DTRACE_pfuncTrap)(PVOID  pvArg, 
                                          PVOID  pvFrame, 
                                          addr_t ulAddress, 
                                          ULONG  ulType,
                                          LW_OBJECT_HANDLE ulThread);   /*  Ӳ���쳣����                */
    PVOID               DTRACE_pvArg;                                   /*  DTRACE_pfuncIsTrap ����     */
    
    LW_OBJECT_HANDLE    DTRACE_ulStopSem;                               /*  ���б��ϵ���ͣ���߳��ź���  */
} DTRACE_NODE;
typedef DTRACE_NODE    *PDTRACE_NODE;
/*********************************************************************************************************
  dtrace ȫ�ֱ���
*********************************************************************************************************/
static LW_LIST_LINE_HEADER      _G_plineDtraceHeader;
static LW_OBJECT_HANDLE         _G_ulDtraceLock;

#define __DTRACE_LOCK()         API_SemaphoreMPend(_G_ulDtraceLock, LW_OPTION_WAIT_INFINITE)
#define __DTRACE_UNLOCK()       API_SemaphoreMPost(_G_ulDtraceLock)
/*********************************************************************************************************
** ��������: dtrace_create
** ��������: ����һ�� dtrace ���Խڵ�
** �䡡��  : pfuncTrap         �ϵ����뺯��
**           pvArg             �ϵ����뺯������
** �䡡��  : dtrace �ڵ�
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
PVOID dtrace_create (BOOL (*pfuncTrap)(), PVOID pvArg)
{
    PDTRACE_NODE    pdtrace;

    if (pfuncTrap == LW_NULL) {
        _ErrorHandle(EINVAL);
        return  (LW_NULL);
    }
    
    if (_G_ulDtraceLock == LW_OBJECT_HANDLE_INVALID) {
        _G_ulDtraceLock =  API_SemaphoreMCreate("dtrace_lock", 
                                                LW_PRIO_DEF_CEILING, 
                                                LW_OPTION_INHERIT_PRIORITY | 
                                                LW_OPTION_DELETE_SAFE |
                                                LW_OPTION_OBJECT_GLOBAL, 
                                                LW_NULL);
    }
    
    pdtrace = (PDTRACE_NODE)__SHEAP_ALLOC(sizeof(DTRACE_NODE));
    if (pdtrace == LW_NULL) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "system heap is low memory.\r\n");
        _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);
        return  (LW_NULL);
    }
    lib_bzero(pdtrace, sizeof(DTRACE_NODE));
    
    pdtrace->DTRACE_ulHostThread = API_ThreadIdSelf();
    pdtrace->DTRACE_pfuncTrap    = pfuncTrap;
    pdtrace->DTRACE_pvArg        = pvArg;
    
    /*
     *  DTRACE_ulStopSem �ź����ɱ��źŴ�ϻָ�ִ�� (����ʹ�� FIFO �͵ȴ�)
     */
    pdtrace->DTRACE_ulStopSem = API_SemaphoreBCreate("dtrace_stopsem", LW_FALSE, 
                                                     LW_OPTION_SIGNAL_INTER | 
                                                     LW_OPTION_WAIT_FIFO |
                                                     LW_OPTION_OBJECT_GLOBAL, 
                                                     LW_NULL);
    if (pdtrace->DTRACE_ulStopSem == LW_OBJECT_HANDLE_INVALID) {
        __SHEAP_FREE(pdtrace);
        return  (LW_NULL);
    }
    
    __DTRACE_LOCK();
    _List_Line_Add_Ahead(&pdtrace->DTRACE_lineManage, &_G_plineDtraceHeader);
    __DTRACE_UNLOCK();
    
    return  ((PVOID)pdtrace);
}
/*********************************************************************************************************
** ��������: dtrace_delete
** ��������: ɾ��һ�� dtrace �ڵ�
** �䡡��  : pvDtrace      dtrace �ڵ�
** �䡡��  : ERROR or NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
INT dtrace_delete (PVOID pvDtrace)
{
    PDTRACE_NODE    pdtrace;

    if (pvDtrace == LW_NULL) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    pdtrace = (PDTRACE_NODE)pvDtrace;
    
    __DTRACE_LOCK();
    _List_Line_Del(&pdtrace->DTRACE_lineManage, &_G_plineDtraceHeader);
    __DTRACE_UNLOCK();
    
    API_SemaphoreBDelete(&pdtrace->DTRACE_ulStopSem);
    
    __SHEAP_FREE(pdtrace);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: dtrace_read
** ��������: dtrace ��ȡ�������߳�ָ���ڴ�����
** �䡡��  : pvDtrace      dtrace �ڵ�
**           ulAddress     ��ַ
**           pvBuffer      ��ȡ����
**           stNbytes      ��С
** �䡡��  : ERROR or NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
INT dtrace_read (PVOID pvDtrace, addr_t ulAddress, PVOID pvBuffer, size_t  stNbytes)
{
    /*
     *  ���� SylixOS ����ͳһ�ĵ�ַ�ռ�, ����, ����ֱ�ӿ�������
     */
    if (pvBuffer == LW_NULL) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    lib_memcpy(pvBuffer, (const PVOID)ulAddress, stNbytes);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: dtrace_write
** ��������: dtrace ���ñ������߳�ָ���ڴ�����
** �䡡��  : pvDtrace      dtrace �ڵ�
**           ulAddress     ��ַ
**           pvBuffer      �������
**           stNbytes      ��С
** �䡡��  : ERROR or NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
INT dtrace_write (PVOID pvDtrace, addr_t ulAddress, const PVOID pvBuffer, size_t  stNbytes)
{
    if (pvBuffer == LW_NULL) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    lib_memcpy((PVOID)ulAddress, pvBuffer, stNbytes);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: dtrace_continue
** ��������: dtrace ʹ�������̼߳���ִ��
** �䡡��  : pvDtrace      dtrace �ڵ�
** �䡡��  : ERROR or NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
INT dtrace_continue (PVOID pvDtrace)
{
    PDTRACE_NODE    pdtrace;

    if (pvDtrace == LW_NULL) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    pdtrace = (PDTRACE_NODE)pvDtrace;
    
    API_SemaphoreBFlush(pdtrace->DTRACE_ulStopSem, LW_NULL);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: dtrace_continue_one
** ��������: dtrace ʹ����ֹͣ���̼߳���ִ��
** �䡡��  : pvDtrace      dtrace �ڵ�
** �䡡��  : ERROR or NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
INT dtrace_continue_one (PVOID pvDtrace)
{
    PDTRACE_NODE    pdtrace;

    if (pvDtrace == LW_NULL) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    pdtrace = (PDTRACE_NODE)pvDtrace;
    
    API_SemaphoreBPost(pdtrace->DTRACE_ulStopSem);
    API_SemaphoreBPend(pdtrace->DTRACE_ulStopSem, LW_OPTION_NOT_WAIT);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
  һ�º���һ��Ϊ����ϵͳʹ��, ����ϵͳ���쳣ʱ������������.
*********************************************************************************************************/
/*********************************************************************************************************
** ��������: dtrace_trap
** ��������: ����ϵͳ�쳣����ʱ, ֪ͨ dtrace. (�˺��������ڱ��쳣�ж��̵߳��쳣������)
** �䡡��  : pvFrame       trap �쳣�߳̾��ջ��ָ��, ��ϵ�ṹ���
**           ulAddress     trap ��ַ
**           ulType        trap ����
**           ulThread      trap �쳣�߳̾��
** �䡡��  :  0 ��ʾ�Ƕϵ�, 
             -1 ��ʾ���Ƕϵ�.
              1 ��ʾ�ϵ��Ѿ��ָ�, ����ִ��
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
INT dtrace_trap (PVOID              pvFrame,
                 addr_t             ulAddress, 
                 ULONG              ulType,
                 LW_OBJECT_HANDLE   ulThread)
{
    INT                 iRet = PX_ERROR;
    PLW_LIST_LINE       plineTemp;
    PDTRACE_NODE        pdtrace;
    LW_OBJECT_HANDLE    ulHostThread;
    
    if (_G_ulDtraceLock == LW_OBJECT_HANDLE_INVALID) {
        _G_ulDtraceLock =  API_SemaphoreMCreate("dtrace_lock", 
                                                LW_PRIO_DEF_CEILING, 
                                                LW_OPTION_INHERIT_PRIORITY | 
                                                LW_OPTION_DELETE_SAFE |
                                                LW_OPTION_OBJECT_GLOBAL, 
                                                LW_NULL);
    }

    __DTRACE_LOCK();
    for (plineTemp  = _G_plineDtraceHeader; 
         plineTemp != LW_NULL; 
         plineTemp  = _list_line_get_next(plineTemp)) {                 /*  �������п��ƿ�              */
        
        pdtrace = _LIST_ENTRY(plineTemp, DTRACE_NODE, DTRACE_lineManage);
        
        iRet = pdtrace->DTRACE_pfuncTrap(pdtrace->DTRACE_pvArg, 
                                         pvFrame,
                                         ulAddress, 
                                         ulType, 
                                         ulThread);
        if (iRet > 0) {                                                 /*  �˶ϵ��ѻָ�, ���Լ���ִ��  */
            plineTemp = LW_NULL;
            break;
        
        } else if (iRet == 0) {                                         /*  �ϵ���Ч, ��Ҫ��ͣ��ǰ�߳�  */
            ulHostThread = pdtrace->DTRACE_ulHostThread;                /*  �����߳�                    */
            break;
        }
    }
    __DTRACE_UNLOCK();
    
    if (plineTemp == LW_NULL) {
        return  (iRet);
    }
    
    /*
     *  ���� SIGTRAP �ź�.
     */
    kill(ulHostThread, SIGTRAP);                                        /*  ֪ͨ GDB Server             */
    
    API_SemaphoreBPend(pdtrace->DTRACE_ulStopSem, 
                       LW_OPTION_WAIT_INFINITE);                        /*  �ȴ�����ִ��                */
    
    return  (iRet);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
