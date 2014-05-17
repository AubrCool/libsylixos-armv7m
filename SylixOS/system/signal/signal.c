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
** ��   ��   ��: signal.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2007 �� 06 �� 03 ��
**
** ��        ��: ϵͳ�źŴ������⡣ 

** BUG
2008.05.18  ʹ�� __KERNEL_ENTER() ���� ThreadLock();
2008.08.11  API_kill ����Ծ�����͵��ж�, ���� shell ��һ����������·����źŽ� idle ɾ����, ������Ϊ
            û�м��������.
2009.01.13  �޸����ע��.
2009.05.24  ������ TCB_bIsInDeleteProc Ϊ TRUE ���̷߳����ź�.
2011.08.15  �ش����: ������ posix ����ĺ����Ժ�����ʽ(�Ǻ�)����.
2012.03.20  ���ٶ� _K_ptcbTCBCur ������, �������þֲ�����, ���ٶԵ�ǰ CPU ID ��ȡ�Ĵ���.
2012.08.24  �����źź���֧�ֽ��̺�, ��Ӧ�źŽ����͵��Է����߳�.
2012.12.12  sigprocmask �����ź�����ʱ, ��Щ�ź��ǲ������ε�.
2013.01.15  sigaction ��װ���ź�������, Ҫ���������ε��ź�.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
/*********************************************************************************************************
  �ü�����
*********************************************************************************************************/
#if LW_CFG_SIGNAL_EN > 0
#include "signalPrivate.h"
/*********************************************************************************************************
  �ڲ���������
*********************************************************************************************************/
extern PLW_CLASS_SIGCONTEXT  _signalGetCtx(PLW_CLASS_TCB  ptcb);
extern VOID                  _sigPendFree(PLW_CLASS_SIGPEND  psigpendFree);
extern BOOL                  _sigPendRun(PLW_CLASS_TCB  ptcb);
extern INT                   _sigPendGet(PLW_CLASS_SIGCONTEXT  psigctx, 
                                         const sigset_t  *psigset, struct siginfo *psiginfo);
