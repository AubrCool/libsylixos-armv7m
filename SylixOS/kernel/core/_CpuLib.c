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
** ��   ��   ��: _CpuLib.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2014 �� 01 �� 10 ��
**
** ��        ��: CPU ������.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** ��������: _CpuActive
** ��������: �� CPU ����Ϊ����״̬ (�ر��ж�״̬�±�����)
** �䡡��  : pcpu      CPU �ṹ
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : ���뱣֤ pcpu ��ǰִ���߳���һ����Ч�� TCB ���� _K_tcbDummyKernel ��������
*********************************************************************************************************/
INT  _CpuActive (PLW_CLASS_CPU   pcpu)
{
    PLW_CLASS_TCB   ptcbOrg;

    if (LW_CPU_IS_ACTIVE(pcpu)) {
        return  (PX_ERROR);
    }
    
    LW_TCB_GET_CUR(ptcbOrg);
    
    LW_SPIN_LOCK_IGNIRQ(&_K_slScheduler);
    
    pcpu->CPU_ulStatus |= LW_CPU_STATUS_ACTIVE;
    KN_SMP_MB();
    
    _CandTableUpdate(pcpu);                                             /*  ���º�ѡ�߳�                */

    pcpu->CPU_ptcbTCBCur  = LW_CAND_TCB(pcpu);
    pcpu->CPU_ptcbTCBHigh = LW_CAND_TCB(pcpu);
    
    LW_SPIN_UNLOCK_SCHED(&_K_slScheduler, ptcbOrg);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: _CpuInactiveNoLock
** ��������: �� CPU ����Ϊ�Ǽ���״̬ (�������������ر��ж�״̬�±�����)
** �䡡��  : pcpu      CPU �ṹ
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
INT  _CpuInactiveNoLock (PLW_CLASS_CPU   pcpu)
{
    INT             i;
    ULONG           ulCPUId = pcpu->CPU_ulCPUId;
    PLW_CLASS_TCB   ptcb;

    if (!LW_CPU_IS_ACTIVE(pcpu)) {
        return  (PX_ERROR);
    }
    
    ptcb = LW_CAND_TCB(pcpu);
    
    pcpu->CPU_ulStatus &= ~LW_CPU_STATUS_ACTIVE;
    KN_SMP_MB();
    
    _CandTableRemove(pcpu);                                             /*  �Ƴ���ѡִ���߳�            */
    LW_CAND_ROT(pcpu) = LW_FALSE;                                       /*  ������ȼ����Ʊ�־          */

    pcpu->CPU_ptcbTCBCur  = LW_NULL;
    pcpu->CPU_ptcbTCBHigh = LW_NULL;
    
    for (i = 0; i < LW_NCPUS; i++) {                                    /*  �������� CPU ����           */
        if (ulCPUId != i) {
            PLW_CLASS_CPU   pcpuOther = LW_CPU_GET(i);
            PLW_CLASS_TCB   ptcbCand  = LW_CAND_TCB(pcpuOther);
            if (LW_PRIO_IS_HIGH(ptcb->TCB_ucPriority,
                                ptcbCand->TCB_ucPriority)) {            /*  ��ǰ�˳����������ȼ���      */
                LW_CAND_ROT(pcpuOther) = LW_TRUE;
            }
        }
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
