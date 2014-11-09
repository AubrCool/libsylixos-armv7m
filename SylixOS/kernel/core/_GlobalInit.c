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
** ��   ��   ��: _GlobalInit.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2006 �� 12 �� 13 ��
**
** ��        ��: ����ϵͳ��ʼ�������⡣

** BUG
2007.03.22  ����ϵͳû������ʱ�Ĵ��������
2007.04.12  �����ж϶�ջ������ַ��ʼ��
2007.04.12  ����ж϶�ջ
2007.06.06  ����ж�Ƕ�ײ�������
2008.01.20  ȡ�����̵߳���������ȫ�ֱ�����ʼ��.
2008.03.29  ��ʼ���ֻ������ĵȴ�����Ϳ��Ź����������ͷ.
2009.04.29  ����� SMP ����ں����ĳ�ʼ��.
2009.10.12  ����� CPU ID �ĳ�ʼ��.
2009.11.01  ����ע��.
            10.31��, �ҹ�ΰ��Ŀ�ѧ��Ǯѧɭ����, ����98��. ���߶�Ǯ���ޱȾ���! 
            ����<ʿ��ͻ��>�и߳ϵ�һ�仰׷˼Ǯ��, "�����ⶫ��, ���治��˵������, ����������!". 
            Ҳ��������Լ�.
2010.01.22  �����ں��߳�ɨ����β�ĳ�ʼ��.
2010.08.03  ���� tick spinlock ��ʼ��.
2012.07.04  �ϲ� hook ��ʼ��.
2012.09.11  _GlobalInit() �м���� FPU �ĳ�ʼ��.
2013.12.19  ȥ�� FPU �ĳ�ʼ��, ���� bsp �ں˳�ʼ���ص��н���, �û���Ҫָ�� fpu ������.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#define  __KERNEL_MAIN_FILE                                             /*  ����ϵͳ���ļ�              */
#define  __LW_BITMAP                                                    /*  ����ϵͳ���ļ�              */
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
  �ж϶�ջ����
*********************************************************************************************************/
LW_STACK    _K_stkInterruptStack[LW_CFG_MAX_PROCESSORS][LW_CFG_INT_STK_SIZE / sizeof(LW_STACK)];
PLW_STACK   _K_pstkInterruptBase[LW_CFG_MAX_PROCESSORS];                /*  �жϴ���ʱ�Ķ�ջ����ַ      */
                                                                        /*  ͨ�� CPU_STK_GROWTH �ж�    */