/*********************************************************************************************************
  �ڲ����ͺ�������
*********************************************************************************************************/
extern LW_SEND_VAL           _doKill(PLW_CLASS_TCB  ptcb, INT  iSigNo);
extern LW_SEND_VAL           _doSigQueue(PLW_CLASS_TCB  ptcb, INT  iSigNo, const union sigval  sigvalue);
/*********************************************************************************************************
  �ڲ�������غ�������
*********************************************************************************************************/
#if LW_CFG_MODULELOADER_EN > 0
#include "../SylixOS/loader/include/loader_vppatch.h"
#endif                                                                  /*  LW_CFG_MODULELOADER_EN > 0  */
/*********************************************************************************************************
** ��������: sigemptyset
** ��������: ��ʼ��һ���յ��źż�
** �䡡��  : 
**           psigset                 �źż�
** �䡡��  : ERROR_NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  sigemptyset (sigset_t    *psigset)
{
    *psigset = 0;
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: sigfillset
** ��������: ��ʼ��һ�������źż�
** �䡡��  : psigset                 �źż�
** �䡡��  : ERROR_NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  sigfillset (sigset_t	*psigset)
{
    *psigset = ~0;
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: sigaddset
** ��������: ��һ���źż����һ���ź�
** �䡡��  : psigset                 �źż�
**           iSigNo                  �ź�
** �䡡��  : ERROR_NONE , EINVAL
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  sigaddset (sigset_t  *psigset, INT  iSigNo)
{
    if (__issig(iSigNo)) {
        *psigset |= __sigmask(iSigNo);
        return  (ERROR_NONE);
    }
    _ErrorHandle(EINVAL);
    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: sigdelset
** ��������: ��һ���źż���ɾ��һ���ź�
** �䡡��  : psigset                 �źż�
**           iSigNo                  �ź�
** �䡡��  : ERROR_NONE , EINVAL
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  sigdelset (sigset_t  *psigset, INT  iSigNo)
{
    if (__issig(iSigNo)) {
        *psigset &= ~__sigmask(iSigNo);
        return  (ERROR_NONE);
    }
    _ErrorHandle(EINVAL);
    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: sigismember
** ��������: ���һ���ź��Ƿ�����һ���źż�
** �䡡��  : 
**           psigset                 �źż�
**           iSigNo                  �ź�
** �䡡��  : 0 or 1 or -1
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  sigismember (const sigset_t  *psigset, INT  iSigNo)
{
    if (__issig(iSigNo)) {
        if (*psigset & __sigmask(iSigNo)) {
            return  (1);
        } else {
            return  (0);
        }
    }
    _ErrorHandle(EINVAL);
    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: sigaction
** ��������: ����һ��ָ���źŵķ�������, ͬʱ�ɻ�ȡԭʼ��������. 
**           (������ struct sigaction ����, ��������ֱ��ʹ�� sigaction ������)
** �䡡��  : iSigNo        �ź�
**           psigactionNew �µĴ���ṹ
**           psigactionOld ����Ĵ���ṹ
** �䡡��  : ERROR_NONE , EINVAL
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT   sigaction (INT                      iSigNo, 
                 const struct sigaction  *psigactionNew,
                 struct sigaction        *psigactionOld)
{
    struct sigaction               *psigaction;
    PLW_CLASS_SIGCONTEXT            psigctx;
    PLW_CLASS_TCB                   ptcbCur;
    REGISTER PLW_CLASS_SIGPEND      psigpend;
    REGISTER INT                    iSigIndex = __sigindex(iSigNo);     /*  TCB_sigaction �±�          */
    
    if (!__issig(iSigNo)) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);
    
    psigctx    = _signalGetCtx(ptcbCur);
    psigaction = &psigctx->SIGCTX_sigaction[iSigIndex];
    
    if (psigactionOld) {
        *psigactionOld = *psigaction;                                   /*  ����������Ϣ                */
    }
    
    if (psigactionNew == LW_NULL) {
        return  (ERROR_NONE);
    }
    
    __KERNEL_ENTER();
    *psigaction = *psigactionNew;                                       /*  �����µĴ�����ƿ�          */
    psigaction->sa_mask &= ~__SIGNO_UNMASK;                             /*  ��Щ�źŲ�������            */
    __KERNEL_EXIT();
    
    if (psigaction->sa_handler == SIG_IGN) {                            /*  ����Ϊ���Ը��ź�            */
        __KERNEL_ENTER();                                               /*  �����ں�                    */
        
        psigctx->SIGCTX_sigsetPending &= ~__sigmask(iSigNo);            /*  û�еȴ� unmask ��ִ�е��ź�*/
        psigctx->SIGCTX_sigsetKill    &= ~__sigmask(iSigNo);            /*  û��������״̬ kill ����ź�*/
        
        {                                                               /*  ɾ�������е�����źŽڵ�    */
                     PLW_LIST_RING  pringHead = psigctx->SIGCTX_pringSigQ[iSigIndex];
            REGISTER PLW_LIST_RING  pringSigP = pringHead;
            
            if (pringHead) {                                            /*  ���Ѷ����д��ڽڵ�          */
                do {
                    psigpend  = _LIST_ENTRY(pringSigP, 
                                            LW_CLASS_SIGPEND, 
                                            SIGPEND_ringSigQ);          /*  ��� sigpend ���ƿ��ַ     */
                    
                    pringSigP = _list_ring_get_next(pringSigP);         /*  ��һ���ڵ�                  */
                    
                    if ((psigpend->SIGPEND_siginfo.si_code != SI_KILL) &&
                        (psigpend->SIGPEND_iNotify         == SIGEV_SIGNAL)) {
                        _sigPendFree(psigpend);                         /*  ��Ҫ�������ж���            */
                    }
                } while (pringSigP != pringHead);
            }
        }
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: signal
** ��������: ��ָһ���źŵĴ�����
** �䡡��  : iSigNo        �ź�
**           pfuncHandler  ������
** �䡡��  : ����Ĵ�����
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
PSIGNAL_HANDLE signal(INT  iSigNo, PSIGNAL_HANDLE  pfuncHandler)
{
    INT               iError;
    struct sigaction  sigactionNew;
    struct sigaction  sigactionOld;
    
    sigactionNew.sa_handler = pfuncHandler;
    sigactionNew.sa_flags   = 0;                                        /*  ������ͬ�����ź�Ƕ��        */
    
    sigemptyset(&sigactionNew.sa_mask);
    
    iError = sigaction(iSigNo, &sigactionNew, &sigactionOld);
    if (iError) {
        return  (SIG_ERR);
    } else {
        return  (sigactionOld.sa_handler);
    }
}
/*********************************************************************************************************
** ��������: sigvec
** ��������: ��װ�ź�������� (BSD ����)
** �䡡��  : iBlock                   �µ������ź�����
** �䡡��  : ���������
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  sigvec (INT  iSigNo, const struct sigvec *pvec, struct sigvec *pvecOld)
{
    struct sigaction sigactionNew;
    struct sigaction sigactionOld;
    
    INT              iSave  = 0;
    INT              iError = PX_ERROR;
    
    if (pvec) {                                                         /*  �����µ��ź�����            */
        sigactionNew.sa_handler = pvec->sv_handler;
        sigactionNew.sa_mask    = pvec->sv_mask;
        sigactionNew.sa_flags   = pvec->sv_flags;
        if (pvecOld) {
            iError = sigaction(iSigNo, &sigactionNew, &sigactionOld);
            iSave  = 1;
        } else {
            iError = sigaction(iSigNo, &sigactionNew, LW_NULL);
        }
    } else if (pvecOld) {
        iError = sigaction(iSigNo, LW_NULL, &sigactionOld);             /*  ��ȡ                        */
        iSave  = 1;
    }
    
    if (iSave) {
        pvecOld->sv_handler = sigactionOld.sa_handler;
        pvecOld->sv_mask    = sigactionOld.sa_mask;
        pvecOld->sv_flags   = sigactionOld.sa_flags;
    }
    
    return  (iError);
}
/*********************************************************************************************************
** ��������: sigpending
** ��������: ��õ�ǰ����ʱ����δ������ź� (�д�����. ���Ǳ�������)
** �䡡��  : psigset                 �źż�
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  sigpending (sigset_t  *psigset)
{
    PLW_CLASS_SIGCONTEXT  psigctx;
    PLW_CLASS_TCB         ptcbCur;
    
    if (psigset) {
        LW_TCB_GET_CUR_SAFE(ptcbCur);
        psigctx  = _signalGetCtx(ptcbCur);
        *psigset = psigctx->SIGCTX_sigsetPending;
        return  (ERROR_NONE);
    
    } else {
        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** ��������: sigprocmask
** ��������: ���Ի�ı䵱ǰ�̵߳��ź�����
** �䡡��  : iCmd                    ����
**           psigset                 ���źż�
**           psigsetOld              �����źż�
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  sigprocmask (INT              iCmd, 
                  const sigset_t  *psigset, 
                        sigset_t  *psigsetOld)
{
    PLW_CLASS_TCB         ptcbCur;
    PLW_CLASS_SIGCONTEXT  psigctx;
    sigset_t              sigsetBlock;
    
    if (LW_CPU_GET_CUR_NESTING()) {                                     /*  �������ж��е���            */
        return  (PX_ERROR);
    }
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);
    
    psigctx = _signalGetCtx(ptcbCur);
    
    if (psigsetOld) {                                                   /*  ������ϵ�                  */
        *psigsetOld = psigctx->SIGCTX_sigsetSigBlockMask;
    }
    
    if (!psigset) {                                                     /*  �µ��Ƿ���Ч                */
        return  (ERROR_NONE);
    }
    
    __KERNEL_ENTER();                                                   /*  �����ں�                    */
    
    switch (iCmd) {
    
    case SIG_BLOCK:                                                     /*  �������                    */
        sigsetBlock  = *psigset;
        sigsetBlock &= ~__SIGNO_UNMASK;                                 /*  ��Щ�ź��ǲ������ε�        */
        psigctx->SIGCTX_sigsetSigBlockMask |= sigsetBlock;
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
        _ErrorHandle(ERROR_NONE);
        return  (ERROR_NONE);
        
    case SIG_UNBLOCK:                                                   /*  ɾ������                    */
        psigctx->SIGCTX_sigsetSigBlockMask &= ~(*psigset);
        break;
        
    case SIG_SETMASK:                                                   /*  ��������                    */
        psigctx->SIGCTX_sigsetSigBlockMask  = *psigset;
        break;
    
    default:                                                            /*  ����                        */
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "command invalidate.\r\n");
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    _sigPendRun(ptcbCur);                                               /*  ��������ǰ�������ź���Ҫ����*/
    
    __KERNEL_EXIT();                                                    /*  �˳��ں�                    */
    _ErrorHandle(ERROR_NONE);
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: sigsetmask
** ��������: ���õ�ǰ�߳��µ����� (BSD ����)
** �䡡��  : iMask                   �µ�����
** �䡡��  : ���������
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  sigsetmask (INT  iMask)
{
    sigset_t    sigsetNew;
    sigset_t    sigsetOld;
    INT         iMaskOld;
    
    lib_memcpy(&sigsetNew, &iMask, sizeof(INT));
    (VOID)sigprocmask(SIG_SETMASK, &sigsetNew, &sigsetOld);
    lib_memcpy(&iMaskOld,  &sigsetOld, sizeof(INT));
    
    return  (iMaskOld);
}
/*********************************************************************************************************
** ��������: sigsetblock
** ��������: ���µ���Ҫ�������ź���ӵ���ǰ�߳� (BSD ����)
** �䡡��  : iBlock                   �µ������ź�����
** �䡡��  : ���������
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  sigblock (INT  iMask)
{
    sigset_t    sigsetNew;
    sigset_t    sigsetOld;
    INT         iMaskOld;
    
    lib_memcpy(&sigsetNew, &iMask, sizeof(INT));
    (VOID)sigprocmask(SIG_BLOCK, &sigsetNew, &sigsetOld);
    lib_memcpy(&iMaskOld,  &sigsetOld, sizeof(INT));
    
    return  (iMaskOld);
}
/*********************************************************************************************************
** ��������: kill
** ��������: ��ָ�����̷߳����ź�, ����ǽ���, �����͸������߳�.
** �䡡��  : ulId                    �߳� id ���� ���̺�
**           iSigNo                  �ź�
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  kill (LW_OBJECT_HANDLE  ulId, INT  iSigNo)
{
    REGISTER UINT16             usIndex;
    REGISTER PLW_CLASS_TCB      ptcb;
             LW_SEND_VAL        sendval;
    
#if LW_CFG_MODULELOADER_EN > 0
    if (ulId < LW_CFG_MAX_THREADS) {                                    /*  ���̺�                      */
        ulId = vprocMainThread((pid_t)ulId);
    }
