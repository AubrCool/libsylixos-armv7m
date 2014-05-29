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
** ��   ��   ��: powerM.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2008 �� 03 �� 06 ��
**
** ��        ��: ϵͳ�����Ĺ�����. �����û�����, �ɷ�Ϊ������Ŀ��Ƶ�, ÿ�����Ƶ���������ӳ�ʱ��.
                 ����, ϵͳ�к��� LCD, ��ô���Է�Ϊ�������Ŀ��Ƶ�. ������һ��ʱ�����˲�����, ����
                 �ر� LCD ����, �ھ���һ��ʱ�����˲�����, �ɹر� LCD ��ʾϵͳ�Խ��͹���.
                 
** BUG
2008.05.18  ʹ�� __KERNEL_ENTER() ���� ThreadLock();
2008.05.31  ʹ�� __KERNEL_MODE_...().
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
/*********************************************************************************************************
  �ü�����
*********************************************************************************************************/
#if (LW_CFG_POWERM_EN > 0) && (LW_CFG_MAX_POWERM_NODES > 0)
/*********************************************************************************************************
  ȫ�ֱ���
*********************************************************************************************************/
extern LW_CLASS_POWERM_NODE _G__pmnControlNode[LW_CFG_MAX_POWERM_NODES];
extern LW_OBJECT_HANDLE     _G_ulPowerMLock;
extern LW_CLASS_WAKEUP      _G_wuPowerM;
/*********************************************************************************************************
  �ڲ�����
*********************************************************************************************************/
PLW_CLASS_POWERM_NODE  _Allocate_PowerM_Object(VOID);
VOID                   _Free_PowerM_Object(PLW_CLASS_POWERM_NODE    p_pmnFree);
/*********************************************************************************************************
** ��������: API_PowerMCreate
** ��������: ����һ�����Ŀ��ƶ�ʱ��
** �䡡��  : 
**           pcName              ���Ŀ��Ƶ������
**           ulOption            ����ѡ��
**           pulId               Idָ��
** �䡡��  : ���Ŀ��ƶ�ʱ��
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
                                           
                                       (�������ж��е���)
*********************************************************************************************************/
LW_API  
LW_OBJECT_HANDLE  API_PowerMCreate (PCHAR            pcName,
                                    ULONG            ulOption,
                                    LW_OBJECT_ID    *pulId)
{
    REGISTER ULONG                  ulIdTemp;
    REGISTER PLW_CLASS_POWERM_NODE  p_pmnNewNode;
    
    if (LW_CPU_GET_CUR_NESTING()) {                                     /*  �������ж��е���            */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "called from ISR.\r\n");
        _ErrorHandle(ERROR_KERNEL_IN_ISR);
        return  (LW_OBJECT_HANDLE_INVALID);
    }
    
    if (_Object_Name_Invalid(pcName)) {                                 /*  ���������Ч��              */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "name too long.\r\n");
        _ErrorHandle(ERROR_KERNEL_PNAME_TOO_LONG);
        return  (LW_OBJECT_HANDLE_INVALID);
    }
    
    __KERNEL_MODE_PROC(
        p_pmnNewNode = _Allocate_PowerM_Object();                       /*  ���һ�����ƿ�              */
    );
    
    if (!p_pmnNewNode) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "powerm node full.\r\n");
        _ErrorHandle(ERROR_POWERM_FULL);
        return  (LW_OBJECT_HANDLE_INVALID);
    }
    
    if (pcName) {                                                       /*  ��������                    */
        lib_strcpy(p_pmnNewNode->PMN_cPowerMName, pcName);
    } else {
        p_pmnNewNode->PMN_cPowerMName[0] = PX_EOS;                      /*  �������                    */
    }
    
    p_pmnNewNode->PMN_ulCounter     = 0;
    p_pmnNewNode->PMN_ulCounterSave = 0;
    p_pmnNewNode->PMN_pvArg         = LW_NULL;
    p_pmnNewNode->PMN_pfuncCallback = LW_NULL;
    p_pmnNewNode->PMN_bIsUsing      = LW_TRUE;
    
    ulIdTemp = _MakeObjectId(_OBJECT_POWERM, 
                             LW_CFG_PROCESSOR_NUMBER, 
                             p_pmnNewNode->PMN_usIndex);                /*  �������� id                 */
    
    if (pulId) {
        *pulId = ulIdTemp;
    }
    
    __LW_OBJECT_CREATE_HOOK(ulIdTemp, ulOption);
    
    _DebugHandle(__LOGMESSAGE_LEVEL, "powerm node \"");
    _DebugHandle(__LOGMESSAGE_LEVEL, (pcName ? pcName : ""));
    _DebugHandle(__LOGMESSAGE_LEVEL, "\" has been create.\r\n");
    
    return  (ulIdTemp);
}
/*********************************************************************************************************
** ��������: API_PowerMDelete
** ��������: ɾ��һ�����Ŀ��ƶ�ʱ��
** �䡡��  : pulId               ���ָ��
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
                                           
                                       (�������ж��е���)
*********************************************************************************************************/
LW_API  
ULONG  API_PowerMDelete (LW_OBJECT_HANDLE   *pulId)
{
    REGISTER LW_OBJECT_HANDLE          ulId;
    REGISTER UINT16                    usIndex;
    REGISTER PLW_CLASS_POWERM_NODE     p_pmnDelNode;
    
    ulId = *pulId;
    
    usIndex = _ObjectGetIndex(ulId);
    
    if (LW_CPU_GET_CUR_NESTING()) {                                     /*  �������ж��е���            */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "called from ISR.\r\n");
        _ErrorHandle(ERROR_KERNEL_IN_ISR);
        return  (ERROR_KERNEL_IN_ISR);
    }
    
