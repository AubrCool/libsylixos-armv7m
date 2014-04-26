/*********************************************************************************************************
**
**                                    �й������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                       SylixOS(TM)
**
**                               Copyright  All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: sched.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2009 �� 12 �� 30 ��
**
** ��        ��: posix ���ȼ��ݿ�.

** BUG:
2011.03.26  Higher numerical values for the priority represent higher priorities.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../include/px_sched.h"                                        /*  �Ѱ�������ϵͳͷ�ļ�        */
#include "../include/posixLib.h"                                        /*  posix �ڲ�������            */
/*********************************************************************************************************
  �ü�֧��
*********************************************************************************************************/
#if LW_CFG_POSIX_EN > 0
/*********************************************************************************************************
** ��������: sched_get_priority_max
** ��������: ��õ����������������ȼ� (pthread �̲߳����� idle ͬ���ȼ�!)
** �䡡��  : iPolicy       ���Ȳ��� (Ŀǰ����)
** �䡡��  : ������ȼ�
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  sched_get_priority_max (int  iPolicy)
{
    (void)iPolicy;

    return  (__PX_PRIORITY_MAX);                                        /*  254                         */
}
/*********************************************************************************************************
** ��������: sched_get_priority_min
** ��������: ��õ������������С���ȼ�
** �䡡��  : iPolicy       ���Ȳ��� (Ŀǰ����)
** �䡡��  : ��С���ȼ�
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  sched_get_priority_min (int  iPolicy)
{
    (void)iPolicy;
    
    return  (__PX_PRIORITY_MIN);                                        /*  1                           */
}
/*********************************************************************************************************
** ��������: sched_yield
** ��������: ����ǰ������뵽ͬ���ȼ���������������, �����ó�һ�� CPU ����.
** �䡡��  : NONE
** �䡡��  : ERROR_NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  sched_yield (void)
{
    API_ThreadYield(API_ThreadIdSelf());
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: sched_setparam
** ��������: ����ָ���������������
** �䡡��  : lThreadId     �߳� ID
**           pschedparam   ����������
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  sched_setparam (pid_t  lThreadId, const struct sched_param  *pschedparam)
{
    UINT8       ucPriority;
    ULONG       ulError;
    
    if (pschedparam == LW_NULL) {
        errno = EINVAL;
        return  (PX_ERROR);
    }
    if ((pschedparam->sched_priority < __PX_PRIORITY_MIN) ||
        (pschedparam->sched_priority > __PX_PRIORITY_MAX)) {
        errno = EINVAL;
        return  (PX_ERROR);
    }
    
    ucPriority= (UINT8)__PX_PRIORITY_CONVERT(pschedparam->sched_priority);
    
    PX_ID_VERIFY(lThreadId, pid_t);
    
    ulError = API_ThreadSetPriority((LW_OBJECT_HANDLE)lThreadId, ucPriority);
    if (ulError) {
        errno = ESRCH;
        return  (PX_ERROR);
    } else {
        return  (ERROR_NONE);
    }
}
/*********************************************************************************************************
** ��������: sched_getparam
** ��������: ���ָ���������������
** �䡡��  : lThreadId     �߳� ID
**           pschedparam   ����������
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  sched_getparam (pid_t  lThreadId, struct sched_param  *pschedparam)
{
    UINT8       ucPriority;
    ULONG       ulError;
    
    if (pschedparam == LW_NULL) {
        errno = EINVAL;
        return  (PX_ERROR);
    }
    
    PX_ID_VERIFY(lThreadId, pid_t);

    ulError = API_ThreadGetPriority((LW_OBJECT_HANDLE)lThreadId, &ucPriority);
    if (ulError) {
        errno = ESRCH;
        return  (PX_ERROR);
    } else {
        pschedparam->sched_priority = __PX_PRIORITY_CONVERT(ucPriority);
        return  (ERROR_NONE);
    }
}
/*********************************************************************************************************
** ��������: sched_setscheduler
** ��������: ����ָ�����������
** �䡡��  : lThreadId     �߳� ID
**           iPolicy       ���Ȳ���
**           pschedparam   ����������
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  sched_setscheduler (pid_t                      lThreadId, 
                         int                        iPolicy, 
                         const struct sched_param  *pschedparam)
{
    UINT8       ucPriority;
    UINT8       ucActivatedMode;

    if (pschedparam == LW_NULL) {
        errno = EINVAL;
        return  (PX_ERROR);
    }
    
    if ((iPolicy != LW_OPTION_SCHED_FIFO) &&
        (iPolicy != LW_OPTION_SCHED_RR)) {
        errno = EINVAL;
        return  (PX_ERROR);
    }
    
    if ((pschedparam->sched_priority < __PX_PRIORITY_MIN) ||
        (pschedparam->sched_priority > __PX_PRIORITY_MAX)) {
        errno = EINVAL;
        return  (PX_ERROR);
    }
    
    ucPriority= (UINT8)__PX_PRIORITY_CONVERT(pschedparam->sched_priority);
    
    PX_ID_VERIFY(lThreadId, pid_t);
    
    if (API_ThreadGetSchedParam((LW_OBJECT_HANDLE)lThreadId,
                                LW_NULL,
                                &ucActivatedMode)) {
        errno = ESRCH;
        return  (PX_ERROR);
    }
    
    API_ThreadSetSchedParam((LW_OBJECT_HANDLE)lThreadId, (UINT8)iPolicy, ucActivatedMode);
    
    if (API_ThreadSetPriority((LW_OBJECT_HANDLE)lThreadId, ucPriority)) {
        errno = ESRCH;
        return  (PX_ERROR);
    } else {
        return  (ERROR_NONE);
    }
}
/*********************************************************************************************************
** ��������: sched_getscheduler
** ��������: ���ָ�����������
** �䡡��  : lThreadId     �߳� ID
** �䡡��  : ���Ȳ���
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  sched_getscheduler (pid_t  lThreadId)
{
    UINT8   ucPolicy;
    
    PX_ID_VERIFY(lThreadId, pid_t);
    
    if (API_ThreadGetSchedParam((LW_OBJECT_HANDLE)lThreadId,
                                &ucPolicy,
                                LW_NULL)) {
        errno = ESRCH;
        return  (PX_ERROR);
    } else {
        return  ((int)ucPolicy);
    }
}
/*********************************************************************************************************
** ��������: sched_rr_get_interval
** ��������: ���ָ�����������
** �䡡��  : lThreadId     �߳� ID
**           interval      current execution time limit.
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  sched_rr_get_interval (pid_t  lThreadId, struct timespec  *interval)
{
    UINT8               ucPolicy;
    UINT16              usCounter = 0;

    if (interval) {
        errno = EINVAL;
        return  (PX_ERROR);
    }

    if (API_ThreadGetSchedParam((LW_OBJECT_HANDLE)lThreadId,
                                &ucPolicy,
                                LW_NULL)) {
        errno = ESRCH;
        return  (PX_ERROR);
    }
    if (ucPolicy != LW_OPTION_SCHED_RR) {
        errno = EINVAL;
        return  (PX_ERROR);
    }
    
    if (API_ThreadGetSliceEx((LW_OBJECT_HANDLE)lThreadId, LW_NULL, &usCounter)) {
        errno = ESRCH;
        return  (PX_ERROR);
    }
    
    __tickToTimespec((ULONG)usCounter, interval);

    return  (ERROR_NONE);
}
#endif                                                                  /*  LW_CFG_POSIX_EN > 0         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
