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
  s_internal.h ��Ҳ����ض���
*********************************************************************************************************/
#if LW_CFG_THREAD_CANCEL_EN > 0
#define __THREAD_CANCEL_POINT()         API_ThreadTestCancel()
#else
#define __THREAD_CANCEL_POINT()
#endif                                                                  /*  LW_CFG_THREAD_CANCEL_EN     */
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
        return;
    }

    iregInterLevel = __KERNEL_ENTER_IRQ();                              /*  �����ں�                    */

    ppcb = _GetPcb(ptcbCur);
    __DEL_FROM_READY_RING(ptcbCur, ppcb);                               /*  �Ӿ�������ɾ��              */
    
    ptcbCur->TCB_ulDelay = ulTick;
    __ADD_TO_WAKEUP_LINE(ptcbCur);                                      /*  ����ȴ�ɨ����              */
    
    __KERNEL_TIME_GET_NO_SPINLOCK(ulKernelTime, ULONG);                 /*  ��¼ϵͳʱ��                */
    
    if (__KERNEL_EXIT_IRQ(iregInterLevel)) {                            /*  ���źż���                  */
        ulTick = _sigTimeOutRecalc(ulKernelTime, ulTick);               /*  ���¼���ȴ�ʱ��            */
        goto __wait_again;                                              /*  �����ȴ�                    */
    }
}
/*********************************************************************************************************
** ��������: API_TimeSleepEx
** ��������: �߳�˯�ߺ���
** �䡡��  : ulTick            ˯�ߵ�ʱ��
**           bSigRet           �Ƿ������źŻ���
** �䡡��  : ERROR_NONE or EINTR
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
                                           
                                       (�������ж��е���)
*********************************************************************************************************/
LW_API
ULONG  API_TimeSleepEx (ULONG  ulTick, BOOL  bSigRet)
{
             INTREG                iregInterLevel;
             
             PLW_CLASS_TCB         ptcbCur;
	REGISTER PLW_CLASS_PCB         ppcb;
	REGISTER ULONG                 ulKernelTime;
    
    if (LW_CPU_GET_CUR_NESTING()) {                                     /*  �������ж��е���            */
        _ErrorHandle(ERROR_KERNEL_IN_ISR);
        return  (ERROR_KERNEL_IN_ISR);
    }
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);                                       /*  ��ǰ������ƿ�              */
    
    MONITOR_EVT_LONG2(MONITOR_EVENT_ID_THREAD, MONITOR_EVENT_THREAD_SLEEP, 
                      ptcbCur->TCB_ulId, ulTick, LW_NULL);
    
__wait_again:
    if (!ulTick) {                                                      /*  �������ӳ�                  */
        return  (ERROR_NONE);
    }

    iregInterLevel = __KERNEL_ENTER_IRQ();                              /*  �����ں�                    */

    ppcb = _GetPcb(ptcbCur);
    __DEL_FROM_READY_RING(ptcbCur, ppcb);                               /*  �Ӿ�������ɾ��              */
    
    ptcbCur->TCB_ulDelay = ulTick;
    __ADD_TO_WAKEUP_LINE(ptcbCur);                                      /*  ����ȴ�ɨ����              */
    
    __KERNEL_TIME_GET_NO_SPINLOCK(ulKernelTime, ULONG);                 /*  ��¼ϵͳʱ��                */
    
    if (__KERNEL_EXIT_IRQ(iregInterLevel)) {                            /*  ���źż���                  */
        if (bSigRet) {
            _ErrorHandle(EINTR);
            return  (EINTR);
        }
        ulTick = _sigTimeOutRecalc(ulKernelTime, ulTick);               /*  ���¼���ȴ�ʱ��            */
        goto __wait_again;                                              /*  �����ȴ�                    */
    }
    
    return  (ERROR_NONE);
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
** ��������: sleep 
** ��������: sleep()����Ŀǰ���߳���ͣ��ֱ���ﵽ���� uiSeconds ��ָ����ʱ�䣬���Ǳ��ź����жϡ�(POSIX)
** �䡡��  : uiSeconds         ˯�ߵ�����
** �䡡��  : ��������ͣ������ uiSeconds ��ָ����ʱ���򷵻� 0�������ź��ж��򷵻�ʣ��������

             error == EINTR    ��ʾ���źż���.
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
                                           
                                       (�������ж��е���)