/*********************************************************************************************************
** ��������: __interStackInit
** ��������: ��ʼ���ж϶�ջ, (SylixOS �� SMP ��ÿһ�� CPU �����Խ����ж�)
** �䡡��  : 
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __interStackInit (VOID)
{
    REGISTER INT        i;
    
    for (i = 0; i < LW_CFG_MAX_PROCESSORS; i++) {
#if CPU_STK_GROWTH == 0
        _K_pstkInterruptBase[i] = &_K_stkInterruptStack[i][0];
#else
        _K_pstkInterruptBase[i] = &_K_stkInterruptStack[i][(LW_CFG_INT_STK_SIZE / sizeof(LW_STACK)) - 1];
#endif                                                                  /*  CPU_STK_GROWTH              */
        lib_memset(_K_stkInterruptStack[i], LW_CFG_STK_EMPTY_FLAG, LW_CFG_INT_STK_SIZE);
    }
}
/*********************************************************************************************************
** ��������: __cpuInit
** ��������: ����ϵͳ CPU ���ƿ�ṹ��ʼ��
** �䡡��  : 
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __cpuInit (VOID)
{
    REGISTER INT        i;
    
    for (i = 0; i < LW_CFG_MAX_PROCESSORS; i++) {
        LW_CPU_GET(i)->CPU_ulStatus = LW_CPU_STATUS_INACTIVE;           /*  CPU INACTIVE                */
        LW_SPIN_INIT(&_K_tcbDummy[i].TCB_slLock);                       /*  ��ʼ��������                */
    }
}
/*********************************************************************************************************
** ��������: __miscSmpInit
** ��������: �� SMP �йص�ȫ�ֱ�����ʼ��
** �䡡��  : 
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __miscSmpInit (VOID)
{
    REGISTER INT            i;
             PLW_CLASS_CPU  pcpu;
    
    for (i = 0; i < LW_CFG_MAX_PROCESSORS; i++) {
        pcpu = LW_CPU_GET(i);
        
        LW_CAND_TCB(pcpu) = LW_NULL;                                    /*  ��ѡ���б�Ϊ��              */
        LW_CAND_ROT(pcpu) = LW_FALSE;                                   /*  û�����ȼ�����              */
        
        pcpu->CPU_ulCPUId        = (ULONG)i;
        pcpu->CPU_iKernelCounter = 1;                                   /*  ��ʼ�� 1, ��ǰ���������    */
        pcpu->CPU_ulIPIVector    = __ARCH_ULONG_MAX;                    /*  Ŀǰ��ȷ���˼��ж�����      */
        LW_SPIN_INIT(&pcpu->CPU_slIpi);                                 /*  ��ʼ�� CPU spinlock         */
    }
}
/*********************************************************************************************************
** ��������: __hookInit
** ��������: hook ��ʼ��
** �䡡��  : 
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��:
*********************************************************************************************************/
static VOID  __hookInit (VOID)
{
    _K_hookKernel.HOOK_ThreadCreate    = LW_NULL;                       /*  �߳̽�������                */
    _K_hookKernel.HOOK_ThreadDelete    = LW_NULL;                       /*  �߳�ɾ������                */
    _K_hookKernel.HOOK_ThreadSwap      = LW_NULL;                       /*  �߳��л�����                */
    _K_hookKernel.HOOK_ThreadTick      = LW_NULL;                       /*  ϵͳʱ���жϹ���            */
    _K_hookKernel.HOOK_ThreadInit      = LW_NULL;                       /*  �̳߳�ʼ������              */
    _K_hookKernel.HOOK_ThreadIdle      = LW_NULL;                       /*  �����̹߳���                */
    _K_hookKernel.HOOK_KernelInitBegin = LW_NULL;                       /*  �ں˳�ʼ����ʼ����          */
    _K_hookKernel.HOOK_KernelInitEnd   = LW_NULL;                       /*  �ں˳�ʼ����������          */
    _K_hookKernel.HOOK_KernelReboot    = LW_NULL;                       /*  �ں�������������            */
    _K_hookKernel.HOOK_WatchDogTimer   = LW_NULL;                       /*  ���Ź���ʱ������            */

    _K_hookKernel.HOOK_ObjectCreate = LW_NULL;                          /*  �����ں˶�����            */
    _K_hookKernel.HOOK_ObjectDelete = LW_NULL;                          /*  ɾ���ں˶�����            */
    _K_hookKernel.HOOK_FdCreate     = LW_NULL;                          /*  �ļ���������������          */
    _K_hookKernel.HOOK_FdDelete     = LW_NULL;                          /*  �ļ�������ɾ������          */
    
    _K_hookKernel.HOOK_CpuIdleEnter = LW_NULL;                          /*  CPU �������ģʽ            */
    _K_hookKernel.HOOK_CpuIdleExit  = LW_NULL;                          /*  CPU �˳�����ģʽ            */
    _K_hookKernel.HOOK_CpuIntEnter  = LW_NULL;                          /*  CPU �����ж�(�쳣)ģʽ      */
    _K_hookKernel.HOOK_CpuIntExit   = LW_NULL;                          /*  CPU �˳��ж�(�쳣)ģʽ      */
}
/*********************************************************************************************************
** ��������: _GlobalInit
** ��������: ��ʼ����ɢȫ�ֱ���
** �䡡��  : 
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID _GlobalInit (VOID)
{
    LW_SYS_STATUS_SET(LW_SYS_STATUS_INIT);                              /*  ϵͳ״̬Ϊ��ʼ��״̬        */

    /*
     *  �ں˹ؼ�����������ʼ��
     */
    LW_SPIN_INIT(&_K_slKernel);                                         /*  ��ʼ���ں�������            */
    LW_SPIN_INIT(&_K_slAtomic);                                         /*  ��ʼ��ԭ�Ӳ���������        */

    /*
     *  �ں˹ؼ������ݽṹ��ʼ��
     */
    __cpuInit();                                                        /*  CPU �ṹ��ʼ��              */
    __interStackInit();                                                 /*  ���ȳ�ʼ���ж϶�ջ          */
    __miscSmpInit();                                                    /*  SMP ��س�ʼ��              */
    __hookInit();                                                       /*  hook ��ʼ��                 */
    
    /*
     *  �ں˹ؼ���״̬������ʼ��
     */
    _K_i64KernelTime = 0;
    
#if LW_CFG_THREAD_CPU_USAGE_CHK_EN > 0
    _K_ulCPUUsageTicks       = 1ul;                                     /*  ����� 0 ����               */
    _K_ulCPUUsageKernelTicks = 0ul;
#endif

    _K_usThreadCounter = 0;                                             /*  �߳�����                    */
    _K_plineTCBHeader  = LW_NULL;                                       /*  TCB ��������ͷ              */
    _K_ulNotRunError   = 0ul;                                           /*  ϵͳδ����ʱ�����ű���    */
    
    __WAKEUP_INIT(&_K_wuDelay);
#if LW_CFG_SOFTWARE_WATCHDOG_EN > 0
    __WAKEUP_INIT(&_K_wuWatchDog);
#endif                                                                  /*  LW_CFG_SOFTWARE_WATCHDOG_EN */
    
#if LW_CFG_THREAD_CPU_USAGE_CHK_EN > 0
    __LW_TICK_CPUUSAGE_ENABLE();                                        /*  ���� CPU �����ʲ���         */
#endif                                                                  /*  LW_CFG_THREAD_CPU_USAGE_... */
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
