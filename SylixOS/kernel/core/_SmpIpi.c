/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                       SylixOS(TM)
**
**                               Copyright  All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: _SmpIpi.c
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2013 年 07 月 19 日
**
** 描        述: CPU 核间中断, (用于 SMP 多核系统)

** BUG:
2014.04.09  不能像没有 ACTIVE 的 CPU 发送核间中断.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
  裁剪支持
*********************************************************************************************************/
#if LW_CFG_SMP_EN > 0
/*********************************************************************************************************
** 函数名称: _SmpSendIpi
** 功能描述: 发送一个除自定义以外的核间中断给指定的 CPU 
             关中断情况下被调用, 如果需要等待, 则必须保证其他 CPU 已经运行.
** 输　入  : ulCPUId       CPU ID
**           ulIPIVec      核间中断类型 (除自定义类型中断以外)
**           iWait         是否等待处理结束 (LW_IPI_SCHED 绝不允许等待, 否则会死锁)
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  _SmpSendIpi (ULONG  ulCPUId, ULONG  ulIPIVec, INT  iWait)
{
    PLW_CLASS_CPU   pcpuDst = LW_CPU_GET(ulCPUId);
    ULONG           ulMask  = (ULONG)(1 << ulIPIVec);
    
    if (!LW_CPU_IS_ACTIVE(pcpuDst)) {                                   /*  CPU 必须被激活              */
        return;
    }
    
    LW_CPU_ADD_IPI_PEND(ulCPUId, ulMask);
    KN_SMP_WMB();
    
    archMpInt(ulCPUId);
    
    if (iWait && (ulIPIVec != LW_IPI_SCHED)) {
        while (LW_CPU_GET_IPI_PEND(ulCPUId) & ulMask) {                 /*  等待结束                    */
            LW_SPINLOCK_DELAY();
        }
    }
}
/*********************************************************************************************************
** 函数名称: _SmpSendIpiAllOther
** 功能描述: 发送一个除自定义以外的核间中断给所有其他 CPU 
             关中断情况下被调用, 如果需要等待, 则必须保证其他 CPU 已经运行.
** 输　入  : ulIPIVec      核间中断类型 (除自定义类型中断以外)
**           iWait         是否等待处理结束 (LW_IPI_SCHED 绝不允许等待, 否则会死锁)
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  _SmpSendIpiAllOther (ULONG  ulIPIVec, INT  iWait)
{
    ULONG   i;
    ULONG   ulCPUId;
    
    ulCPUId = LW_CPU_GET_CUR_ID();
    
    for (i = 0; i < LW_NCPUS; i++) {
        if (ulCPUId != i) {
            _SmpSendIpi(i, ulIPIVec, iWait);
        }
    }
}
/*********************************************************************************************************
** 函数名称: _SmpCallIpi
** 功能描述: 发送一个自定义核间中断给指定的 CPU
             关中断情况下被调用, 如果需要等待, 则必须保证其他 CPU 已经运行.
** 输　入  : ulCPUId       CPU ID
**           pipim         核间中断参数
** 输　出  : 调用返回值
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  _SmpCallIpi (ULONG  ulCPUId, PLW_IPI_MSG  pipim)
{
    PLW_CLASS_CPU   pcpuDst = LW_CPU_GET(ulCPUId);
    
    if (!LW_CPU_IS_ACTIVE(pcpuDst)) {                                   /*  CPU 必须被激活              */
        return  (ERROR_NONE);
    }
    
    LW_SPIN_LOCK_IGNIRQ(&pcpuDst->CPU_slIpi);
    
    _List_Ring_Add_Last(&pipim->IPIM_ringManage, &pcpuDst->CPU_pringMsg);
    pcpuDst->CPU_uiMsgCnt++;

    LW_CPU_ADD_IPI_PEND(ulCPUId, LW_IPI_CALL_MSK);
    KN_SMP_WMB();
    
    archMpInt(ulCPUId);
    
    LW_SPIN_UNLOCK_IGNIRQ(&pcpuDst->CPU_slIpi);
    
    while (pipim->IPIM_iWait) {                                         /*  等待结束                    */
        LW_SPINLOCK_DELAY();
    }
    
    return  (pipim->IPIM_iRet);
}
/*********************************************************************************************************
** 函数名称: _SmpCallIpiAllOther
** 功能描述: 发送一个自定义核间中断给其他所有 CPU 
             关中断情况下被调用, 如果需要等待, 则必须保证其他 CPU 已经运行.
** 输　入  : pipim         核间中断参数
** 输　出  : NONE (无法确定返回值)
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  _SmpCallIpiAllOther (PLW_IPI_MSG  pipim)
{
    ULONG   i;
    ULONG   ulCPUId;
    INT     iWait = pipim->IPIM_iWait;
    
    ulCPUId = LW_CPU_GET_CUR_ID();
    
    for (i = 0; i < LW_NCPUS; i++) {
        if (ulCPUId != i) {
            _SmpCallIpi(i, pipim);
            pipim->IPIM_iWait = iWait;
            KN_SMP_WMB();
        }
    }
}
/*********************************************************************************************************
** 函数名称: _SmpCallFunc
** 功能描述: 利用核间中断让指定的 CPU 运行指定的函数
             关中断情况下被调用, 必须保证其他 CPU 已经运行.
** 输　入  : ulCPUId       CPU ID
**           pfunc         同步执行函数
**           pvArg         同步参数
**           pfuncAsync    异步执行函数
**           pvAsync       异步执行参数
**           iOpt          选项 IPIM_OPT_NORMAL / IPIM_OPT_NOKERN
** 输　出  : 调用返回值
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
INT  _SmpCallFunc (ULONG        ulCPUId, 
                   FUNCPTR      pfunc, 
                   PVOID        pvArg,
                   VOIDFUNCPTR  pfuncAsync,
                   PVOID        pvAsync,
                   INT          iOpt)
{
    LW_IPI_MSG  ipim;
    
    ipim.IPIM_pfuncCall      = pfunc;
    ipim.IPIM_pvArg          = pvArg;
    ipim.IPIM_pfuncAsyncCall = pfuncAsync;
    ipim.IPIM_pvAsyncArg     = pvAsync;
    ipim.IPIM_iRet           = -1;
    ipim.IPIM_iOption        = iOpt;
    ipim.IPIM_iWait          = 1;
    
    return  (_SmpCallIpi(ulCPUId, &ipim));
}
/*********************************************************************************************************
** 函数名称: _SmpCallFunc
** 功能描述: 利用核间中断让指定的 CPU 运行指定的函数
             关中断情况下被调用, 必须保证其他 CPU 已经运行.
** 输　入  : pfunc         同步执行函数
**           pvArg         同步参数
**           pfuncAsync    异步执行函数
**           pvAsync       异步执行参数
**           iOpt          选项 IPIM_OPT_NORMAL / IPIM_OPT_NOKERN
** 输　出  : NONE (无法确定返回值)
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  _SmpCallFuncAllOther (FUNCPTR      pfunc, 
                            PVOID        pvArg,
                            VOIDFUNCPTR  pfuncAsync,
                            PVOID        pvAsync,
                            INT          iOpt)
{
    LW_IPI_MSG  ipim;
    
    ipim.IPIM_pfuncCall      = pfunc;
    ipim.IPIM_pvArg          = pvArg;
    ipim.IPIM_pfuncAsyncCall = pfuncAsync;
    ipim.IPIM_pvAsyncArg     = pvAsync;
    ipim.IPIM_iRet           = -1;
    ipim.IPIM_iOption        = iOpt;
    ipim.IPIM_iWait          = 1;
    
    _SmpCallIpiAllOther(&ipim);
}
/*********************************************************************************************************
** 函数名称: _SmpProcFlushTlb
** 功能描述: 处理核间中断刷新 TLB 的操作
** 输　入  : pcpuCur       当前 CPU
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
#if LW_CFG_VMM_EN > 0

static VOID  _SmpProcFlushTlb (PLW_CLASS_CPU  pcpuCur)
{
    INTREG          iregInterLevel;
    PLW_MMU_CONTEXT pmmuctx = __vmmGetCurCtx();

    iregInterLevel = KN_INT_DISABLE();
    
    __VMM_MMU_INV_TLB(pmmuctx);                                         /*  无效快表                    */
    
    KN_INT_ENABLE(iregInterLevel);

    KN_SMP_MB();
    LW_CPU_CLR_IPI_PEND2(pcpuCur, LW_IPI_FLUSH_TLB_MSK);                /*  清除                        */
    LW_SPINLOCK_NOTIFY();
}

