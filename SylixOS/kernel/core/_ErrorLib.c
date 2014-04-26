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
** ��   ��   ��: _ErrorLib.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2006 �� 12 �� 13 ��
**
** ��        ��: ����ϵͳ�������.

** BUG:
2011.03.04  ʹ�� __errno ��Ϊ������.
2012.03.20  ���ٶ� _K_ptcbTCBCur ������, �������þֲ�����, ���ٶԵ�ǰ CPU ID ��ȡ�Ĵ���.
2012.07.21  �������������ʾ���ݺ���, ����ʹ�ú�.
2014.04.08  ���� _DebugHandle() �Ѿ�Ϊ����, ɥʧ�˻�õ�����λ����Ϣ������, ���ﲻ�ٴ�ӡ����λ��.
2013.07.18  ʹ���µĻ�ȡ TCB �ķ���, ȷ�� SMP ϵͳ��ȫ.
2013.12.13  �� _DebugHandle() ����Ϊ _DebugMessage(), _DebugHandle() ת�ɺ�ʵ��.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
  ��ӡ��Ϣ
*********************************************************************************************************/
#define __ERROR_THREAD_SHOW()   do {    \
            if (LW_CPU_GET_CUR_NESTING()) { \
                _K_pfuncKernelDebugError("in interrupt context.\r\n");  \
            } else {    \
                REGISTER PLW_CLASS_TCB   ptcb;  \
                LW_TCB_GET_CUR_SAFE(ptcb);  \
                if (ptcb && ptcb->TCB_cThreadName[0] != PX_EOS) { \
                    _K_pfuncKernelDebugError("in thread \"");   \
                    _K_pfuncKernelDebugError(ptcb->TCB_cThreadName);    \
                    _K_pfuncKernelDebugError("\" context.\r\n");    \
                }   \
            }   \
        } while (0)
/*********************************************************************************************************
** ��������: __errno
** ��������: posix ��õ�ǰ errno
** �䡡��  : 
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : ���� longwing ������ʷԭ����� ulong ���������, �� posix ʹ�� errno_t ����, ���������ϵͳ
             �� errno_t ����Ϊ int ��, ����ʹ�� GCC 3.x ���ϰ汾���� -fstrict-aliasing ����.����Ĵ������
             �����һ������: strict aliasing, Ŀǰ���˾�����Դ���.
*********************************************************************************************************/
errno_t *__errno (VOID)
{
    INTREG          iregInterLevel;
    PLW_CLASS_CPU   pcpuCur;
    PLW_CLASS_TCB   ptcbCur;
    errno_t        *perrno;
    
    iregInterLevel = KN_INT_DISABLE();                                  /*  �ر��ж�, ��ֹ���ȵ����� CPU*/
    
    pcpuCur = LW_CPU_GET_CUR();
    
    if (pcpuCur->CPU_ulInterNesting) {
        perrno = (errno_t *)(&pcpuCur->CPU_ulInterError[pcpuCur->CPU_ulInterNesting]);
    } else {
        ptcbCur = pcpuCur->CPU_ptcbTCBCur;
        if (ptcbCur) {
            perrno = (errno_t *)(&ptcbCur->TCB_ulLastError);
        } else {
            perrno = (errno_t *)(&_K_ulNotRunError);
        }
    }
    
    KN_INT_ENABLE(iregInterLevel);
    
    return  (perrno);
}
/*********************************************************************************************************
** ��������: _ErrorHandle
** ��������: ��¼��ǰ�����
** �䡡��  : ulErrorCode       ��ǰ�����
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : 
*********************************************************************************************************/
VOID  _ErrorHandle (ULONG  ulErrorCode)
{
    INTREG          iregInterLevel;
    PLW_CLASS_CPU   pcpuCur;
    PLW_CLASS_TCB   ptcbCur;
    
#if LW_CFG_ERRORNO_AUTO_CLEAR == 0
    if (ulErrorCode == 0) {
        return;
    }
#endif

    iregInterLevel = KN_INT_DISABLE();                                  /*  �ر��ж�, ��ֹ���ȵ����� CPU*/
    
    pcpuCur = LW_CPU_GET_CUR();
    
    if (pcpuCur->CPU_ulInterNesting) {
        pcpuCur->CPU_ulInterError[pcpuCur->CPU_ulInterNesting] = ulErrorCode;
    } else {
        ptcbCur = pcpuCur->CPU_ptcbTCBCur;
        if (ptcbCur) {
            ptcbCur->TCB_ulLastError = ulErrorCode;
        } else {
            _K_ulNotRunError = ulErrorCode;
        }
    }
    
    KN_INT_ENABLE(iregInterLevel);
}
/*********************************************************************************************************
** ��������: _DebugMessage
** ��������: �ں˴�ӡ������Ϣ
** �䡡��  : iLevel      �ȼ�
**           pcPosition  λ��
**           pcString    ��ӡ����
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : 
*********************************************************************************************************/
VOID  _DebugMessage (INT  iLevel, CPCHAR  pcPosition, CPCHAR  pcString)
{
#if LW_CFG_ERRORMESSAGE_EN > 0
    if (_K_pfuncKernelDebugError && (iLevel & __ERRORMESSAGE_LEVEL)) {
        _K_pfuncKernelDebugError(pcPosition);
        _K_pfuncKernelDebugError("() error: ");
        _K_pfuncKernelDebugError(pcString);
        __ERROR_THREAD_SHOW();
    }
#endif

#if LW_CFG_LOGMESSAGE_EN > 0
    if (_K_pfuncKernelDebugLog && (iLevel & __LOGMESSAGE_LEVEL)) {
        _K_pfuncKernelDebugLog(pcString);
    }
#endif
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