#if LW_CFG_ARG_CHK_EN > 0
    if (!_ObjectClassOK(ulId, _OBJECT_POWERM)) {                        /*  �������ͼ��                */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
    if (_PowerM_Index_Invalid(usIndex)) {                               /*  �������������              */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
#endif
    
    __POWERM_LOCK();                                                    /*  ����                        */

    p_pmnDelNode = &_G__pmnControlNode[usIndex];
    
    if (!p_pmnDelNode->PMN_bIsUsing) {
        __POWERM_UNLOCK();                                              /*  ����                        */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
    
    if (p_pmnDelNode->PMN_bInQ) {                                       /*  �Ѿ�������ɨ������          */
        _WakeupDel(&_G_wuPowerM, &p_pmnDelNode->PMN_wunTimer);
    }
    
    p_pmnDelNode->PMN_ulCounterSave = 0;
    p_pmnDelNode->PMN_bIsUsing      = LW_FALSE;
    
    _ObjectCloseId(pulId);
    
    __POWERM_UNLOCK();                                                  /*  ����                        */
    
    _DebugHandle(__LOGMESSAGE_LEVEL, "powerm node \"");
    _DebugHandle(__LOGMESSAGE_LEVEL, p_pmnDelNode->PMN_cPowerMName);
    _DebugHandle(__LOGMESSAGE_LEVEL, "\" has been delete.\r\n");
    
    __KERNEL_MODE_PROC(
        _Free_PowerM_Object(p_pmnDelNode);
    );
    
    __LW_OBJECT_DELETE_HOOK(ulId);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_PowerMStart
** ��������: ����һ��ָ���ڵ�Ĺ��Ŀ��ƶ�ʱ��
** �䡡��  : ulId                ���Ŀ��ƶ�ʱ�����
**           ulMaxIdleTime       �ÿ��Ƶ���Ŀ���ʱ��
**           pfuncCallback       ����ʱ�䵽ʱִ�еĺ���
**           pvArg               �ص���������
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
                                           
                                       (�������ж��е���)
*********************************************************************************************************/
LW_API  
ULONG  API_PowerMStart (LW_OBJECT_HANDLE    ulId,
                        ULONG               ulMaxIdleTime,
                        LW_HOOK_FUNC        pfuncCallback,
                        PVOID               pvArg)
{
    REGISTER UINT16                    usIndex;
    REGISTER PLW_CLASS_POWERM_NODE     p_pmnNode;
    
    usIndex = _ObjectGetIndex(ulId);

    if (LW_CPU_GET_CUR_NESTING()) {                                     /*  �������ж��е���            */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "called from ISR.\r\n");
        _ErrorHandle(ERROR_KERNEL_IN_ISR);
        return  (ERROR_KERNEL_IN_ISR);
    }
    
#if LW_CFG_ARG_CHK_EN > 0
    if (!ulMaxIdleTime) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "ulMaxIdleTime invalidate.\r\n");
        _ErrorHandle(ERROR_POWERM_TIME);
        return  (ERROR_POWERM_TIME);
    }
    if (!pfuncCallback) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "pfuncCallback invalidate.\r\n");
        _ErrorHandle(ERROR_POWERM_FUNCTION);
        return  (ERROR_POWERM_FUNCTION);
    }
    if (!_ObjectClassOK(ulId, _OBJECT_POWERM)) {                        /*  �������ͼ��                */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
    if (_PowerM_Index_Invalid(usIndex)) {                               /*  �������������              */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
#endif

    __POWERM_LOCK();                                                    /*  ����                        */

    p_pmnNode = &_G__pmnControlNode[usIndex];
    
    if (!p_pmnNode->PMN_bIsUsing) {
        __POWERM_UNLOCK();                                              /*  ����                        */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
    
    if (p_pmnNode->PMN_bInQ) {                                          /*  �Ѿ�������ɨ������          */
        _WakeupDel(&_G_wuPowerM, &p_pmnNode->PMN_wunTimer);
        p_pmnNode->PMN_ulCounterSave = ulMaxIdleTime;                   /*  ���¸�ֵ                    */
        p_pmnNode->PMN_pfuncCallback = pfuncCallback;
        p_pmnNode->PMN_pvArg         = pvArg;
        p_pmnNode->PMN_ulCounter     = ulMaxIdleTime;
    
    } else {
        p_pmnNode->PMN_ulCounterSave = ulMaxIdleTime;                   /*  ���¸�ֵ                    */
        p_pmnNode->PMN_pfuncCallback = pfuncCallback;
        p_pmnNode->PMN_pvArg         = pvArg;
        p_pmnNode->PMN_ulCounter     = ulMaxIdleTime;
    }
    _WakeupAdd(&_G_wuPowerM, &p_pmnNode->PMN_wunTimer);

    __POWERM_UNLOCK();                                                  /*  ����                        */
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_PowerMCancel
** ��������: ��ָ�����Ŀ��ƶ�ʱ��ֹͣ
** �䡡��  : ulId                ���Ŀ��ƶ�ʱ�����
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
                                           
                                       (�������ж��е���)
*********************************************************************************************************/
LW_API  
ULONG  API_PowerMCancel (LW_OBJECT_HANDLE    ulId)
{
    REGISTER UINT16                    usIndex;
    REGISTER PLW_CLASS_POWERM_NODE     p_pmnNode;
    
    usIndex = _ObjectGetIndex(ulId);

    if (LW_CPU_GET_CUR_NESTING()) {                                     /*  �������ж��е���            */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "called from ISR.\r\n");
        _ErrorHandle(ERROR_KERNEL_IN_ISR);
        return  (ERROR_KERNEL_IN_ISR);
    }
    
#if LW_CFG_ARG_CHK_EN > 0
    if (!_ObjectClassOK(ulId, _OBJECT_POWERM)) {                        /*  �������ͼ��                */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
    if (_PowerM_Index_Invalid(usIndex)) {                               /*  �������������              */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
#endif

    __POWERM_LOCK();                                                    /*  ����                        */

    p_pmnNode = &_G__pmnControlNode[usIndex];
    
    if (!p_pmnNode->PMN_bIsUsing) {
        __POWERM_UNLOCK();                                              /*  ����                        */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
    
    if (p_pmnNode->PMN_bInQ) {                                          /*  �Ѿ�������ɨ������          */
        p_pmnNode->PMN_ulCounterSave = 0;
        p_pmnNode->PMN_ulCounter     = 0;
        _WakeupDel(&_G_wuPowerM, &p_pmnNode->PMN_wunTimer);
    }
    
    __POWERM_UNLOCK();                                                  /*  ����                        */
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_PowerMConnect
** ��������: ָ�����Ŀ��ƶ�ʱ���ص������Ͳ���
** �䡡��  : ulId                ���Ŀ��ƶ�ʱ�����
**           pfuncCallback       �»ص�����
**           pvArg               �ص���������
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
ULONG  API_PowerMConnect (LW_OBJECT_HANDLE    ulId,
                          LW_HOOK_FUNC        pfuncCallback,
                          PVOID               pvArg)
{
    REGISTER UINT16                    usIndex;
    REGISTER PLW_CLASS_POWERM_NODE     p_pmnNode;
    
    usIndex = _ObjectGetIndex(ulId);

    if (LW_CPU_GET_CUR_NESTING()) {                                     /*  �������ж��е���            */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "called from ISR.\r\n");
        _ErrorHandle(ERROR_KERNEL_IN_ISR);
        return  (ERROR_KERNEL_IN_ISR);
    }
    
#if LW_CFG_ARG_CHK_EN > 0
    if (!pfuncCallback) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "pfuncCallback invalidate.\r\n");
        _ErrorHandle(ERROR_POWERM_FUNCTION);
        return  (ERROR_POWERM_FUNCTION);
    }
    if (!_ObjectClassOK(ulId, _OBJECT_POWERM)) {                        /*  �������ͼ��                */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
    if (_PowerM_Index_Invalid(usIndex)) {                               /*  �������������              */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
#endif

    __POWERM_LOCK();                                                    /*  ����                        */

    p_pmnNode = &_G__pmnControlNode[usIndex];
    
    if (!p_pmnNode->PMN_bIsUsing) {
        __POWERM_UNLOCK();                                              /*  ����                        */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
    
    p_pmnNode->PMN_pfuncCallback = pfuncCallback;
    p_pmnNode->PMN_pvArg         = pvArg;
    
    __POWERM_UNLOCK();                                                  /*  ����                        */
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_PowerMSignal
** ��������: ��ָ�����Ŀ��ƶ�ʱ�����ڼ�ʱʱ, ����������Զ�ʱ����λ, ���¼�ʱ.
** �䡡��  : ulId                ���Ŀ��ƶ�ʱ�����
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : �˺��������������ж��е���, �п��ܻᵼ�� excJob �������, ��ɲ�����ʧ����.
                                           API ����
*********************************************************************************************************/
LW_API  
ULONG  API_PowerMSignal (LW_OBJECT_HANDLE    ulId)
{
    REGISTER UINT16                    usIndex;
    REGISTER PLW_CLASS_POWERM_NODE     p_pmnNode;
    REGISTER INT                       iError;
    
    if (LW_CPU_GET_CUR_NESTING()) {                                     /*  �����ж���                  */
        iError = _excJobAdd((VOIDFUNCPTR)API_PowerMSignal, 
                            (PVOID)ulId, 0, 0, 0,
                            0, 0);                                      /*  �ӳٴ���                    */
        if (iError) {
            return  (ERROR_EXCE_LOST);
        } else {
            return  (ERROR_NONE);
        }
    }
    
    usIndex = _ObjectGetIndex(ulId);
    
#if LW_CFG_ARG_CHK_EN > 0
    if (!_ObjectClassOK(ulId, _OBJECT_POWERM)) {                        /*  �������ͼ��                */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
    if (_PowerM_Index_Invalid(usIndex)) {                               /*  �������������              */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
#endif

    __POWERM_LOCK();                                                    /*  ����                        */
    
    p_pmnNode = &_G__pmnControlNode[usIndex];
    
    if (!p_pmnNode->PMN_bIsUsing) {
        __POWERM_UNLOCK();                                              /*  ����                        */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
    
    if (p_pmnNode->PMN_bInQ) {                                          /*  �Ѿ�������ɨ������          */
        _WakeupDel(&_G_wuPowerM, &p_pmnNode->PMN_wunTimer);
    }
        
    p_pmnNode->PMN_ulCounter = p_pmnNode->PMN_ulCounterSave;            /*  ��λ��ʱ��                  */
   
    _WakeupAdd(&_G_wuPowerM, &p_pmnNode->PMN_wunTimer);
    
    __POWERM_UNLOCK();                                                  /*  ����                        */
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_PowerMSignalFast
** ��������: ����Ҫ�Լ��ߵ�Ƶ�ʵ��� API_PowerMSignal ����ʱ, ��ȷ�����׼ȷ����������, 
             ����ʹ����������ӿ��ٶ�, ����: TOUCH SCREEN ��ɨ�����.
** �䡡��  : ulId                ���Ŀ��ƶ�ʱ�����
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : �˺����������ж��е���.
                                           API ����
*********************************************************************************************************/
LW_API  
ULONG   API_PowerMSignalFast (LW_OBJECT_HANDLE    ulId)
{
    REGISTER UINT16                    usIndex;
    REGISTER PLW_CLASS_POWERM_NODE     p_pmnNode;
    
    usIndex = _ObjectGetIndex(ulId);
    
    __POWERM_LOCK();                                                    /*  ����                        */
    
    p_pmnNode = &_G__pmnControlNode[usIndex];
    
    if (p_pmnNode->PMN_bInQ) {
        _WakeupDel(&_G_wuPowerM, &p_pmnNode->PMN_wunTimer);
    }
        
    p_pmnNode->PMN_ulCounter = p_pmnNode->PMN_ulCounterSave;            /*  ��λ��ʱ��                  */
    
    _WakeupAdd(&_G_wuPowerM, &p_pmnNode->PMN_wunTimer);
    
    __POWERM_UNLOCK();                                                  /*  ����                        */
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_PowerMStatus
** ��������: ���ָ�����Ŀ��ƶ�ʱ���Ĺ���״̬
** �䡡��  : ulId                ���Ŀ��ƶ�ʱ�����
**           pulCounter          ��ǰ����ֵ
**           pulMaxIdleTime      ������ʱ��
**           ppfuncCallback      �ص�����
**           ppvArg              �Ե���������
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
ULONG  API_PowerMStatus (LW_OBJECT_HANDLE    ulId,
                         ULONG              *pulCounter,
                         ULONG              *pulMaxIdleTime,
                         LW_HOOK_FUNC       *ppfuncCallback,
                         PVOID              *ppvArg)
{
    REGISTER UINT16                    usIndex;
    REGISTER PLW_CLASS_POWERM_NODE     p_pmnNode;
    
    usIndex = _ObjectGetIndex(ulId);

    if (LW_CPU_GET_CUR_NESTING()) {                                     /*  �������ж��е���            */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "called from ISR.\r\n");
        _ErrorHandle(ERROR_KERNEL_IN_ISR);
        return  (ERROR_KERNEL_IN_ISR);
    }
    
#if LW_CFG_ARG_CHK_EN > 0
    if (!_ObjectClassOK(ulId, _OBJECT_POWERM)) {                        /*  �������ͼ��                */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
    if (_PowerM_Index_Invalid(usIndex)) {                               /*  �������������              */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
#endif

    __POWERM_LOCK();                                                    /*  ����                        */

    p_pmnNode = &_G__pmnControlNode[usIndex];
    
    if (!p_pmnNode->PMN_bIsUsing) {
        __POWERM_UNLOCK();                                              /*  ����                        */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
    
    if (pulCounter) {
        if (p_pmnNode->PMN_bInQ) {
            _WakeupStatus(&_G_wuPowerM, &p_pmnNode->PMN_wunTimer, pulCounter);
        } else {
            *pulCounter = 0ul;
        }
    }
    if (pulMaxIdleTime) {
        *pulMaxIdleTime = p_pmnNode->PMN_ulCounterSave;
    }
    if (ppfuncCallback) {
        *ppfuncCallback = p_pmnNode->PMN_pfuncCallback;
    }
    if (ppvArg) {
        *ppvArg = p_pmnNode->PMN_pvArg;
    }
    
    __POWERM_UNLOCK();                                                  /*  ����                        */
    
    return  (ERROR_NONE);
}
#endif                                                                  /*  LW_CFG_POWERM_EN            */
                                                                        /*  LW_CFG_MAX_POWERM_NODES     */
/*********************************************************************************************************
  END
*********************************************************************************************************/
