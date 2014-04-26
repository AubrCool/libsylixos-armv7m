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
** ��   ��   ��: lib_clock.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2008 �� 08 �� 30 ��
**
** ��        ��: ϵͳ��.

** BUG:
2013.11.28  ���� lib_clock_nanosleep().
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
#if LW_CFG_MODULELOADER_EN > 0
#include "../SylixOS/loader/include/loader_vppatch.h"
#endif                                                                  /*  LW_CFG_MODULELOADER_EN > 0  */
/*********************************************************************************************************
** ��������: lib_clock
** ��������: 
** �䡡��  : 
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
#if LW_CFG_RTC_EN > 0

clock_t  lib_clock (VOID)
{
    return  ((clock_t)API_TimeGet());
}
/*********************************************************************************************************
** ��������: lib_clock_gettime
** ��������: 
** �䡡��  : 
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
INT  lib_clock_gettime (clockid_t  clockid, struct timespec  *tv)
{
    INTREG          iregInterLevel;
    PLW_CLASS_TCB   ptcbCur;

    if (tv == LW_NULL) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    if (clockid == CLOCK_REALTIME) {
        LW_SPIN_LOCK_QUICK(&_K_slKernelRtc, &iregInterLevel);
        *tv = _K_tvTODCurrent;
        LW_SPIN_UNLOCK_QUICK(&_K_slKernelRtc, iregInterLevel);
    
    } else if (clockid == CLOCK_MONOTONIC) {
        LW_SPIN_LOCK_QUICK(&_K_slKernelRtc, &iregInterLevel);
        *tv = _K_tvTODMono;
        LW_SPIN_UNLOCK_QUICK(&_K_slKernelRtc, iregInterLevel);
    
    } else if (clockid == CLOCK_PROCESS_CPUTIME_ID) {
#if LW_CFG_MODULELOADER_EN > 0
        LW_LD_VPROC *vproc = __LW_VP_GET_CUR_PROC();
        if (vproc == LW_NULL) {
            _ErrorHandle(ENOSYS);
            return  (PX_ERROR);
        }
        LW_SPIN_LOCK_QUICK(&_K_slKernelTime, &iregInterLevel);
        __tickToTimespec(vproc->VP_clockUser + vproc->VP_clockSystem, tv);
        LW_SPIN_UNLOCK_QUICK(&_K_slKernelTime, iregInterLevel);
#else
        _ErrorHandle(ENOSYS);
        return  (PX_ERROR);
#endif                                                                  /*  LW_CFG_MODULELOADER_EN > 0  */

    } else if (clockid == CLOCK_THREAD_CPUTIME_ID) {
        LW_SPIN_LOCK_QUICK(&_K_slKernelTime, &iregInterLevel);
        LW_TCB_GET_CUR(ptcbCur);
        __tickToTimespec(ptcbCur->TCB_ulCPUTicks, tv);
        LW_SPIN_UNLOCK_QUICK(&_K_slKernelTime, iregInterLevel);
    
    } else {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: lib_clock_settime
** ��������: 
** �䡡��  : 
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
INT  lib_clock_settime (clockid_t  clockid, const struct timespec  *tv)
{
    INTREG      iregInterLevel;

    if (tv == LW_NULL) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    if (clockid != CLOCK_REALTIME) {                                    /*  CLOCK_REALTIME              */
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    LW_SPIN_LOCK_QUICK(&_K_slKernelRtc, &iregInterLevel);
    _K_tvTODCurrent = *tv;
    LW_SPIN_UNLOCK_QUICK(&_K_slKernelRtc, iregInterLevel);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: lib_clock_nanosleep
** ��������: 
** �䡡��  : 
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
INT  lib_clock_nanosleep (clockid_t  clockid, int  iFlags, 
                          const struct timespec  *rqtp, struct timespec  *rmtp)
{
    INTREG           iregInterLevel;
    struct timespec  tvValue;

    (VOID)clockid;
    
    if (clockid != CLOCK_REALTIME) {
        _ErrorHandle(ENOTSUP);
        return  (PX_ERROR);
    }
    
    if (!rqtp) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    tvValue = *rqtp;
    
    if (iFlags == TIMER_ABSTIME) {
        struct timespec  tvNow;
        LW_SPIN_LOCK_QUICK(&_K_slKernelRtc, &iregInterLevel);
        tvNow = _K_tvTODCurrent;
        LW_SPIN_UNLOCK_QUICK(&_K_slKernelRtc, iregInterLevel);
        
        if ((rqtp->tv_sec < tvNow.tv_sec) ||
            ((rqtp->tv_sec == tvNow.tv_sec) &&
             (rqtp->tv_nsec < tvNow.tv_nsec))) {
            _ErrorHandle(EINVAL);
            return  (PX_ERROR);
        } else {
            __timespecSub(&tvValue, &tvNow);
        }
    }
    
    return  (nanosleep(&tvValue, rmtp));
}
#endif                                                                  /*  LW_CFG_RTC_EN               */
/*********************************************************************************************************
  END
*********************************************************************************************************/