#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
/*********************************************************************************************************
** 函数名称: _SmpProcFlushCache
** 功能描述: 处理核间中断回写 CACHE
** 输　入  : pcpuCur       当前 CPU
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
#if LW_CFG_CACHE_EN > 0

static VOID  _SmpProcFlushCache (PLW_CLASS_CPU  pcpuCur)
{
    API_CacheFlush(DATA_CACHE, (PVOID)0, (size_t)~0);
    
    KN_SMP_MB();
    LW_CPU_CLR_IPI_PEND2(pcpuCur, LW_IPI_FLUSH_CACHE_MSK);              /*  清除                        */
    LW_SPINLOCK_NOTIFY();
}

#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
/*********************************************************************************************************
** 函数名称: _SmpProcBoot
** 功能描述: 处理核间中断其他核正在启动 (当前未处理)
** 输　入  : pcpuCur       当前 CPU
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  _SmpProcBoot (PLW_CLASS_CPU  pcpuCur)
{
    KN_SMP_MB();
    LW_CPU_CLR_IPI_PEND2(pcpuCur, LW_IPI_BOOT_MSK);                     /*  清除                        */
    LW_SPINLOCK_NOTIFY();
}
/*********************************************************************************************************
** 函数名称: _SmpProcCallfunc
** 功能描述: 处理核间中断调用函数
** 输　入  : pcpuCur       当前 CPU
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  _SmpProcCallfunc (PLW_CLASS_CPU  pcpuCur)
{
#define LW_KERNEL_OWN_CPU()     (_K_slKernel.SL_pcpuOwner)

    UINT            i, uiCnt;
    PLW_IPI_MSG     pipim;
    PLW_LIST_RING   pringTemp;
    PLW_LIST_RING   pringDelete;
    VOIDFUNCPTR     pfuncAsync;
    PVOID           pvAsync;
    
    LW_SPIN_LOCK(&pcpuCur->CPU_slIpi);
    
    pringTemp = pcpuCur->CPU_pringMsg;
    uiCnt     = pcpuCur->CPU_uiMsgCnt;
    
    for (i = 0; i < uiCnt; i++) {
        pipim = _LIST_ENTRY(pringTemp, LW_IPI_MSG, IPIM_ringManage);
        if ((LW_KERNEL_OWN_CPU() == pcpuCur) &&
            (pipim->IPIM_iOption & IPIM_OPT_NOKERN)) {                  /*  此函数不能再内核锁定状态执行*/
            pringTemp = _list_ring_get_next(pringTemp);
            continue;
        }
        
        pringDelete = pringTemp;
        pringTemp   = _list_ring_get_next(pringTemp);
        _List_Ring_Del(pringDelete, &pcpuCur->CPU_pringMsg);            /*  删除一个节点                */
        pcpuCur->CPU_uiMsgCnt--;
        LW_SPIN_UNLOCK(&pcpuCur->CPU_slIpi);
        
        if (pipim->IPIM_pfuncCall) {
            pipim->IPIM_iRet = pipim->IPIM_pfuncCall(pipim->IPIM_pvArg);/*  执行同步调用                */
        }
        
        pfuncAsync = pipim->IPIM_pfuncAsyncCall;
        pvAsync    = pipim->IPIM_pvAsyncArg;
        
        KN_SMP_MB();
        pipim->IPIM_iWait = 0;                                          /*  调用结束                    */
        LW_SPINLOCK_NOTIFY();
        
        LW_SPIN_LOCK(&pcpuCur->CPU_slIpi);
        
        if (pfuncAsync) {
            pfuncAsync(pvAsync);                                        /*  执行异步调用                */
        }
    }
    
    KN_SMP_MB();
    if (pcpuCur->CPU_pringMsg == LW_NULL) {
        LW_CPU_CLR_IPI_PEND2(pcpuCur, LW_IPI_CALL_MSK);                 /*  清除                        */
        LW_SPINLOCK_NOTIFY();
    }
    
    LW_SPIN_UNLOCK(&pcpuCur->CPU_slIpi);
}
/*********************************************************************************************************
** 函数名称: _SmpProcIpi
** 功能描述: 处理核间中断 (这里不处理调度器消息)
** 输　入  : pcpuCur       当前 CPU
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  _SmpProcIpi (PLW_CLASS_CPU  pcpuCur)
{
    pcpuCur->CPU_iIPICnt++;                                             /*  核间中断数量 ++             */

