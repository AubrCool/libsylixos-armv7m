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
** ��   ��   ��: _CandTable.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2011 �� 12 �� 05 ��
**
** ��        ��: ����ϵͳ�ں˺�ѡ���б������(�� k_sched.h �е� inline �������󵽴�)

** BUG:
2013.07.29  ���ļ�����Ϊ _CandTable.c ��ʾ��ѡ���б����.
2014.01.10  ����ѡ����� CPU �ṹ��.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** ��������: _CandSeekThread
** ��������: �Ӿ�������ȷ��һ���������е��߳�.
** �䡡��  : ucPriority        ���ȼ�
** �䡡��  : �ھ�����������Ҫ���е��߳�.
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static PLW_CLASS_TCB  _CandSeekThread (UINT8  ucPriority)
{
    REGISTER PLW_CLASS_PCB    ppcb;
    REGISTER PLW_CLASS_TCB    ptcb;
    
    ppcb = _GetPcbByPrio(ucPriority);
    ptcb = _LIST_ENTRY(ppcb->PCB_pringReadyHeader, 
                       LW_CLASS_TCB, 
                       TCB_ringReady);                                  /*  �Ӿ�������ȡ��һ���߳�      */
    
    if (ptcb->TCB_ucSchedPolicy == LW_OPTION_SCHED_FIFO) {              /*  ����� FIFO ֱ������        */
        return  (ptcb);
    
    } else if (ptcb->TCB_usSchedCounter == 0) {                         /*  ȱ��ʱ��Ƭ                  */
        ptcb->TCB_usSchedCounter = ptcb->TCB_usSchedSlice;              /*  ����ʱ��Ƭ                  */
        _list_ring_next(&ppcb->PCB_pringReadyHeader);                   /*  ��һ��                      */
        ptcb = _LIST_ENTRY(ppcb->PCB_pringReadyHeader, 
                           LW_CLASS_TCB, 
                           TCB_ringReady);
    }
    
    return  (ptcb);
}
/*********************************************************************************************************
** ��������: _CandTableSelect
** ��������: ѡ��һ�����ִ���̷߳����ѡ��.
** �䡡��  : pcpu          CPU �ṹ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  _CandTableFill (PLW_CLASS_CPU   pcpu)
{
    REGISTER PLW_CLASS_TCB  ptcb;
    REGISTER PLW_CLASS_PCB  ppcb;
    REGISTER UINT8          ucPriority = _SchedSeekPriority();

    ptcb = _CandSeekThread(ucPriority);                                 /*  ȷ�����Ժ�ѡ���е��߳�      */
    ppcb = _GetPcb(ptcb);
    
    ppcb->PCB_usThreadCandCounter++;
    if (ppcb->PCB_usThreadCandCounter ==
        ppcb->PCB_usThreadReadyCounter) {                               /*  ���ȼ������߳̾��ں�ѡ���б�*/
        __DEL_RDY_MAP(ppcb);                                            /*  �Ӿ�������ɾ��              */
    }
    
    LW_CAND_TCB(pcpu) = ptcb;                                           /*  �����µĿ�ִ���߳�          */
    ptcb->TCB_ulCPUId = pcpu->CPU_ulCPUId;                              /*  ��¼ CPU ��                 */
    ptcb->TCB_bIsCand = LW_TRUE;                                        /*  �����ѡ���б�              */
    _DelTCBFromReadyRing(ptcb, ppcb);                                   /*  �Ӿ��������˳�              */
}
/*********************************************************************************************************
** ��������: _CandTableEmpty
** ��������: ����ѡ���еĺ�ѡ�̷߳��������.
** �䡡��  : pcpu      CPU �ṹ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  _CandTableEmpty (PLW_CLASS_CPU   pcpu)
{
    REGISTER PLW_CLASS_TCB  ptcb;
    REGISTER PLW_CLASS_PCB  ppcb;
    
    ptcb = LW_CAND_TCB(pcpu);
    ppcb = _GetPcb(ptcb);
    
    ppcb->PCB_usThreadCandCounter--;                                    /*  �����ȼ���ѡ�߳���--        */
    if ((ppcb->PCB_usThreadReadyCounter - 
         ppcb->PCB_usThreadCandCounter) == 1) {
        __ADD_RDY_MAP(ppcb);                                            /*  ��ǰ�̱߳���ռ, �ػؾ�����  */
    }
    
    ptcb->TCB_bIsCand = LW_FALSE;
    _AddTCBToReadyRing(ptcb, ppcb, LW_TRUE);                            /*  ���¼��������              */
                                                                        /*  ���������ͷ, �´����ȵ���  */
    LW_CAND_TCB(pcpu) = LW_NULL;
}
/*********************************************************************************************************
** ��������: _CandTableTryAdd
** ��������: ��ͼ��һ�������߳�װ���ѡ�̱߳�.
** �䡡��  : ptcb          �������߳�
**           ppcb          ��Ӧ�����ȼ����ƿ�
** �䡡��  : �Ƿ�����˺�ѡ���б�
** ȫ�ֱ���: 
** ����ģ��: 
** ˵  ��  : ����ϵͳ�ɲ���Ҫʹ�����Ʊ�־, ����������ѡ��.
*********************************************************************************************************/
BOOL  _CandTableTryAdd (PLW_CLASS_TCB  ptcb, PLW_CLASS_PCB  ppcb)
{
    REGISTER PLW_CLASS_CPU   pcpu;
    REGISTER PLW_CLASS_TCB   ptcbCand;
    
#if LW_CFG_SMP_EN > 0                                                   /*  SMP ���                    */
    REGISTER ULONG           i;

    for (i = 0; i < LW_NCPUS; i++) {
        pcpu = LW_CPU_GET(i);
        if (!LW_CPU_IS_ACTIVE(pcpu)) {                                  /*  CPU ����Ϊ����״̬          */
            continue;
        }
        
        ptcbCand = LW_CAND_TCB(pcpu);
        if (ptcbCand == LW_NULL) {                                      /*  ��ѡ��Ϊ��                  */
            LW_CAND_TCB(pcpu) = ptcb;
            ptcb->TCB_ulCPUId = i;                                      /*  ��¼ CPU ��                 */
            ptcb->TCB_bIsCand = LW_TRUE;                                /*  �����ѡ���б�              */
            ppcb->PCB_usThreadCandCounter++;
            return  (LW_TRUE);
        
        } else if (LW_PRIO_IS_HIGH(ptcb->TCB_ucPriority, 
                                   ptcbCand->TCB_ucPriority)) {         /*  ���ȼ����ڵ�ǰ��ѡ�߳�      */
            LW_CAND_ROT(pcpu) = LW_TRUE;                                /*  �������ȼ�����              */
        }
    }
    
    if ((ppcb->PCB_usThreadReadyCounter - 
         ppcb->PCB_usThreadCandCounter) == 1) {
        __ADD_RDY_MAP(ppcb);                                            /*  ��λͼ�����λ��һ          */
    }
    
    return  (LW_FALSE);                                                 /*  �޷������ѡ���б�          */

#else                                                                   /*  �������                    */
    pcpu = LW_CPU_GET(0);
    if (!LW_CPU_IS_ACTIVE(pcpu)) {                                      /*  CPU ����Ϊ����״̬          */
        goto    __can_not_cand;
    }
    
    ptcbCand = LW_CAND_TCB(pcpu);
    if (ptcbCand == LW_NULL) {                                          /*  ��ѡ��Ϊ��                  */
__can_cand:
        LW_CAND_TCB(pcpu) = ptcb;
        ptcb->TCB_ulCPUId = 0;                                          /*  ��¼ CPU ��                 */
        ptcb->TCB_bIsCand = LW_TRUE;                                    /*  �����ѡ���б�              */
        ppcb->PCB_usThreadCandCounter++;
        return  (LW_TRUE);                                              /*  ֱ�Ӽ����ѡ���б�          */
        
    } else if (LW_PRIO_IS_HIGH(ptcb->TCB_ucPriority, 
                               ptcbCand->TCB_ucPriority)) {             /*  ���ȼ��ȵ�ǰ��ѡ�̸߳�      */
        if (__THREAD_LOCK_GET(ptcbCand)) {
            LW_CAND_ROT(pcpu) = LW_TRUE;                                /*  �������ȼ�����              */
        
        } else {
            _CandTableEmpty(pcpu);                                      /*  ��պ�ѡ��                  */
            goto    __can_cand;
        }
    }
    
__can_not_cand:
    if ((ppcb->PCB_usThreadReadyCounter - 
         ppcb->PCB_usThreadCandCounter) == 1) {
        __ADD_RDY_MAP(ppcb);                                            /*  ��λͼ�����λ��һ          */
    }
    
    return  (LW_FALSE);
#endif                                                                  /*  LW_CFG_SMP_EN               */
}
/*********************************************************************************************************
** ��������: _CandTableTryDel
** ��������: ��ͼ�ӽ�һ�����پ����ĺ�ѡ�̴߳Ӻ�ѡ�����˳�
** �䡡��  : ptcb          �������߳�
**           ppcb          ��Ӧ�����ȼ����ƿ�
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID _CandTableTryDel (PLW_CLASS_TCB  ptcb, PLW_CLASS_PCB  ppcb)
{
    REGISTER PLW_CLASS_CPU   pcpu = LW_CPU_GET(ptcb->TCB_ulCPUId);
    
    if (LW_CAND_TCB(pcpu) == ptcb) {                                    /*  �ں�ѡ����                  */
        ptcb->TCB_bIsCand = LW_FALSE;                                   /*  �˳���ѡ���б�              */
        ppcb->PCB_usThreadCandCounter--;                                /*  ��ѡ���� --                 */
        _CandTableFill(pcpu);                                           /*  �������һ����ѡ�߳�        */
        LW_CAND_ROT(pcpu) = LW_FALSE;                                   /*  ������ȼ����Ʊ�־          */
    
    } else {                                                            /*  û���ں�ѡ����              */
        if (ppcb->PCB_usThreadReadyCounter == 
            ppcb->PCB_usThreadCandCounter) {
            __DEL_RDY_MAP(ppcb);                                        /*  ��λͼ�����λ����          */
        }
    }
}
/*********************************************************************************************************
** ��������: _CandTableUpdate
** ��������: ���Խ�������ȼ���������װ���ѡ��. 
** �䡡��  : pcpu      CPU �ṹ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID _CandTableUpdate (PLW_CLASS_CPU   pcpu)
{
    REGISTER UINT8          ucPriority;
    REGISTER PLW_CLASS_TCB  ptcbCand;
             BOOL           bNeedRotate = LW_FALSE;

    if (!LW_CPU_IS_ACTIVE(pcpu)) {                                      /*  CPU ����Ϊ����״̬          */
        return;
    }
    
    ptcbCand = LW_CAND_TCB(pcpu);
    if (ptcbCand == LW_NULL) {                                          /*  ��ǰû�к�ѡ�߳�            */
        _CandTableFill(pcpu);
        return;
    }
    
    ucPriority = _SchedSeekPriority();                                  /*  ��ǰ��������������ȼ�      */
    if (ptcbCand->TCB_usSchedCounter == 0) {                            /*  �Ѿ�û��ʱ��Ƭ��            */
        if (LW_PRIO_IS_HIGH_OR_EQU(ucPriority, 
                                   ptcbCand->TCB_ucPriority)) {         /*  �Ƿ���Ҫ��ת                */
            bNeedRotate = LW_TRUE;
        }
    } else {
        if (LW_PRIO_IS_HIGH(ucPriority, 
                            ptcbCand->TCB_ucPriority)) {
            bNeedRotate = LW_TRUE;
        }
    }
    
    if (bNeedRotate) {                                                  /*  ���ڸ���Ҫ���е��߳�        */
        _CandTableEmpty(pcpu);                                          /*  ��պ�ѡ��                  */
        _CandTableFill(pcpu);                                           /*  ��������ѡ��              */
    }
    
    LW_CAND_ROT(pcpu) = LW_FALSE;                                       /*  ������ȼ����Ʊ�־          */
}
/*********************************************************************************************************
** ��������: _CandTableRemove
** ��������: ��һ�� CPU ��Ӧ�ĺ�ѡ����� 
** �䡡��  : pcpu      CPU �ṹ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID _CandTableRemove (PLW_CLASS_CPU   pcpu)
{
    if (LW_CPU_IS_ACTIVE(pcpu)) {                                       /*  CPU ����Ϊ�Ǽ���״̬        */
        return;
    }
    
    if (LW_CAND_TCB(pcpu)) {
        _CandTableEmpty(pcpu);
    }
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
