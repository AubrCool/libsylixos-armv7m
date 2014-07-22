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
** ��   ��   ��: _Sched.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2006 �� 12 �� 18 ��
**
** ��        ��: ����ϵͳ�ں˵�������

** BUG
2008.01.04  �޸Ĵ����ʽ��ע��.
2008.04.06  ֧���߳������ķ���ֵ�Ĺ���.
2009.04.29  ���� SMP ֧��.
2011.02.22  ���� _SchedInt() �жϵ���(û�н���ǰ�ֳ�ѹջ), �������еĵ��ȶ�����������.
2012.09.07  �Ż�����.
2012.09.23  ���� IDLE ENTER �� IDLE EXIT �ص�����.
2013.07.17  ������ֻ���𱾺˵� cpu �ṹ��ֵ, ������ cpu ֻ���ú˼��ж�֪ͨ����.
2013.07.19  �ϲ� _SchedInt �� _SchedCoreInt ͨ�� Cur �� High ͬʱ�ж��Ƿ���Ҫ���ͺ˼��ж�.
2013.07.21  ������Ӧ�������ж��������Ƿ���Ҫ����, �����Ҫ, ���ͺ˼��ж�, Ȼ���ٴ����˵���.
2013.07.29  �����ѡ���б�������ȼ�����, �����ȴ������.
2013.08.28  �����ں��¼����.
2013.12.02  _SchedGetCandidate() ������ռ��������Ϊ 1.
2014.01.05  �������� BUG ���ٷ��ڴ˴�.
2014.01.07  ���� _SchedCrSwp, ͳһ������Э����ֲ�ĸ�ʽ.
2014.07.21  ���� CPU ֹͣ����.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
  scheduler bug trace
*********************************************************************************************************/
#ifdef  __LW_SCHEDULER_BUG_TRACE_EN
#define __LW_SCHEDULER_BUG_TRACE(ptcb)  do {                                            \
            if (ptcb == LW_NULL) {                                                      \
                _DebugHandle(__ERRORMESSAGE_LEVEL,                                      \
                             "scheduler candidate serious error, ptcb == NULL.\r\n");   \
                for (;;);                                                               \
            } else if (!__LW_THREAD_IS_READY(ptcb)) {                                   \
                CHAR    cBuffer[128];                                                   \
                snprintf(cBuffer, sizeof(cBuffer),                                      \
                         "scheduler candidate serious error, "                          \
                         "ptcb %p, name \"%s\", status 0x%x.\r\n",                      \
                         ptcb, ptcb->TCB_cThreadName, ptcb->TCB_usStatus);              \
                _DebugHandle(__ERRORMESSAGE_LEVEL, cBuffer);                            \
                for (;;);                                                               \
            }                                                                           \
        } while (0)
#else
#define __LW_SCHEDULER_BUG_TRACE(ptcb)
#endif                                                                  /*  __LW_SCHEDULER_BUG_TRACE_EN */
/*********************************************************************************************************
  ����˽�б����л�
*********************************************************************************************************/
#if (LW_CFG_THREAD_PRIVATE_VARS_EN > 0) && (LW_CFG_MAX_THREAD_GLB_VARS > 0)
#define __LW_TASK_SWITCH_VAR(ptcbCur, ptcbHigh)     _ThreadVarSwith(ptcbCur, ptcbHigh)
#define __LW_TASK_SAVE_VAR(ptcbCur)                 _ThreadVarSave(ptcbCur)
#else
#define __LW_TASK_SWITCH_VAR(ptcbCur, ptcbHigh)
#define __LW_TASK_SAVE_VAR(ptcbCur)
#endif
/*********************************************************************************************************
  ���� FPU �������л�
*********************************************************************************************************/
#if LW_CFG_CPU_FPU_EN > 0
#define __LW_TASK_SWITCH_FPU(bIntSwitch)            _ThreadFpuSwith(bIntSwitch)
#define __LW_TASK_SAVE_FPU(ptcbCur, bIntSwitch)     _ThreadFpuSave(ptcbCur, bIntSwitch)
#else
#define __LW_TASK_SWITCH_FPU(bIntSwitch)
#define __LW_TASK_SAVE_FPU(ptcbCur, bIntSwitch)
#endif
/*********************************************************************************************************
** ��������: _SchedSmpNotify
** ��������: ֪ͨ��Ҫ���ȵ� CPU
** �䡡��  : ulCPUIdCur ��ǰ CPU ID
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : ����һ����û�б���������, �Ҳ����˵���������, ����û���ڴ��� sched �˼��ж�, ���ͺ˼��ж�.
*********************************************************************************************************/
#if LW_CFG_SMP_EN > 0