*********************************************************************************************************/
LW_API  
UINT  sleep (UINT    uiSeconds)
{
             INTREG                iregInterLevel;
             
             PLW_CLASS_TCB         ptcbCur;
    REGISTER PLW_CLASS_PCB         ppcb;
	REGISTER ULONG                 ulKernelTime;
	         INT                   iSchedRet;
	         
	         ULONG                 ulTick = (ULONG)(uiSeconds * LW_CFG_TICKS_PER_SEC);
    
    if (LW_CPU_GET_CUR_NESTING()) {                                     /*  �������ж��е���            */
        _ErrorHandle(ERROR_KERNEL_IN_ISR);
        return  (0);
    }
    
__wait_again:
    if (!ulTick) {                                                      /*  �������ӳ�                  */
        return  (0);
    }
    
    __THREAD_CANCEL_POINT();                                            /*  ����ȡ����                  */
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);                                       /*  ��ǰ������ƿ�              */
    
    MONITOR_EVT_LONG2(MONITOR_EVENT_ID_THREAD, MONITOR_EVENT_THREAD_SLEEP, 
                      ptcbCur->TCB_ulId, ulTick, LW_NULL);
    
    iregInterLevel = __KERNEL_ENTER_IRQ();                              /*  �����ں�                    */
    
    ppcb = _GetPcb(ptcbCur);
    __DEL_FROM_READY_RING(ptcbCur, ppcb);                               /*  �Ӿ�������ɾ��              */
    
    ptcbCur->TCB_ulDelay = ulTick;
    __ADD_TO_WAKEUP_LINE(ptcbCur);                                      /*  ����ȴ�ɨ����              */
    
    __KERNEL_TIME_GET_NO_SPINLOCK(ulKernelTime, ULONG);                 /*  ��¼ϵͳʱ��                */
    
    iSchedRet = __KERNEL_EXIT_IRQ(iregInterLevel);                      /*  ����������                  */
    if (iSchedRet == LW_SIGNAL_EINTR) {
        ulTick = _sigTimeOutRecalc(ulKernelTime, ulTick);               /*  ���¼���ȴ�ʱ��            */
        uiSeconds = (UINT)(ulTick / LW_CFG_TICKS_PER_SEC);
        
        _ErrorHandle(EINTR);                                            /*  ���źż���                  */
        return  (uiSeconds);                                            /*  ʣ������                    */
        
    } else if (iSchedRet == LW_SIGNAL_RESTART) {
        ulTick = _sigTimeOutRecalc(ulKernelTime, ulTick);               /*  ���¼���ȴ�ʱ��            */
        goto    __wait_again;
    }
    
    return  (0);
}
/*********************************************************************************************************
** ��������: nanosleep 
** ��������: ʹ���ô˺������߳�˯��һ��ָ����ʱ��, ˯�߹����п��ܱ��źž���!!! (POSIX)
** �䡡��  : rqtp         ˯�ߵ�ʱ��
**           rmtp         ����ʣ��ʱ��Ľṹ.
** �䡡��  : ERROR_NONE  or  PX_ERROR

             error == EINTR    ��ʾ���źż���.
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
                                           
                                       (�������ж��е���)
*********************************************************************************************************/
LW_API  
INT  nanosleep (const struct timespec  *rqtp, struct timespec  *rmtp)
{
             INTREG                iregInterLevel;
             
             PLW_CLASS_TCB         ptcbCur;
    REGISTER PLW_CLASS_PCB         ppcb;
	REGISTER ULONG                 ulKernelTime;
	REGISTER INT                   iRetVal;
	         INT                   iSchedRet;
	
	REGISTER ULONG                 ulError;
             ULONG                 ulTick;
             
    if (!rqtp) {                                                        /*  ָ��ʱ��Ϊ��                */
        _ErrorHandle(EINVAL);
        return (PX_ERROR);
    }
    if (rqtp->tv_nsec >= __TIMEVAL_NSEC_MAX) {                          /*  ʱ���ʽ����                */
        _ErrorHandle(EINVAL);
        return (PX_ERROR);
    }
    
    ulTick = __timespecToTick((struct timespec *)rqtp);
    if (!ulTick) {
        ulTick = 1;                                                     /*  �����ӳ�һ�� tick           */
    }
    
__wait_again:
    if (!ulTick) {                                                      /*  �������ӳ�                  */
        return  (ERROR_NONE);
    }
    
    __THREAD_CANCEL_POINT();                                            /*  ����ȡ����                  */
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);                                       /*  ��ǰ������ƿ�              */
    
    MONITOR_EVT_LONG2(MONITOR_EVENT_ID_THREAD, MONITOR_EVENT_THREAD_SLEEP, 
                      ptcbCur->TCB_ulId, ulTick, LW_NULL);
    
    iregInterLevel = __KERNEL_ENTER_IRQ();                              /*  �����ں�                    */
    
    ppcb = _GetPcb(ptcbCur);
    __DEL_FROM_READY_RING(ptcbCur, ppcb);                               /*  �Ӿ�������ɾ��              */
    
    ptcbCur->TCB_ulDelay = ulTick;
    __ADD_TO_WAKEUP_LINE(ptcbCur);                                      /*  ����ȴ�ɨ����              */
    
    __KERNEL_TIME_GET_NO_SPINLOCK(ulKernelTime, ULONG);                 /*  ��¼ϵͳʱ��                */
    
    iSchedRet = __KERNEL_EXIT_IRQ(iregInterLevel);                      /*  ����������                  */
    if (iSchedRet == LW_SIGNAL_EINTR) {
        iRetVal =  PX_ERROR;                                            /*  ���źż���                  */
        ulError =  EINTR;
    
    } else if (iSchedRet == LW_SIGNAL_RESTART) {
        ulTick = _sigTimeOutRecalc(ulKernelTime, ulTick);               /*  ���¼���ȴ�ʱ��            */
        goto    __wait_again;
    
    } else {
        iRetVal =  ERROR_NONE;                                          /*  ��Ȼ����                    */
        ulError =  ERROR_NONE;
    }
    
    if (rmtp) {
        ULONG           ulTimeTemp;
        
        struct timespec tvNow;
        struct timespec tvThen;
        
        __KERNEL_TIME_GET(ulTimeTemp, ULONG);                           /*  ��¼ϵͳʱ��                */
        
        __tickToTimespec(ulKernelTime, &tvThen);                        /*  �ȴ�ǰ��ʱ��                */
        __tickToTimespec(ulTimeTemp,   &tvNow);                         /*  �ȴ����ʱ��                */
    
        __timespecSub(&tvNow, &tvThen);                                 /*  ����ʱ���                  */
        
        if (__timespecLeftTime(&tvNow, (struct timespec *)rqtp)) {      /*  �Ƿ���ڲ��                */
            *rmtp = *rqtp;
            __timespecSub(rmtp, &tvNow);                                /*  ����ʱ��ƫ��                */
        } else {
            rmtp->tv_sec  = 0;                                          /*  ������ʱ����              */
            rmtp->tv_nsec = 0;
        }
    }
             
    _ErrorHandle(ulError);                                              /*  ���� errno ֵ               */
    return  (iRetVal);
}
/*********************************************************************************************************
** ��������: usleep 
** ��������: ʹ���ô˺������߳�˯��һ��ָ����ʱ��(us Ϊ��λ), ˯�߹����п��ܱ��źž���!!! (POSIX)
** �䡡��  : usecondTime       ʱ�� (us)
** �䡡��  : ERROR_NONE  or  PX_ERROR
             error == EINTR    ��ʾ���źż���.
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
                                           
                                       (�������ж��е���)
*********************************************************************************************************/
LW_API  
INT  usleep (usecond_t   usecondTime)
{
    struct timespec ts;

    ts.tv_sec  = (time_t)(usecondTime / 1000000);
    ts.tv_nsec = (LONG)(usecondTime % 1000000) * 1000ul;
    
    return  (nanosleep(&ts, LW_NULL));
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
