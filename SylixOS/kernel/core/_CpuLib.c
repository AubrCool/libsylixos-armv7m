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
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : ���뱣֤ pcpu ��ǰִ���߳���һ����Ч�� TCB ���� _K_tcbDummyKernel ��������
*********************************************************************************************************/
VOID  _CpuActive (PLW_CLASS_CPU   pcpu)
{
    PLW_CLASS_TCB   ptcbOrg;

    if (LW_CPU_IS_ACTIVE(pcpu)) {
        return;
    }
    
    LW_TCB_GET_CUR(ptcbOrg);
    
    LW_SPIN_LOCK_IGNIRQ(&_K_slScheduler);
    
    pcpu->CPU_ulStatus |= LW_CPU_STATUS_ACTIVE;
    KN_SMP_MB();
    
    _CandTableUpdate(pcpu);                                             /*  ���º�ѡ�߳�                */

    pcpu->CPU_ptcbTCBCur  = LW_CAND_TCB(pcpu);
    pcpu->CPU_ptcbTCBHigh = LW_CAND_TCB(pcpu);
    
    LW_SPIN_UNLOCK_SCHED(&_K_slScheduler, ptcbOrg);
}
/*********************************************************************************************************
** ��������: _CpuInactive
** ��������: �� CPU ����Ϊ�Ǽ���״̬ (�ر��ж�״̬�±�����)
** �䡡��  : pcpu      CPU �ṹ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _CpuInactive (PLW_CLASS_CPU   pcpu)
{
    PLW_CLASS_TCB   ptcbOrg;

    if (!LW_CPU_IS_ACTIVE(pcpu)) {
        return;
    }
    
    LW_TCB_GET_CUR(ptcbOrg);
    
    LW_SPIN_LOCK_IGNIRQ(&_K_slScheduler);
    
    pcpu->CPU_ulStatus &= ~LW_CPU_STATUS_ACTIVE;
    KN_SMP_MB();
    
    _CandTableRemove(pcpu);                                             /*  �Ƴ���ѡִ���߳�            */
    LW_CAND_ROT(pcpu) = LW_FALSE;                                       /*  ������ȼ����Ʊ�־          */

    pcpu->CPU_ptcbTCBCur  = LW_NULL;
    pcpu->CPU_ptcbTCBHigh = LW_NULL;
    
    LW_SPIN_UNLOCK_SCHED(&_K_slScheduler, ptcbOrg);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