static VOID  _SchedSmpNotify (ULONG  ulCPUIdCur)
{
    INT             i;
    PLW_CLASS_CPU   pcpu;
    
    for (i = 0; i < LW_NCPUS; i++) {                                    /*  ���� CPU ����Ƿ���Ҫ����   */
        if (ulCPUIdCur != i) {
            pcpu = LW_CPU_GET(i);
            if ((__SHOULD_SCHED(pcpu, 0)) && LW_CAND_ROT(pcpu) &&
                ((LW_CPU_GET_IPI_PEND(i) & LW_IPI_SCHED_MSK) == 0)) {
                _SmpSendIpi(i, LW_IPI_SCHED, 0);                        /*  �����˼��ж�                */
            }
        }
    }
}
/*********************************************************************************************************
** ��������: _SchedCpuDown
** ��������: CPU ֹͣ����
** �䡡��  : pcpuCur       ��ǰ CPU ���ƿ�
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static LW_INLINE VOID  _SchedCpuDown (PLW_CLASS_CPU  pcpuCur, BOOL  bIsIntSwtich)
{
    REGISTER PLW_CLASS_TCB  ptcbCur = pcpuCur->CPU_ptcbTCBCur;
    REGISTER ULONG          ulCPUId = pcpuCur->CPU_ulCPUId;

    _CpuInactiveNoLock(pcpuCur);                                        /*  ֹͣ CPU                    */
    
    __LW_TASK_SAVE_VAR(ptcbCur);
    __LW_TASK_SAVE_FPU(ptcbCur, bIsIntSwtich);
    
    _SchedSmpNotify(ulCPUId);                                           /*  �������� CPU ����           */
    
    LW_CPU_CLR_IPI_PEND(ulCPUId, LW_IPI_DOWN_MSK);                      /*  ��� CPU DOWN �жϱ�־      */
    
    LW_SPIN_UNLOCK_IGNIRQ(&_K_slScheduler);                             /*  ���������� spinlock         */

    bspCpuDown(ulCPUId);                                                /*  BSP ֹͣ CPU                */
    
    bspDebugMsg("CPU Down error!\r\n");                                 /*  �������е�����              */
    for (;;);
}

#endif                                                                  /*  LW_CFG_SMP_EN               */
/*********************************************************************************************************
** ��������: _SchedSwap
** ��������: arch �����л��������ȱ��浱ǰ�߳�������, Ȼ����ô˺���, Ȼ���ٻָ���Ҫִ�������������.
** �䡡��  : pcpuCur   ��ǰCPU
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : 
*********************************************************************************************************/
VOID _SchedSwp (PLW_CLASS_CPU pcpuCur)
{
    REGISTER PLW_CLASS_TCB      ptcbCur      = pcpuCur->CPU_ptcbTCBCur;
    REGISTER PLW_CLASS_TCB      ptcbHigh     = pcpuCur->CPU_ptcbTCBHigh;
    REGISTER LW_OBJECT_HANDLE   ulCurId      = ptcbCur->TCB_ulId;
    REGISTER LW_OBJECT_HANDLE   ulHighId     = ptcbHigh->TCB_ulId;
             BOOL               bIsIntSwtich = pcpuCur->CPU_bIsIntSwtich;

    _StackCheckGuard(ptcbCur);                                          /*  ��ջ������                */
    
#if LW_CFG_SMP_EN > 0
    if (LW_CPU_GET_IPI_PEND(pcpuCur->CPU_ulCPUId) & LW_IPI_DOWN_MSK) {  /*  ��ǰ CPU ��Ҫֹͣ           */
        _SchedCpuDown(pcpuCur, bIsIntSwtich);
    }
#endif                                                                  /*  LW_CFG_SMP_EN > 0           */
    
    __LW_TASK_SWITCH_VAR(ptcbCur, ptcbHigh);                            /*  �߳�˽�л������л�          */
    __LW_TASK_SWITCH_FPU(bIsIntSwtich);
    
    if (_ObjectGetIndex(ulHighId) < LW_NCPUS) {                         /*  CPU �������ģʽ            */
        __LW_CPU_IDLE_ENTER_HOOK(ulCurId);
        
    } else if (_ObjectGetIndex(ulCurId) < LW_NCPUS) {                   /*  CPU �˳�����ģʽ            */
        __LW_CPU_IDLE_EXIT_HOOK(ulHighId);
    }
    
#if LW_CFG_SMP_EN > 0
    _SchedSmpNotify(pcpuCur->CPU_ulCPUId);                              /*  SMP ����֪ͨ                */
#endif                                                                  /*  LW_CFG_SMP_EN               */

    pcpuCur->CPU_ptcbTCBCur = ptcbHigh;                                 /*  �л�����                    */

    LW_SPIN_UNLOCK_SCHED(&_K_slScheduler, ptcbCur);                     /*  ���������� spinlock         */
    
    bspTaskSwapHook(ulCurId, ulHighId);
    __LW_THREAD_SWAP_HOOK(ulCurId, ulHighId);
    
    if (bIsIntSwtich) {
        MONITOR_EVT_LONG2(MONITOR_EVENT_ID_SCHED, MONITOR_EVENT_SCHED_INT, 
                          ulCurId, ulHighId, LW_NULL);
    } else {
        MONITOR_EVT_LONG2(MONITOR_EVENT_ID_SCHED, MONITOR_EVENT_SCHED_TASK,
                          ulCurId, ulHighId, LW_NULL);
    }
}
/*********************************************************************************************************
** ��������: _CoroutineDeleteAll
** ��������: �ͷ�ָ���߳����е�Э�̿ռ�.
** �䡡��  : ptcb                     �߳̿��ƿ�
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
#if LW_CFG_COROUTINE_EN > 0

VOID _SchedCrSwp (PLW_CLASS_CPU pcpuCur)
{
    pcpuCur->CPU_pcrcbCur = pcpuCur->CPU_pcrcbNext;                     /*  �л�Э��                    */
}