#endif                                                                  /*  LW_CFG_MODULELOADER_EN > 0  */

    usIndex = _ObjectGetIndex(ulId);
    
    if (!_ObjectClassOK(ulId, _OBJECT_THREAD)) {                        /*  ��� ID ������Ч��          */
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (PX_ERROR);
    }
    if (_Thread_Index_Invalid(usIndex)) {                               /*  ����߳���Ч��              */
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (PX_ERROR);
    }
    
    if (!__issig(iSigNo)) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    if (LW_CPU_GET_CUR_NESTING() || (ulId == API_ThreadIdSelf())) {
        _excJobAdd((VOIDFUNCPTR)kill, (PVOID)ulId, (PVOID)iSigNo, 0, 0, 0, 0);
        _ErrorHandle(ERROR_NONE);
        return  (ERROR_NONE);
    }

#if LW_CFG_SMP_EN > 0
    if (LW_NCPUS > 1) {                                                 /*  �������� SMP ���ģʽ       */
        if (API_ThreadStop(ulId)) {
            return  (PX_ERROR);
        }
    }
#endif                                                                  /*  LW_CFG_SMP_EN               */

    __KERNEL_ENTER();                                                   /*  �����ں�                    */
    if (_Thread_Invalid(usIndex)) {
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    ptcb = __GET_TCB_FROM_INDEX(usIndex);
    if (ptcb->TCB_iDeleteProcStatus) {
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
        _ErrorHandle(ERROR_THREAD_OTHER_DELETE);
        return  (PX_ERROR);
    }
    
    sendval = _doKill(ptcb, iSigNo);                                    /*  �����ź�                    */
    
#if LW_CFG_SMP_EN > 0
    if (LW_NCPUS > 1) {                                                 /*  �������� SMP ���ģʽ       */
        _ThreadContinue(ptcb, LW_FALSE);                                /*  ���ں�״̬�»��ѱ�ֹͣ�߳�  */
    }
#endif                                                                  /*  LW_CFG_SMP_EN               */
    __KERNEL_EXIT();                                                    /*  �˳��ں�                    */
    
#if LW_CFG_SIGNALFD_EN > 0
    if (sendval == SEND_BLOCK) {
        _sigfdReadUnblock(ulId, iSigNo);
    }
#endif                                                                  /*  LW_CFG_SIGNALFD_EN > 0      */
    
    _ErrorHandle(ERROR_NONE);
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: killTrap
** ��������: ��ָ���������ź�, ͬʱֹͣ�Լ�. (���������쳣��������ִ��)
** �䡡��  : ulId                    �߳� id (������Ϊ���̺�)
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  killTrap (LW_OBJECT_HANDLE  ulId)
{
    REGISTER PLW_CLASS_TCB  ptcbCur;
    
    if (!LW_CPU_GET_CUR_NESTING()) {                                    /*  �������쳣��                */
        return  (PX_ERROR);
    }
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);                                       /*  ��ǰ������ƿ�              */
    
    __KERNEL_ENTER();                                                   /*  �����ں�                    */
    _ThreadStop(ptcbCur);
    __KERNEL_EXIT();                                                    /*  �˳��ں�                    */
    
    _excJobAdd((VOIDFUNCPTR)kill, (PVOID)ulId, (PVOID)SIGTRAP, 0, 0, 0, 0);
    
    _ErrorHandle(ERROR_NONE);
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: raise
** ��������: ���Լ������ź�
** �䡡��  : iSigNo                  �ź�
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  raise (INT  iSigNo)
{
    return  (kill(API_ThreadIdSelf(), iSigNo));
}
/*********************************************************************************************************
** ��������: sigqueue_internal
** ��������: ���Ͷ��������ź�, ����ǽ���, �����͸������߳�.
** �䡡��  : ulId                    �߳� id ���� ���̺�
**           iSigNo                  �ź�
**           pvSigValue              �ź� value
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
static INT  sigqueue_internal (LW_OBJECT_HANDLE  ulId, INT   iSigNo, PVOID  pvSigValue)
{
    REGISTER UINT16             usIndex;
    REGISTER PLW_CLASS_TCB      ptcb;
             LW_SEND_VAL        sendval;
             union sigval       sigvalue;
             
#if LW_CFG_MODULELOADER_EN > 0
    if (ulId < LW_CFG_MAX_THREADS) {                                    /*  ���̺�                      */
        ulId = vprocMainThread((pid_t)ulId);
    }