#if LW_CFG_VMM_EN > 0
    if (LW_CPU_GET_IPI_PEND2(pcpuCur) & LW_IPI_FLUSH_TLB_MSK) {         /*  更新 MMU 快表               */
        _SmpProcFlushTlb(pcpuCur);
    }
#else
    KN_SMP_MB();
    LW_CPU_CLR_IPI_PEND2(pcpuCur, LW_IPI_FLUSH_TLB_MSK);
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
    
#if LW_CFG_CACHE_EN > 0
    if (LW_CPU_GET_IPI_PEND2(pcpuCur) & LW_IPI_FLUSH_CACHE_MSK) {       /*  回写 CACHE                  */
        _SmpProcFlushCache(pcpuCur);
    }
#else
    KN_SMP_MB();
    LW_CPU_CLR_IPI_PEND2(pcpuCur, LW_IPI_FLUSH_CACHE_MSK);
#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */

    if (LW_CPU_GET_IPI_PEND2(pcpuCur) & LW_IPI_BOOT_MSK) {              /*  其他核正在启动              */
        _SmpProcBoot(pcpuCur);
    }
    
    if (LW_CPU_GET_IPI_PEND2(pcpuCur) & LW_IPI_CALL_MSK) {              /*  自定义调用 ?                */
        _SmpProcCallfunc(pcpuCur);
    }
    
    KN_SMP_MB();
    LW_CPU_CLR_IPI_PEND2(pcpuCur, LW_IPI_NOP_MSK);                      /*  去掉 nop 类型核间中断       */
    LW_SPINLOCK_NOTIFY();
}
/*********************************************************************************************************
** 函数名称: _SmpUpdateIpi
** 功能描述: 由于存在被延迟执行的 IPI CALL, 此函数会在合适的时机被调用, 尝试执行 IPI CALL.
** 输　入  : pcpuCur   当前 CPU
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  _SmpUpdateIpi (PLW_CLASS_CPU  pcpuCur)
{
    archMpInt(pcpuCur->CPU_ulCPUId);
}

#endif                                                                  /*  LW_CFG_SMP_EN               */
/*********************************************************************************************************
  END
*********************************************************************************************************/