#endif                                                                  /*  LW_CFG_COROUTINE_EN > 0     */
/*********************************************************************************************************
** ��������: _Sched
** ��������: �ں˵�����
** �䡡��  : NONE
** �䡡��  : �߳������ķ���ֵ
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
INT  _Sched (VOID)
{
             INTREG           iregInterLevel;
             ULONG            ulCPUId;
             PLW_CLASS_CPU    pcpuCur;
             PLW_CLASS_TCB    ptcbCur;
             PLW_CLASS_TCB    ptcbCand;
    REGISTER INT              iRetVal = ERROR_NONE;
    
#if LW_CFG_SMP_EN > 0
             BOOL             bStatusReq = LW_FALSE;

__status_change:
    if (bStatusReq) {
        _ThreadStatusChangeCur(ptcbCur);
    }
#endif                                                                  /*  LW_CFG_SMP_EN               */

    LW_SPIN_LOCK_QUICK(&_K_slScheduler, &iregInterLevel);               /*  �������� spinlock �ر��ж�  */
    
    ulCPUId = LW_CPU_GET_CUR_ID();                                      /*  ��ǰ CPUID                  */
    pcpuCur = LW_CPU_GET(ulCPUId);                                      /*  ��ǰ CPU ���ƿ�             */
    ptcbCur = pcpuCur->CPU_ptcbTCBCur;
    
#if LW_CFG_SMP_EN > 0
    if (ptcbCur->TCB_plineStatusReqHeader) {                            /*  ����ǰ����ı�״̬        */
        if (__THREAD_LOCK_GET(ptcbCur) <= 1ul) {                        /*  �Ƿ���Խ���״̬�л�        */
            LW_SPIN_UNLOCK_QUICK(&_K_slScheduler, iregInterLevel);      /*  ���������� spinlock ���ж�*/
            bStatusReq = LW_TRUE;
            goto    __status_change;
        }
    }
#endif                                                                  /*  LW_CFG_SMP_EN               */
    
    ptcbCand = _SchedGetCand(pcpuCur, 1ul);                             /*  �����Ҫ���е��߳�          */
    if (ptcbCand != ptcbCur) {                                          /*  ����뵱ǰ���еĲ�ͬ, �л�  */
        __LW_SCHEDULER_BUG_TRACE(ptcbCand);                             /*  ������ BUG ���             */
        pcpuCur->CPU_bIsIntSwtich = LW_FALSE;                           /*  ���жϵ���                  */
        pcpuCur->CPU_ptcbTCBHigh  = ptcbCand;
        archTaskCtxSwitch(pcpuCur);                                     /*  �߳��л�,���ͷŵ����������� */
        LW_SPIN_LOCK_IGNIRQ(&_K_slScheduler);                           /*  ���������������¼���        */
    }
