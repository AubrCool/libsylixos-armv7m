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
** ��   ��   ��: _ThreadJoinLib.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2006 �� 12 �� 18 ��
**
** ��        ��: �̺߳ϲ��ͽ���CORE������

** BUG
2007.11.13  ʹ���������������������ȫ��װ.
2007.11.13  ���� TCB_ptcbJoin ��Ϣ��¼���ж�.
2008.03.30  ʹ���µľ���������.
2010.08.03  ���ⲿ��ʹ�õĺ�����Ϊ static ����, 
            ����ĺ����������ں�����ģʽ�µ��õ�, ����ֻ��ر��жϾͿɱ��� SMP ��ռ.
2012.03.20  ���ٶ� _K_ptcbTCBCur ������, �������þֲ�����, ���ٶԵ�ǰ CPU ID ��ȡ�Ĵ���.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** ��������: _ThreadJoinSuspend
** ��������: �̺߳ϲ��������Լ� (�ڽ����ں˲����жϺ󱻵���)
** �䡡��  : ptcbCur   ��ǰ������ƿ�
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  _ThreadJoinSuspend (PLW_CLASS_TCB  ptcbCur)
{
    REGISTER PLW_CLASS_PCB    ppcbMe;
             
    ppcbMe = _GetPcb(ptcbCur);
    
    ptcbCur->TCB_ulSuspendNesting++;
    __DEL_FROM_READY_RING(ptcbCur, ppcbMe);                             /*  �Ӿ�������ɾ��              */
    ptcbCur->TCB_usStatus |= LW_THREAD_STATUS_SUSPEND;
}
/*********************************************************************************************************
** ��������: _ThreadJoinResume
** ��������: ���������߳� (�ڽ����ں˺󱻵���)
** �䡡��  : ptcb      ������ƿ�
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  _ThreadJoinResume (PLW_CLASS_TCB  ptcb)
{
             INTREG                iregInterLevel;
    REGISTER PLW_CLASS_PCB         ppcb;
    
    ptcb->TCB_ptcbJoin = LW_NULL;                                       /*  �����¼�ĵȴ��߳� tcb      */
    
    ppcb = _GetPcb(ptcb);
    
    iregInterLevel = KN_INT_DISABLE();
    
    if (ptcb->TCB_ulSuspendNesting) {
        ptcb->TCB_ulSuspendNesting--;
    
    } else {
        KN_INT_ENABLE(iregInterLevel);
        return;
    }
    
    if (ptcb->TCB_ulSuspendNesting) {                                   /*  ����Ƕ���û������������    */
        KN_INT_ENABLE(iregInterLevel);
        return;
    }

    ptcb->TCB_usStatus &= (~LW_THREAD_STATUS_SUSPEND);
    
    if (__LW_THREAD_IS_READY(ptcb)) {
       ptcb->TCB_ucSchedActivate = LW_SCHED_ACT_INTERRUPT;              /*  �жϼ��ʽ                */
       __ADD_TO_READY_RING(ptcb, ppcb);                                 /*  ���������                  */
    }
    
    KN_INT_ENABLE(iregInterLevel);   
}
/*********************************************************************************************************
** ��������: _ThreadJoin
** ��������: ����ǰ�̺߳ϲ��������߳� (�ڽ����ں˺󱻵���)
** �䡡��  : ptcbDes          Ҫ�ϲ����߳�
**           ppvRetValSave    Ŀ���߳̽���ʱ�ķ���ֵ��ŵ�ַ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _ThreadJoin (PLW_CLASS_TCB  ptcbDes, PVOID  *ppvRetValSave)
{
    INTREG                iregInterLevel;
    PLW_CLASS_TCB         ptcbCur;
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);                                       /*  ��ǰ������ƿ�              */
    
    MONITOR_EVT_LONG2(MONITOR_EVENT_ID_THREAD, MONITOR_EVENT_THREAD_JOIN, 
                      ptcbCur->TCB_ulId, ptcbDes->TCB_ulId, LW_NULL);
    
    ptcbCur->TCB_ppvJoinRetValSave = ppvRetValSave;                     /*  �����ŷ���ֵ�ĵ�ַ        */
                                                                        /*  ����ȴ�����                */
    iregInterLevel = KN_INT_DISABLE();
    
    _List_Line_Add_Ahead(&ptcbCur->TCB_lineJoin, &ptcbDes->TCB_plineJoinHeader);
    
    ptcbCur->TCB_ptcbJoin = ptcbDes;                                    /*  ��¼Ŀ���߳�                */
    
    _ThreadJoinSuspend(ptcbCur);                                        /*  �����Լ��ȴ��Է�����        */
    
    KN_INT_ENABLE(iregInterLevel);
}
/*********************************************************************************************************
** ��������: _ThreadReleaseAllJoin
** ��������: ָ���߳̽���ϲ������������߳� (�ڽ����ں˺󱻵���)
** �䡡��  : ptcbDes          Ҫ�ϲ����߳�
**           pvArg            ����ֵ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _ThreadReleaseAllJoin (PLW_CLASS_TCB  ptcbDes, PVOID  pvArg)
{
    REGISTER PLW_LIST_LINE         plineList;
    REGISTER PLW_CLASS_TCB         ptcbJoin;
    
    for (plineList  = ptcbDes->TCB_plineJoinHeader;
         plineList != LW_NULL;
         plineList  = _list_line_get_next(plineList)) {
         
        ptcbJoin = _LIST_ENTRY(plineList, LW_CLASS_TCB, TCB_lineJoin);  /*  �ҵ��ϲ������̵߳�TCB       */
         
        if (ptcbJoin->TCB_ppvJoinRetValSave) {                          /*  �ȴ�����ֵ                  */
            *ptcbJoin->TCB_ppvJoinRetValSave = pvArg;                   /*  ���淵��ֵ                  */
        }
         
        MONITOR_EVT_LONG1(MONITOR_EVENT_ID_THREAD, MONITOR_EVENT_THREAD_DETACH, 
                          ptcbJoin->TCB_ulId, LW_NULL);
                           
        _List_Line_Del(&ptcbJoin->TCB_lineJoin, &ptcbDes->TCB_plineJoinHeader);

        _ThreadJoinResume(ptcbJoin);                                    /*  �ͷźϲ����߳�              */
    }
}
/*********************************************************************************************************
** ��������: _ThreadDetach
** ��������: ָ���߳̽���ϲ������������̲߳������������̺߳ϲ��Լ� (�ڽ����ں˺󱻵���)
** �䡡��  : ptcbDes          Ҫ�ϲ����߳�
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _ThreadDetach (PLW_CLASS_TCB  ptcbDes)
{
    if (ptcbDes->TCB_plineJoinHeader) {                                 /*  �Ѿ����̺߳ϲ�              */
        _ThreadReleaseAllJoin(ptcbDes, LW_NULL);                        /*  ����                        */
    }
    
    ptcbDes->TCB_bDetachFlag = LW_TRUE;                                 /*  �Ͻ��ϲ��Լ�                */
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
