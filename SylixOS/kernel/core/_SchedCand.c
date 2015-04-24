/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: _SchedCand.c
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2006 年 12 月 14 日
**
** 描        述: 调度器与候选表相关操作.

** BUG:
2009.04.14  全面将调度器升级到 SMP 多核支持.
2009.04.28  升级了 SMP 调度器, 调度算法更加优化.
2009.07.11  _SchedYield() 不需要 bIsSeekPriority 加速, 最多执行一次.
2010.01.04  _SchedSwitchRunningAndReady() 运行线程的 TCB 中需要重新记录新的 CPUID.
2010.01.12  _SchedYield() 在 SMP 模式下不需要循环.
2012.03.27  _SchedSliceTick() 如果不是 RR 线程, 不起任何作用.
            _SchedSeekThread() 如果是 FIFO 线程, 则不判断时间片信息.
2013.05.07  __LW_SCHEDULER_BUG_TRACE() 打印更加详细的信息.
2013.07.29  如果候选运行表产生优先级卷绕, 则首先处理卷绕.
2013.12.02  _SchedGetCandidate() 加入任务锁定层数参数, 因为调用此函数时, 任务可能进入了调度器自旋锁.
2014.01.05  调度器故障跟踪功能不再放在扫描候选表操作中.
2014.01.10  _SchedSeekThread() 放入 _CandTable.c 中.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** 函数名称: _SchedGetCand
** 功能描述: 获的需要运行的线程表 (被调用时已经锁定了调度器 spinlock)
** 输　入  : ptcbRunner        需要运行的 TCB 列表 (大小等于 CPU 数量)
**           ulCPUIdCur        当前 CPU ID
**           ulCurMaxLock      当前 CPU 允许的最多任务锁定层数.
** 输　出  : 
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
PLW_CLASS_TCB  _SchedGetCand (PLW_CLASS_CPU  pcpuCur, ULONG  ulCurMaxLock)
{
    if (!__SHOULD_SCHED(pcpuCur, ulCurMaxLock)) {                       /*  当前执行线程不能调度        */
        return  (pcpuCur->CPU_ptcbTCBCur);
        
    } else {                                                            /*  可以执行线程切换            */
        if (LW_CAND_ROT(pcpuCur)) {                                     /*  产生优先级卷绕              */
            _CandTableUpdate(pcpuCur);
        }
        return  (LW_CAND_TCB(pcpuCur));
    }
}
/*********************************************************************************************************
** 函数名称: _SchedTick
** 功能描述: 时间片处理 (tick 中断服务程序中被调用)
** 输　入  : NONE
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  _SchedTick (VOID)
{
             INTREG         iregInterLevel;
    REGISTER PLW_CLASS_CPU  pcpu;
    REGISTER PLW_CLASS_TCB  ptcb;
             UINT8          ucPriority;
             INT            i;
             
    LW_SPIN_LOCK_QUICK(&_K_slKernel, &iregInterLevel);                  /*  锁内核 spinlock 关闭中断    */
             
#if LW_CFG_SMP_EN > 0
    for (i = 0; i < LW_NCPUS; i++) {
#else
    i = 0;
#endif                                                                  /*  LW_CFG_SMP_EN               */
        
        pcpu = LW_CPU_GET(i);
        if (LW_CPU_IS_ACTIVE(pcpu)) {                                   /*  CPU 必须激活                */
            ptcb = pcpu->CPU_ptcbTCBCur;
            if (ptcb->TCB_ucSchedPolicy == LW_OPTION_SCHED_RR) {        /*  round-robin 线程            */
                if (ptcb->TCB_usSchedCounter == 0) {                    /*  时间片已经耗尽              */
                    if (LW_CAND_ROT(pcpu) == LW_FALSE) {
                        if (_SchedSeekPriority(pcpu, &ucPriority) &&    /*  就绪未运行任务的最高优先级  */
                            LW_PRIO_IS_HIGH_OR_EQU(ucPriority,
                                                   ptcb->TCB_ucPriority)) {
                            LW_CAND_ROT(pcpu) = LW_TRUE;                /*  下次调度时检查轮转          */
                        }
                    }
                } else {
                    ptcb->TCB_usSchedCounter--;
                }
            }
        }
        
#if LW_CFG_SMP_EN > 0
    }
#endif                                                                  /*  LW_CFG_SMP_EN               */
    
    LW_SPIN_UNLOCK_QUICK(&_K_slKernel, iregInterLevel);                 /*  解锁内核 spinlock 打开中断  */
}
/*********************************************************************************************************
** 函数名称: _SchedYield
** 功能描述: 指定线程主动让出 CPU 使用权, (同优先级内) (此函数被调用时已进入内核且中断已经关闭)
** 输　入  : ptcb          在就绪表中的线程
**           ppcb          对应的优先级控制块
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
#if LW_CFG_THREAD_SCHED_YIELD_EN > 0

VOID  _SchedYield (PLW_CLASS_TCB  ptcb, PLW_CLASS_PCB  ppcb)
{
             UINT8          ucPriority;
    REGISTER PLW_CLASS_CPU  pcpu;

    if (__LW_THREAD_IS_RUNNING(ptcb)) {                                 /*  必须正在执行                */
        pcpu = LW_CPU_GET_CUR();
        if (_SchedSeekPriority(pcpu, &ucPriority) &&                    /*  就绪未运行任务的最高优先级  */
            LW_PRIO_IS_HIGH_OR_EQU(ucPriority,
                                   ptcb->TCB_ucPriority)) {
            ptcb->TCB_usSchedCounter = 0;                               /*  没收剩余时间片              */
            LW_CAND_ROT(LW_CPU_GET(ptcb->TCB_ulCPUId)) = LW_TRUE;       /*  下次调度时检查轮转          */
        }
    }
}

#endif                                                                  /*  LW_CFG_THREAD_SCHED_YIELD_EN*/
/*********************************************************************************************************
  END
*********************************************************************************************************/