#endif                                                                  /*  LW_CFG_MODULELOADER_EN > 0  */
    
    usIndex = _ObjectGetIndex(ulId);
    
    if (!_ObjectClassOK(ulId, _OBJECT_THREAD)) {                        /*  ��� ID ������Ч��          */
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (PX_ERROR);
    }
    if (_Thread_Index_Invalid(usIndex)) {                               /*  ����߳���Ч��              */
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (PX_ERROR);
    }
    
    if (!__issig(iSigNo)) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    if (LW_CPU_GET_CUR_NESTING() || (ulId == API_ThreadIdSelf())) {
        _excJobAdd((VOIDFUNCPTR)sigqueue_internal, (PVOID)ulId, (PVOID)iSigNo, (PVOID)pvSigValue, 
                   0, 0, 0);
        _ErrorHandle(ERROR_NONE);
        return  (ERROR_NONE);
    }
    
#if LW_CFG_SMP_EN > 0
    if (LW_NCPUS > 1) {                                                 /*  �������� SMP ���ģʽ       */
        if (API_ThreadStop(ulId)) {
            return  (PX_ERROR);
        }
    }
#endif                                                                  /*  LW_CFG_SMP_EN               */

    __KERNEL_ENTER();                                                   /*  �����ں�                    */
    if (_Thread_Invalid(usIndex)) {
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    ptcb = __GET_TCB_FROM_INDEX(usIndex);
    if (ptcb->TCB_iDeleteProcStatus) {
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
        _ErrorHandle(ERROR_THREAD_OTHER_DELETE);
        return  (PX_ERROR);
    }

    sigvalue.sival_ptr = pvSigValue;
    
    sendval = _doSigQueue(ptcb, iSigNo, sigvalue);

#if LW_CFG_SMP_EN > 0
    if (LW_NCPUS > 1) {                                                 /*  �������� SMP ���ģʽ       */
        _ThreadContinue(ptcb, LW_FALSE);                                /*  ���ں�״̬�»��ѱ�ֹͣ�߳�  */
    }
#endif                                                                  /*  LW_CFG_SMP_EN               */
    __KERNEL_EXIT();                                                    /*  �˳��ں�                    */
    
#if LW_CFG_SIGNALFD_EN > 0
    if (sendval == SEND_BLOCK) {
        _sigfdReadUnblock(ulId, iSigNo);
    }
#endif                                                                  /*  LW_CFG_SIGNALFD_EN > 0      */
    
    _ErrorHandle(ERROR_NONE);
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: sigqueue
** ��������: ���Ͷ��������ź�, ����ǽ���, �����͸������߳�.
** �䡡��  : ulId                    �߳� id ���� ���̺�
**           iSigNo                  �ź�
**           sigvalue                �ź� value
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  sigqueue (LW_OBJECT_HANDLE  ulId, INT   iSigNo, const union sigval  sigvalue)
{
    return  (sigqueue_internal(ulId, iSigNo, sigvalue.sival_ptr));
}
/*********************************************************************************************************
** ��������: pause
** ��������: �ȴ�һ���źŵĵ���
** �䡡��  : NONE
** �䡡��  : ERROR
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  pause (VOID)
{
             INTREG         iregInterLevel;
             PLW_CLASS_TCB  ptcbCur;
    REGISTER PLW_CLASS_PCB  ppcb;
    
    __THREAD_CANCEL_POINT();                                            /*  ����ȡ����                  */
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);                                       /*  ��ǰ������ƿ�              */

    MONITOR_EVT_LONG1(MONITOR_EVENT_ID_SIGNAL, MONITOR_EVENT_SIGNAL_PAUSE, 
                      ptcbCur->TCB_ulId, LW_NULL);
    
    iregInterLevel = __KERNEL_ENTER_IRQ();                              /*  �����ں�                    */
    
    ptcbCur->TCB_usStatus |= LW_THREAD_STATUS_SIGNAL;                   /*  �ȴ��ź�                    */
    ppcb = _GetPcb(ptcbCur);
    __DEL_FROM_READY_RING(ptcbCur, ppcb);                               /*  �Ӿ�������ɾ��              */
    
    __KERNEL_EXIT_IRQ(iregInterLevel);                                  /*  �˳��ں�                    */
    
    _ErrorHandle(EINTR);
    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: sigsuspend
