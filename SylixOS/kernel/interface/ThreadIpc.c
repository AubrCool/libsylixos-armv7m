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
** ��   ��   ��: ThreadIpc.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2013 �� 03 �� 16 ��
**
** ��        ��: ����ϵͳ�� IPC ��������Ҫ�������뼤���֧�֡�

** BUG:
2013.05.05  �жϵ���������ֵ, �������������û����˳�.
2013.07.18  ʹ���µĻ�ȡ TCB �ķ���, ȷ�� SMP ϵͳ��ȫ.
2013.09.17  ʹ�� POSIX �涨��ȡ���㶯��.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
  s_internal.h ��Ҳ����ض���
*********************************************************************************************************/
#if LW_CFG_THREAD_CANCEL_EN > 0
#define __THREAD_CANCEL_POINT()         API_ThreadTestCancel()
#else
#define __THREAD_CANCEL_POINT()
#endif                                                                  /*  LW_CFG_THREAD_CANCEL_EN     */
/*********************************************************************************************************
** ��������: API_ThreadIpcWait
** ��������: ��ǰ�̵߳ȴ�һ�� IPC �¼�
** �䡡��  : ulTimeOut     ��ʱʱ��
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
                                           
                                       (�������ж��е���)
*********************************************************************************************************/
LW_API  
ULONG  API_ThreadIpcWait (ULONG  ulTimeOut)
{
             INTREG             iregInterLevel;
             ULONG              ulTimeSave;                             /*  ϵͳ�¼���¼                */
             INT                iSchedRet;
             
             PLW_CLASS_TCB      ptcbCur;
    REGISTER PLW_CLASS_PCB      ppcb;
    
__wait_again:
    if (ulTimeOut == LW_OPTION_NOT_WAIT) {                              /*  �����еȴ�                  */
        _ErrorHandle(EAGAIN);
        return  (EAGAIN);
    }
    
    __THREAD_CANCEL_POINT();                                            /*  ����ȡ����                  */
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);                                       /*  ��ǰ������ƿ�              */
    
    iregInterLevel = __KERNEL_ENTER_IRQ();                              /*  �����ں�                    */
    
    ptcbCur->TCB_usStatus      |= LW_THREAD_STATUS_IPC;                 /*  �ȴ� IPC                    */
    ptcbCur->TCB_ucWaitTimeOut  = LW_WAIT_TIME_CLEAR;                   /*  ��յȴ�ʱ��                */
    
    ppcb = _GetPcb(ptcbCur);
    __DEL_FROM_READY_RING(ptcbCur, ppcb);                               /*  �Ӿ�������ɾ��              */
    
    if (ulTimeOut == LW_OPTION_WAIT_INFINITE) {
        ptcbCur->TCB_ulDelay = 0ul;
    } else {
        ptcbCur->TCB_ulDelay = ulTimeOut;                               /*  ���ó�ʱʱ��                */
        __ADD_TO_WAKEUP_LINE(ptcbCur);                                  /*  ����ȴ�ɨ����              */
    }
    
    __KERNEL_TIME_GET(ulTimeSave, ULONG);                               /*  ��¼ϵͳʱ��                */
    
    iSchedRet = __KERNEL_EXIT_IRQ(iregInterLevel);                      /*  ����������                  */
    if (iSchedRet == LW_SIGNAL_EINTR) {
        _ErrorHandle(EINTR);
        return  (EINTR);
    
    } else if (iSchedRet == LW_SIGNAL_RESTART) {
        ulTimeOut = _sigTimeOutRecalc(ulTimeSave, ulTimeOut);           /*  ���¼��㳬ʱʱ��            */
        goto    __wait_again;
    }
    
    iregInterLevel = __KERNEL_ENTER_IRQ();                              /*  �����ں�                    */
    if (ptcbCur->TCB_ucWaitTimeOut == LW_WAIT_TIME_OUT) {               /*  �ȴ���ʱ                    */
        __KERNEL_EXIT_IRQ(iregInterLevel);                              /*  �˳��ں�                    */
        _ErrorHandle(EAGAIN);
        return  (EAGAIN);
    }
    __KERNEL_EXIT_IRQ(iregInterLevel);                                  /*  �˳��ں�                    */
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_ThreadIpcWakeup
** ��������: ����ȴ�ָ�� IPC ���߳�
** �䡡��  : ulId      �߳� ID
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : ʹ�õ�ʱ��һ��Ҫȷ��Ŀ���̲߳��������ڵȴ��ź���, �������ɴ���.

                                           API ����
                                           
                                       (�������ж��е���)
*********************************************************************************************************/
LW_API  
ULONG  API_ThreadIpcWakeup (LW_OBJECT_HANDLE  ulId)
{
             INTREG                iregInterLevel;
    REGISTER UINT16                usIndex;
    REGISTER PLW_CLASS_TCB         ptcb;
    REGISTER PLW_CLASS_PCB         ppcb;
	
    usIndex = _ObjectGetIndex(ulId);
    
#if LW_CFG_ARG_CHK_EN > 0
    if (!_ObjectClassOK(ulId, _OBJECT_THREAD)) {                         /*  ��� ID ������Ч��         */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "thread handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
    
    if (_Thread_Index_Invalid(usIndex)) {                                /*  ����߳���Ч��             */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "thread handle invalidate.\r\n");
        _ErrorHandle(ERROR_THREAD_NULL);
        return  (ERROR_THREAD_NULL);
    }
#endif

    __KERNEL_ENTER();                                                   /*  �����ں�                    */
    if (_Thread_Invalid(usIndex)) {
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "thread handle invalidate.\r\n");
        _ErrorHandle(ERROR_THREAD_NULL);
        return  (ERROR_THREAD_NULL);
    }
    
    iregInterLevel = KN_INT_DISABLE();                                  /*  �ر��ж�                    */
    
    ptcb = _K_ptcbTCBIdTable[usIndex];
    ppcb = _GetPcb(ptcb);
    
    if (ptcb->TCB_usStatus & LW_THREAD_STATUS_DELAY) {
        __DEL_FROM_WAKEUP_LINE(ptcb);                                   /*  �ӵȴ�����ɾ��              */
        ptcb->TCB_ulDelay = 0ul;
    }
    
    if (ptcb->TCB_usStatus & LW_THREAD_STATUS_IPC) {
        ptcb->TCB_usStatus &= (~LW_THREAD_STATUS_IPC);
    }
        
    ptcb->TCB_ucWaitTimeOut = LW_WAIT_TIME_CLEAR;
    
    if (__LW_THREAD_IS_READY(ptcb)) {                                   /*  ����Ƿ����                */
        ptcb->TCB_ucSchedActivate = LW_SCHED_ACT_OTHER;
        __ADD_TO_READY_RING(ptcb, ppcb);                                /*  ���������                  */
    }
    
    KN_INT_ENABLE(iregInterLevel);
    __KERNEL_EXIT();                                                    /*  �˳��ں� (���ܻ����)       */
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
