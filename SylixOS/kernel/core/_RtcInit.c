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
** 文   件   名: _RtcInit.c
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2007 年 02 月 05 日
**
** 描        述: RTC 初始化。

** BUG:
2010.01.04  开始使用新的 TOD 时钟库.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** 函数名称: _RtcInit
** 功能描述: RTC 初始化
** 输　入  : 
** 输　出  : 
** 全局变量: 
** 调用模块:
*********************************************************************************************************/
VOID  _RtcInit (VOID)
{
#if LW_CFG_RTC_EN > 0
    /*
     *  初始化为 2000.1.1 0:0:0
     */
    struct tm   tmNow = {
        0, 0, 0, 1, 0, (2000 - 1900), 6, 0, 0
    };
    time_t      timeNow;
    
    LW_SPIN_INIT(&_K_slKernelRtc);                                      /*  初始化 rtc spinlock         */
    
    timeNow  = lib_timegm(&tmNow);
    timezone = (-(3600 * 8));                                           /*  默认为东8区 CST-8:00:00     */
    
    _K_tvTODCurrent.tv_sec  = timeNow;
    _K_tvTODCurrent.tv_nsec = 0;
    
    _K_tvTODMono.tv_sec  = 0;
    _K_tvTODMono.tv_nsec = 0;
#endif                                                                  /*  LW_CFG_RTC_EN > 0           */
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