** ��������: ʹ��ָ��������ȴ�һ����Ч�źŵĵ���, Ȼ�󷵻���ǰ���ź�����.
** �䡡��  : psigsetMask        ָ�����ź�����
** �䡡��  : ERROR
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  sigsuspend (const sigset_t  *psigsetMask)
{
             INTREG         iregInterLevel;
             PLW_CLASS_TCB  ptcbCur;
    REGISTER PLW_CLASS_PCB  ppcb;
             BOOL           bIsRun;
    
             PLW_CLASS_SIGCONTEXT   psigctx;
             sigset_t               sigsetOld;
             
    if (!psigsetMask) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    __THREAD_CANCEL_POINT();                                            /*  ����ȡ����                  */
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);                                       /*  ��ǰ������ƿ�              */
    
    MONITOR_EVT_D2(MONITOR_EVENT_ID_SIGNAL, MONITOR_EVENT_SIGNAL_SIGSUSPEND, 
                   ptcbCur->TCB_ulId, *psigsetMask, LW_NULL);
    
    __KERNEL_ENTER();                                                   /*  �����ں�                    */
    psigctx = _signalGetCtx(ptcbCur);
    
    sigsetOld = psigctx->SIGCTX_sigsetSigBlockMask;                     /*  ��¼��ǰ������              */
    psigctx->SIGCTX_sigsetSigBlockMask = *psigsetMask & (~__SIGNO_UNMASK);
    
    bIsRun = _sigPendRun(ptcbCur);
    if (bIsRun) {
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
        sigprocmask(SIG_SETMASK, &sigsetOld, LW_NULL);                  /*  ����Ϊԭ�ȵ� mask           */
        
        _ErrorHandle(EINTR);
        return  (PX_ERROR);
    }
    
    iregInterLevel = KN_INT_DISABLE();                                  /*  �ر��ж�                    */
    ptcbCur->TCB_usStatus |= LW_THREAD_STATUS_SIGNAL;                   /*  �ȴ��ź�                    */
    ppcb = _GetPcb(ptcbCur);
    __DEL_FROM_READY_RING(ptcbCur, ppcb);                               /*  �Ӿ�������ɾ��              */
    KN_INT_ENABLE(iregInterLevel);                                      /*  ���ж�                    */
    
    __KERNEL_EXIT();                                                    /*  �˳��ں�                    */
    
    sigprocmask(SIG_SETMASK, &sigsetOld, NULL);
    
    _ErrorHandle(EINTR);
    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: sigwait
