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
** ��   ��   ��: TimeSleep.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2006 �� 12 �� 14 ��
**
** ��        ��: �߳�˯�ߺ�����

** BUG
2008.03.29  ʹ���µĵȴ�����.
2008.03.30  ʹ���µľ���������.
2008.04.12  ������źŵ�֧��.
2008.04.13  ���ںܶ��ں˻�����Ҫ��ȷ���ӳ�, ������������ API_TimeSleep �� posix sleep.
            API_TimeSleep �ǲ��ܱ��źŻ��ѵ�, �� posix sleep �ǿ��Ա��źŻ��ѵ�.
2009.10.12  ����� SMP ��˵�֧��.
2012.03.20  ���ٶ� _K_ptcbTCBCur ������, �������þֲ�����, ���ٶԵ�ǰ CPU ID ��ȡ�Ĵ���.
2013.07.10  ȥ�� API_TimeUSleep API.
            API_TimeMSleep ���ٱ�֤һ�� tick �ӳ�.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** ��������: API_TimeSleep
** ��������: �߳�˯�ߺ��� (�� API ���ܱ��źŻ���)
** �䡡��  : ulTick            ˯�ߵ�ʱ��
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
                                           
                                       (�������ж��е���)
*********************************************************************************************************/
LW_API
VOID  API_TimeSleep (ULONG    ulTick)
{
             INTREG                iregInterLevel;
             
             PLW_CLASS_TCB         ptcbCur;
	REGISTER PLW_CLASS_PCB         ppcb;
	REGISTER ULONG                 ulKernelTime;
    
    if (LW_CPU_GET_CUR_NESTING()) {                                     /*  �������ж��е���            */
        _ErrorHandle(ERROR_KERNEL_IN_ISR);
        return;
    }
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);                                       /*  ��ǰ������ƿ�              */
    
    MONITOR_EVT_LONG2(MONITOR_EVENT_ID_THREAD, MONITOR_EVENT_THREAD_SLEEP, 
                      ptcbCur->TCB_ulId, ulTick, LW_NULL);
    
__wait_again:
    if (!ulTick) {                                                      /*  �������ӳ�                  */
        _ErrorHandle(ERROR_NONE);
        return;
    }

    iregInterLevel = __KERNEL_ENTER_IRQ();                              /*  �����ں�                    */

    ppcb = _GetPcb(ptcbCur);
    __DEL_FROM_READY_RING(ptcbCur, ppcb);                               /*  �Ӿ�������ɾ��              */
    
    ptcbCur->TCB_ulDelay = ulTick;
    __ADD_TO_WAKEUP_LINE(ptcbCur);                                      /*  ����ȴ�ɨ����              */
    
    __KERNEL_TIME_GET(ulKernelTime, ULONG);                             /*  ��¼ϵͳʱ��                */
    
    if (__KERNEL_EXIT_IRQ(iregInterLevel)) {                            /*  ���źż���                  */
        ulTick = _sigTimeOutRecalc(ulKernelTime, ulTick);               /*  ���¼���ȴ�ʱ��            */
        goto __wait_again;                                              /*  �����ȴ�                    */
    }
    
    _ErrorHandle(ERROR_NONE);
    return;
}
/*********************************************************************************************************
** ��������: API_TimeSSleep
** ��������: �߳�˯�ߺ���
** �䡡��  : ulSeconds            ˯�ߵ�ʱ�� (��)
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
                                           
                                       (�������ж��е���)
*********************************************************************************************************/
LW_API  
VOID    API_TimeSSleep (ULONG   ulSeconds)
{
    API_TimeSleep(ulSeconds * LW_CFG_TICKS_PER_SEC);
}
/*********************************************************************************************************
** ��������: API_TimeMSleep
** ��������: �߳�˯�ߺ���
** �䡡��  : ulMSeconds            ˯�ߵ�ʱ�� (����)
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
                                           
                                       (�������ж��е���)
*********************************************************************************************************/
LW_API  
VOID    API_TimeMSleep (ULONG   ulMSeconds)
{
    REGISTER ULONG      ulTicks;
    
    if (ulMSeconds == 0) {
        return;
    }
    
    ulTicks = LW_MSECOND_TO_TICK_1(ulMSeconds);
    
    API_TimeSleep(ulTicks);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