#if LW_CFG_SMP_EN > 0                                                   /*  SMP ϵͳ                    */
      else {
        _SchedSmpNotify(ulCPUId);                                       /*  SMP ����֪ͨ                */
        LW_SPIN_UNLOCK_QUICK(&_K_slScheduler, iregInterLevel);          /*  ���������� spinlock ���ж�*/
        return  (iRetVal);
    }
#endif                                                                  /*  LW_CFG_SMP_EN               */
    
    LW_TCB_GET_CUR(ptcbCur);                                            /*  ����µĵ�ǰ TCB            */
    
    iRetVal = ptcbCur->TCB_iSchedRet;                                   /*  ��õ������źŵķ���ֵ      */
    ptcbCur->TCB_iSchedRet = ERROR_NONE;                                /*  ���                        */
    
    LW_SPIN_UNLOCK_QUICK(&_K_slScheduler, iregInterLevel);              /*  ���������� spinlock ���ж�*/
    
    return  (iRetVal);
}
/*********************************************************************************************************
** ��������: _SchedInt
** ��������: �ں˵�����(��ͨ�жϵ���, ���ô���ǰ����������, ����ȡ�������ķ���ֵ)
** �䡡��  : NONE
** �䡡��  : �߳������ķ���ֵ
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
INT  _SchedInt (VOID)
{
             INTREG           iregInterLevel;
             ULONG            ulCPUId;
             PLW_CLASS_CPU    pcpuCur;
             PLW_CLASS_TCB    ptcbCur;
             PLW_CLASS_TCB    ptcbCand;
    REGISTER INT              iRetVal = ERROR_NONE;
    
#if LW_CFG_SMP_EN > 0
             BOOL             bStatusReq = LW_FALSE;
    
__status_change:
    if (bStatusReq) {
        _ThreadStatusChangeCur(ptcbCur);
    }
#endif                                                                  /*  LW_CFG_SMP_EN               */
    
    LW_SPIN_LOCK_QUICK(&_K_slScheduler, &iregInterLevel);               /*  �������� spinlock �ر��ж�  */
    
    ulCPUId = LW_CPU_GET_CUR_ID();                                      /*  ��ǰ CPUID                  */
    pcpuCur = LW_CPU_GET(ulCPUId);                                      /*  ��ǰ CPU ���ƿ�             */
    ptcbCur = pcpuCur->CPU_ptcbTCBCur;
    
#if LW_CFG_SMP_EN > 0
    if (ptcbCur->TCB_plineStatusReqHeader) {                            /*  ����ǰ����ı�״̬        */
        if (__THREAD_LOCK_GET(ptcbCur) <= 1ul) {                        /*  �Ƿ���Խ���״̬�л�        */
            LW_SPIN_UNLOCK_QUICK(&_K_slScheduler, iregInterLevel);      /*  ���������� spinlock ���ж�*/
            bStatusReq = LW_TRUE;
            goto    __status_change;
        }
    }
    LW_CPU_CLR_IPI_PEND(ulCPUId, LW_IPI_SCHED_MSK);                     /*  ����˼�����жϱ�־        */
    
    if (LW_CPU_GET_IPI_PEND(pcpuCur->CPU_ulCPUId) & LW_IPI_DOWN_MSK) {  /*  ��ǰ CPU ��Ҫֹͣ           */
        _SchedCpuDown(pcpuCur, LW_TRUE);
    }
#endif                                                                  /*  LW_CFG_SMP_EN               */
    
    ptcbCand = _SchedGetCand(pcpuCur, 1ul);                             /*  �����Ҫ���е��߳�          */
    if (ptcbCand != ptcbCur) {                                          /*  ����뵱ǰ���еĲ�ͬ, �л�  */
        __LW_SCHEDULER_BUG_TRACE(ptcbCand);                             /*  ������ BUG ���             */
        pcpuCur->CPU_bIsIntSwtich = LW_TRUE;                            /*  �жϵ���                    */
        pcpuCur->CPU_ptcbTCBHigh  = ptcbCand;
        archIntCtxLoad(pcpuCur);                                        /*  �ж����������߳��л�        */
        LW_SPIN_LOCK_IGNIRQ(&_K_slScheduler);                           /*  ���������������¼���        */
    }
#if LW_CFG_SMP_EN > 0                                                   /*  SMP ϵͳ                    */
      else {
        _SchedSmpNotify(ulCPUId);                                       /*  SMP ����֪ͨ                */
        LW_SPIN_UNLOCK_QUICK(&_K_slScheduler, iregInterLevel);          /*  ���������� spinlock ���ж�*/
        return  (iRetVal);
    }