** ��������: �ȴ� sigset ���źŵĵ������Դ��еķ�ʽ���źŶ�����ȡ���źŽ��д���, �źŽ����ٱ�ִ��.
** �䡡��  : psigset       ָ�����źż�
**           psiginfo      ��ȡ���ź���Ϣ
** �䡡��  : ERROR
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  sigwait (const sigset_t  *psigset, INT  *piSig)
{
             INTREG             iregInterLevel;
             PLW_CLASS_TCB      ptcbCur;
    REGISTER PLW_CLASS_PCB      ppcb;
    
             INT                    iSigNo;
             PLW_CLASS_SIGCONTEXT   psigctx;
             struct siginfo         siginfo;
             LW_CLASS_SIGWAIT       sigwt;
    
    if (!psigset) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    __THREAD_CANCEL_POINT();                                            /*  ����ȡ����                  */
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);                                       /*  ��ǰ������ƿ�              */
    
    MONITOR_EVT_D2(MONITOR_EVENT_ID_SIGNAL, MONITOR_EVENT_SIGNAL_SIGWAIT, 
                   ptcbCur->TCB_ulId, *psigset, LW_NULL);
    
    __KERNEL_ENTER();                                                   /*  �����ں�                    */
    psigctx = _signalGetCtx(ptcbCur);
    
    iSigNo = _sigPendGet(psigctx, psigset, &siginfo);                   /*  ��鵱ǰ�Ƿ��еȴ����ź�    */
    if (__issig(iSigNo)) {
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
        if (piSig) {
            *piSig = iSigNo;
        }
        _ErrorHandle(ERROR_NONE);
        return  (ERROR_NONE);
    }
    
    iregInterLevel = KN_INT_DISABLE();                                  /*  �ر��ж�                    */
    ptcbCur->TCB_usStatus |= LW_THREAD_STATUS_SIGNAL;                   /*  �ȴ��ź�                    */
    ppcb = _GetPcb(ptcbCur);
    __DEL_FROM_READY_RING(ptcbCur, ppcb);                               /*  �Ӿ�������ɾ��              */
    KN_INT_ENABLE(iregInterLevel);                                      /*  ���ж�                    */
    
    sigwt.SIGWT_sigset = *psigset;
    psigctx->SIGCTX_sigwait = &sigwt;                                   /*  ����ȴ���Ϣ                */
    
    if (__KERNEL_EXIT()) {                                              /*  �Ƿ������źż���            */
        psigctx->SIGCTX_sigwait = LW_NULL;
        _ErrorHandle(EINTR);                                            /*  SA_RESTART Ҳ�˳�           */
        return  (PX_ERROR);
    }
    
    psigctx->SIGCTX_sigwait = LW_NULL;
    if (piSig) {
        *piSig = sigwt.SIGWT_siginfo.si_signo;
    }
    
    _ErrorHandle(ERROR_NONE);
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: sigwaitinfo
** ��������: �ȴ� sigset ���źŵĵ������Դ��еķ�ʽ���źŶ�����ȡ���źŽ��д���, �źŽ����ٱ�ִ��.
** �䡡��  : psigset       ָ�����źż�
**           psiginfo      ��ȡ���ź���Ϣ
** �䡡��  : ERROR
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  sigwaitinfo (const sigset_t *psigset, struct  siginfo  *psiginfo)
{
             INTREG             iregInterLevel;
             PLW_CLASS_TCB      ptcbCur;
    REGISTER PLW_CLASS_PCB      ppcb;
    
             INT                    iSigNo;
             PLW_CLASS_SIGCONTEXT   psigctx;
             struct siginfo         siginfo;
             LW_CLASS_SIGWAIT       sigwt;
    
    if (!psigset) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    __THREAD_CANCEL_POINT();                                            /*  ����ȡ����                  */
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);                                       /*  ��ǰ������ƿ�              */
    
    MONITOR_EVT_D2(MONITOR_EVENT_ID_SIGNAL, MONITOR_EVENT_SIGNAL_SIGWAIT, 
                   ptcbCur->TCB_ulId, *psigset, LW_NULL);
    
    __KERNEL_ENTER();                                                   /*  �����ں�                    */
    psigctx = _signalGetCtx(ptcbCur);
    
    iSigNo = _sigPendGet(psigctx, psigset, &siginfo);                   /*  ��鵱ǰ�Ƿ��еȴ����ź�    */
    if (__issig(iSigNo)) {
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
        if (psiginfo) {
            *psiginfo = siginfo;
        }
        _ErrorHandle(ERROR_NONE);
        return  (siginfo.si_signo);
    }
    
    iregInterLevel = KN_INT_DISABLE();                                  /*  �ر��ж�                    */
    ptcbCur->TCB_usStatus |= LW_THREAD_STATUS_SIGNAL;                   /*  �ȴ��ź�                    */
    ppcb = _GetPcb(ptcbCur);
    __DEL_FROM_READY_RING(ptcbCur, ppcb);                               /*  �Ӿ�������ɾ��              */
    KN_INT_ENABLE(iregInterLevel);                                      /*  ���ж�                    */
    
    sigwt.SIGWT_sigset = *psigset;
    psigctx->SIGCTX_sigwait = &sigwt;                                   /*  ����ȴ���Ϣ                */
    
    if (__KERNEL_EXIT()) {                                              /*  �Ƿ������źż���            */
        psigctx->SIGCTX_sigwait = LW_NULL;
        _ErrorHandle(EINTR);                                            /*  SA_RESTART Ҳ�˳�           */
        return  (PX_ERROR);
    }
    
    psigctx->SIGCTX_sigwait = LW_NULL;
    if (psiginfo) {
        *psiginfo = sigwt.SIGWT_siginfo;
    }
    
    _ErrorHandle(ERROR_NONE);
    return  (sigwt.SIGWT_siginfo.si_signo);
}
/*********************************************************************************************************
** ��������: sigtimedwait
** ��������: �ȴ� sigset ���źŵĵ������Դ��еķ�ʽ���źŶ�����ȡ���źŽ��д���, �źŽ����ٱ�ִ��.
** �䡡��  : psigset        ָ�����źż�
**           psiginfo      ��ȡ���ź���Ϣ
**           ptv           ��ʱʱ�� (NULL ��ʾһֱ�ȴ�)
** �䡡��  : ERROR
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  sigtimedwait (const sigset_t *psigset, struct  siginfo  *psiginfo, const struct timespec *ptv)
{
             INTREG             iregInterLevel;
             PLW_CLASS_TCB      ptcbCur;
    REGISTER PLW_CLASS_PCB      ppcb;
    
             INT                    iSigNo;
             PLW_CLASS_SIGCONTEXT   psigctx;
             struct siginfo         siginfo;
             LW_CLASS_SIGWAIT       sigwt;
             
             ULONG                  ulTimeOut;
    
    if (!psigset) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    if (ptv == LW_NULL) {                                               /*  ���õȴ�                    */
        ulTimeOut = LW_OPTION_WAIT_INFINITE;
    } else {
        ulTimeOut = __timespecToTick(ptv);
    }
    
    __THREAD_CANCEL_POINT();                                            /*  ����ȡ����                  */
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);                                       /*  ��ǰ������ƿ�              */
    
    MONITOR_EVT_D2(MONITOR_EVENT_ID_SIGNAL, MONITOR_EVENT_SIGNAL_SIGWAIT, 
                   ptcbCur->TCB_ulId, *psigset, LW_NULL);
    
    __KERNEL_ENTER();                                                   /*  �����ں�                    */
    psigctx = _signalGetCtx(ptcbCur);
    
    iSigNo = _sigPendGet(psigctx, psigset, &siginfo);                   /*  ��鵱ǰ�Ƿ��еȴ����ź�    */
    if (__issig(iSigNo)) {
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
        if (psiginfo) {
            *psiginfo = siginfo;
        }
        _ErrorHandle(ERROR_NONE);
        return  (siginfo.si_signo);
    }
    
    if (ulTimeOut == LW_OPTION_NOT_WAIT) {                              /*  �����еȴ�                  */
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
        _ErrorHandle(EAGAIN);
        return  (PX_ERROR);
    }
    
    iregInterLevel = KN_INT_DISABLE();                                  /*  �ر��ж�                    */
    ptcbCur->TCB_usStatus |= LW_THREAD_STATUS_SIGNAL;                   /*  �ȴ��ź�                    */
    ptcbCur->TCB_ucWaitTimeOut  = LW_WAIT_TIME_CLEAR;                   /*  ��յȴ�ʱ��                */
    
    ppcb = _GetPcb(ptcbCur);
    __DEL_FROM_READY_RING(ptcbCur, ppcb);                               /*  �Ӿ�������ɾ��              */
    
    if (ulTimeOut != LW_OPTION_WAIT_INFINITE) {
        ptcbCur->TCB_ulDelay = ulTimeOut;                               /*  ���ó�ʱʱ��                */
        __ADD_TO_WAKEUP_LINE(ptcbCur);                                  /*  ����ȴ�ɨ����              */
    } else {
        ptcbCur->TCB_ulDelay = 0ul;
    }
    KN_INT_ENABLE(iregInterLevel);                                      /*  ���ж�                    */
    
    sigwt.SIGWT_sigset = *psigset;
    psigctx->SIGCTX_sigwait = &sigwt;                                   /*  ����ȴ���Ϣ                */
    
    if (__KERNEL_EXIT()) {                                              /*  �Ƿ������źż���            */
        psigctx->SIGCTX_sigwait = LW_NULL;
        _ErrorHandle(EINTR);                                            /*  SA_RESTART Ҳ�˳�           */
        return  (PX_ERROR);
    }
    
    __KERNEL_ENTER();                                                   /*  �����ں�                    */
    if (ptcbCur->TCB_ucWaitTimeOut == LW_WAIT_TIME_OUT) {               /*  �ȴ���ʱ                    */
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
        _ErrorHandle(EAGAIN);
        return  (PX_ERROR);
    }
    __KERNEL_EXIT();                                                    /*  �˳��ں�                    */
    
    psigctx->SIGCTX_sigwait = LW_NULL;
    if (psiginfo) {
        *psiginfo = sigwt.SIGWT_siginfo;
    }
    
    _ErrorHandle(ERROR_NONE);
    return  (sigwt.SIGWT_siginfo.si_signo);
}
#endif                                                                  /*  LW_CFG_SIGNAL_EN > 0        */
/*********************************************************************************************************
  END
*********************************************************************************************************/
