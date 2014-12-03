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
2014.12.03  ����ʹ�� suspend ����, ת��ʹ�� LW_THREAD_STATUS_JOIN.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** ��������: _ThreadJoinWait
** ��������: �̺߳ϲ��������Լ� (�ڽ����ں˲����жϺ󱻵���)
** �䡡��  : ptcbCur   ��ǰ������ƿ�
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  _ThreadJoinWait (PLW_CLASS_TCB  ptcbCur)
{
    REGISTER PLW_CLASS_PCB    ppcbMe;
             
    ppcbMe = _GetPcb(ptcbCur);
    __DEL_FROM_READY_RING(ptcbCur, ppcbMe);                             /*  �Ӿ�������ɾ��              */
    ptcbCur->TCB_usStatus |= LW_THREAD_STATUS_JOIN;                     /*  ����Ϊ join ״̬            */
}
/*********************************************************************************************************
** ��������: _ThreadJoinWakeup
** ��������: ���������߳� (�ڽ����ں˺󱻵���)
** �䡡��  : ptcb      ������ƿ�
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  _ThreadJoinWakeup (PLW_CLASS_TCB  ptcb)
{
             INTREG                iregInterLevel;
    REGISTER PLW_CLASS_PCB         ppcb;
    
    ptcb->TCB_ptcbJoin = LW_NULL;                                       /*  �����¼�ĵȴ��߳� tcb      */

    iregInterLevel = KN_INT_DISABLE();

    ptcb->TCB_usStatus &= (~LW_THREAD_STATUS_JOIN);
    
    if (__LW_THREAD_IS_READY(ptcb)) {
       ptcb->TCB_ucSchedActivate = LW_SCHED_ACT_INTERRUPT;              /*  �жϼ��ʽ                */
       ppcb = _GetPcb(ptcb);
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
    
    _ThreadJoinWait(ptcbCur);                                           /*  �����Լ��ȴ��Է�����        */
    
    KN_INT_ENABLE(iregInterLevel);
}
/*********************************************************************************************************
** ��������: _ThreadReleaseAllJoin
** ��������: ָ���߳̽���ϲ������������߳� (�ڽ����ں˺󱻵���)
** �䡡��  : ptcbDes          �ϲ���Ŀ���߳�
**           ptcbWakeup       ��Ҫ���ѵ��߳�
**           pvArg            ����ֵ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _ThreadReleaseJoin (PLW_CLASS_TCB  ptcbDes, PLW_CLASS_TCB  ptcbWakeup, PVOID  pvArg)
{
    if (ptcbWakeup->TCB_ppvJoinRetValSave) {                            /*  �ȴ�����ֵ                  */
        *ptcbWakeup->TCB_ppvJoinRetValSave = pvArg;                     /*  ���淵��ֵ                  */
    }
    
    MONITOR_EVT_LONG2(MONITOR_EVENT_ID_THREAD, MONITOR_EVENT_THREAD_DETACH, 
                      ptcbWakeup->TCB_ulId, pvArg, LW_NULL);
                      
    _List_Line_Del(&ptcbWakeup->TCB_lineJoin, &ptcbDes->TCB_plineJoinHeader);
    
    _ThreadJoinWakeup(ptcbWakeup);                                      /*  �ͷźϲ����߳�              */
}
/*********************************************************************************************************
** ��������: _ThreadReleaseAllJoin
** ��������: ָ���߳̽���ϲ������������߳� (�ڽ����ں˺󱻵���)
** �䡡��  : ptcbDes          �ϲ���Ŀ���߳�
**           pvArg            ����ֵ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _ThreadReleaseAllJoin (PLW_CLASS_TCB  ptcbDes, PVOID  pvArg)
{
    REGISTER PLW_CLASS_TCB  ptcbJoin;
    
    while (ptcbDes->TCB_plineJoinHeader) {
        ptcbJoin = _LIST_ENTRY(ptcbDes->TCB_plineJoinHeader, 
                               LW_CLASS_TCB, TCB_lineJoin);
        _ThreadReleaseJoin(ptcbDes, ptcbJoin, pvArg);
    }
}
/*********************************************************************************************************
** ��������: _ThreadDetach
** ��������: ָ���߳̽���ϲ������������̲߳������������̺߳ϲ��Լ� (�ڽ����ں˺󱻵���)
** �䡡��  : ptcbDes          �ϲ���Ŀ���߳�
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