#endif                                                                  /*  LW_CFG_SMP_EN               */

    iRetVal = ERROR_NONE;                                               /*  ����źŵķ���ֵ            */
    
    LW_SPIN_UNLOCK_QUICK(&_K_slScheduler, iregInterLevel);              /*  ���������� spinlock ���ж�*/
    
    return  (iRetVal);
}
/*********************************************************************************************************
** ��������: _SchedSetRet
** ��������: ���õ�ǰ�������������ֵ, �ڲ�����������ʱ, ����ȡ���ֵ
** �䡡��  : iSchedRet         ����������ֵ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SchedSetRet (INT  iSchedSetRet)
{
    INTREG           iregInterLevel;
    PLW_CLASS_TCB    ptcbCur;
    
    LW_SPIN_LOCK_QUICK(&_K_slScheduler, &iregInterLevel);               /*  �������� spinlock �ر��ж�  */
    LW_TCB_GET_CUR(ptcbCur);
    if (ptcbCur->TCB_iSchedRet == ERROR_NONE) {
        ptcbCur->TCB_iSchedRet =  iSchedSetRet;
    }
    LW_SPIN_UNLOCK_QUICK(&_K_slScheduler, iregInterLevel);              /*  ���������� spinlock ���ж�*/
}
/*********************************************************************************************************
** ��������: _SchedSetPrio
** ��������: ����ָ���������ȼ� (�����ں�״̬�±�����)
** �䡡��  : ptcb           ����
**           ucPriority     ���ȼ�
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : ����ǵ�ǰ���ں�ѡ����, ֻҪ����Ŀ���������� CPU ��ѡ����Ʊ�Ǽ���, ��ǰ����������ʱ, ����
             �����������Ʊ�־, ���½���һ����ռ���Ȳ���, ����и��Ӻ��ʵ��������, ����ռĿ������.
*********************************************************************************************************/
VOID  _SchedSetPrio (PLW_CLASS_TCB  ptcb, UINT8  ucPriority)
{
    INTREG           iregInterLevel;
    PLW_CLASS_PCB    ppcbFrom;
    PLW_CLASS_PCB    ppcbTo;
    
    ppcbFrom = _GetPcb(ptcb);
    ppcbTo   = _GetPcbByPrio(ucPriority);
    
    MONITOR_EVT_LONG3(MONITOR_EVENT_ID_THREAD, MONITOR_EVENT_THREAD_PRIO,
                      ptcb->TCB_ulId, ptcb->TCB_ucPriority, ucPriority, LW_NULL);
                      
    iregInterLevel = KN_INT_DISABLE();                                  /*  �ر��ж�                    */
    
    ppcbFrom->PCB_usThreadCounter--;                                    /*  �������ȼ����ƿ��߳����� -- */
    ppcbTo->PCB_usThreadCounter++;                                      /*  �������ȼ����ƿ��߳����� ++ */
    
    ptcb->TCB_ucPriority = ucPriority;                                  /*  �����µ����ȼ�              */
    
    if (__LW_THREAD_IS_READY(ptcb)) {                                   /*  �߳̾���                    */
        LW_SPIN_LOCK_IGNIRQ(&_K_slScheduler);                           /*  ����������������            */
        if (ptcb->TCB_bIsCand) {                                        /*  �ں�ѡ����                  */
            ppcbFrom->PCB_usThreadReadyCounter--;                       /*  �������ȼ����ƿ�������� -- */
            ppcbTo->PCB_usThreadReadyCounter++;                         /*  �������ȼ����ƿ�������� ++ */
            
            ppcbFrom->PCB_usThreadCandCounter--;                        /*  �������ȼ����ƿ��ѡ���� -- */
            ppcbTo->PCB_usThreadCandCounter++;                          /*  �������ȼ����ƿ��ѡ���� ++ */
            LW_CAND_ROT(LW_CPU_GET(ptcb->TCB_ulCPUId)) =  LW_TRUE;      /*  �˳��ں�ʱ������ռ����      */
        
        } else {                                                        /*  ���ں�ѡ����                */
            __DEL_FROM_READY_RING_NOLOCK(ptcb, ppcbFrom);               /*  �Ӿ�������ɾ��              */
            __ADD_TO_READY_RING_NOLOCK(ptcb, ppcbTo);                   /*  �����µľ�����              */
        }
        LW_SPIN_UNLOCK_IGNIRQ(&_K_slScheduler);                         /*  ����������������            */
    }
    
    KN_INT_ENABLE(iregInterLevel);                                      /*  ���ж�                    */
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
