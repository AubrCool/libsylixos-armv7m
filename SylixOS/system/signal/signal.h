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
** ��   ��   ��: signal.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2007 �� 06 �� 03 ��
**
** ��        ��: ����ϵͳ�ź���ض���.

** BUG:
2011.11.22  ���źŸ��������� 1 ~ 63.
2012.12.09  ����Ա�׼�ź� code ��֧��.
2013.05.05  ���� SA_RESTART ֧��.
*********************************************************************************************************/

#ifndef  __SIGNAL_H
#define  __SIGNAL_H

#if  LW_CFG_SIGNAL_EN > 0
/*********************************************************************************************************
  SIGNAL NUMBER �����޸�!
  
  �źž�����������ж�, ��ִ���źž��ʱ, �����ǲ�������ô����������ܵĺ���.
  SylixOS �涨�źž�����Բ�������ÿ��ܻ��� suspend �����ĺ���, ����: API_ThreadSuspend
  API_ThreadDelete...
*********************************************************************************************************/

/*********************************************************************************************************
  ע��:
  �ź����������:
  
  �ź�����: ��ǰ����� kill �źŵ���, �ұ�����, ��ô�źŽ��ò�������. ����, һ�����������, �������յ�����
            �� kill ���ź�, ���ᱻִ��һ��. 
            ��ǰ����� queue �źŵ���, �ұ�����, ��ô�źŽ��ò�������. ����, һ�����������, �����εĹ�����
            ���յ������ź�, �����ж��ٴ�.
            ��ǰ����������źŵ���, �� timer, aio, msgq �ȵ�, ��ֱ�Ӻ���.
            
  �źź���: ���źŵĴ�����Ϊ����ʱ, ���յ����źŻ�ֱ������, ��ȫ��������״̬.
*********************************************************************************************************/

#define  NSIG                   63                                      /*  �ź�����                    */

/*********************************************************************************************************
  �ź�����
*********************************************************************************************************/

#define  SIGHUP                 1                                       /*  �ҶϿ����ն˻����          */
#define  SIGINT                 2                                       /*  ���Լ��̵��ж�              */
#define  SIGQUIT                3                                       /*  ���Լ��̵��˳�              */
#define  SIGILL                 4                                       /*  �Ƿ�ָ��                    */
#define  SIGTRAP                5                                       /*  ���ٶϵ�                    */
#define  SIGABRT                6                                       /*  �쳣����                    */
#define  SIGUNUSED              7                                       /*  û��ʹ��                    */
#define  SIGFPE                 8                                       /*  Э����������                */
#define  SIGKILL                9                                       /*  ǿ�Ƚ��̽���                */
#define  SIGBUS                10                                       /*  bus error                   */
#define  SIGSEGV               11                                       /*  ��Ч�ڴ�����                */
#define  SIGUNUSED2            12                                       /*  ��ʱû��ʹ��2               */
#define  SIGPIPE               13                                       /*  �ܵ�д�����޶���          */
#define  SIGALRM               14                                       /*  ʵʱ��ʱ������              */
#define  SIGTERM               15                                       /*  ������ֹ                    */
#define  SIGCNCL               16                                       /*  �߳�ȡ���ź�                */
#define  SIGSTOP               17                                       /*  ֹͣ����ִ��                */
#define  SIGTSTP               18                                       /*  tty����ֹͣ����             */
#define  SIGCONT               19                                       /*  �ָ����̼���ִ��            */
#define  SIGCHLD               20                                       /*  �ӽ���ֹͣ����ֹ          */
#define  SIGTTIN               21                                       /*  ��̨������������            */
#define  SIGTTOU               22                                       /*  ��̨�����������            */
#define  SIGCANCEL             SIGTERM

#define  SIGUSR1               30                                       /*  user defined signal 1       */
#define  SIGUSR2               31                                       /*  user defined signal 2       */

#define  SIGPOLL               32                                       /*  pollable event              */
#define  SIGPROF               33                                       /*  profiling timer expired     */
#define  SIGSYS                34                                       /*  bad system call             */
#define  SIGURG                35                                       /*  high bandwidth data is      */
                                                                        /*  available at socket         */
#define  SIGVTALRM             36                                       /*  virtual timer expired       */
#define  SIGXCPU               37                                       /*  CPU time limit exceeded     */
#define  SIGXFSZ               38                                       /* file size time limit exceeded*/

#define  SIGWINCH              41

/*********************************************************************************************************
  ʵʱ�źŷ�Χ
*********************************************************************************************************/

#define  SIGRTMIN              48                                       /*  ��Сʵʱ�ź�                */
#define  SIGRTMAX              63                                       /*  ���ʵʱ�ź�                */

/*********************************************************************************************************
  ����ִ�е��źž������
*********************************************************************************************************/

#define  SIG_ERR               (PSIGNAL_HANDLE)-1                       /*  �����źž��                */
#define  SIG_DFL               (PSIGNAL_HANDLE)0                        /*  Ĭ���źž��                */
#define  SIG_IGN               (PSIGNAL_HANDLE)1                        /*�����Ե��źž��������        */

/*********************************************************************************************************
  sa_flag ����
*********************************************************************************************************/

#define  SA_NOCLDSTOP          0x00000001                               /*  �ӽ��̱�ɾ��ʱ��Ҫ����      */
                                                                        /*  SIGCHLD�ź�                 */
#define  SA_SIGINFO            0x00000002                               /*  �źž����Ҫ siginfo ����   */

#define  SA_RESTART            0x10000000                               /*  ִ���źž����, ��������    */
#define  SA_INTERRUPT          0x20000000
#define  SA_NOMASK             0x40000000                               /*  ����ֹ��ָ���źŴ�������  */
                                                                        /*  ���յ��ź�                  */
#define  SA_ONESHOT            0x80000000                               /*  �źž��һ�������þͻָ���  */
                                                                        /*  Ĭ��״̬                    */

#define  SA_RESETHAND          SA_ONESHOT                               /*  ִ�о����, ���źž������Ϊ*/
                                                                        /*  ����                        */
/*********************************************************************************************************
  sigprocmask() �ı��źż�������
*********************************************************************************************************/

#define  SIG_BLOCK             1                                        /*  �������źż��ϸ������źż�  */
#define  SIG_UNBLOCK           2                                        /*  �������źż���ɾ�����źż�  */
#define  SIG_SETMASK           3                                        /*  �����ź�������              */

/*********************************************************************************************************
  �źŲ���Դ����
*********************************************************************************************************/

#define  SI_KILL               0                                        /* ʹ�� kill() ���͵��ź�       */
#define  SI_QUEUE              2                                        /* ʹ�� sigqueue() ���͵��ź�   */
#define  SI_TIMER              3                                        /* POSIX ��ʱ��������ź�       */
#define  SI_ASYNCIO            4                                        /* �첽 I/O ϵͳ��ɷ��͵��ź�  */
#define  SI_MESGQ              5                                        /* ���յ�һ����Ϣ�������ź�     */

/*********************************************************************************************************
  �ź� code
*********************************************************************************************************/
/*********************************************************************************************************
  SIGILL
*********************************************************************************************************/

#define  ILL_ILLOPC            1                                        /* Illegal opcode               */
#define  ILL_ILLOPN            2                                        /* Illegal operand              */
#define  ILL_ILLADR            3                                        /* Illegal addressing mode      */
#define  ILL_ILLTRP            4                                        /* Illegal trap                 */
#define  ILL_PRVOPC            5                                        /* Priviledged instruction      */
#define  ILL_PRVREG            6                                        /* Priviledged register         */
#define  ILL_COPROC            7                                        /* Coprocessor error            */
#define  ILL_BADSTK            8                                        /* Internal stack error         */

/*********************************************************************************************************
  SIGFPE
*********************************************************************************************************/

#define  FPE_INTDIV            1                                        /* Integer divide by zero       */
#define  FPE_INTOVF            2                                        /* Integer overflow             */
#define  FPE_FLTDIV            3                                        /* Floating-point divide by zero*/
#define  FPE_FLTOVF            4                                        /* Floating-point overflow      */
#define  FPE_FLTUND            5                                        /* Floating-point underflow     */
#define  FPE_FLTRES            6                                        /* Floating-point inexact result*/
#define  FPE_FLTSUB            7                                        /* Subscript out of range       */

/*********************************************************************************************************
  SIGSEGV
*********************************************************************************************************/

#define  SEGV_MAPERR           1                                        /* Address not mapped to object */
#define  SEGV_ACCERR           2                                        /* Invalid permissions for      */
                                                                        /* mapped object                */
/*********************************************************************************************************
  SIGBUS
*********************************************************************************************************/

#define  BUS_ADRALN            1                                        /* Invalid address alignment    */
#define  BUS_ADRERR            2                                        /* Nonexistent physical memory  */
#define  BUS_OBJERR            3                                        /* Object-specific hardware err */

/*********************************************************************************************************
  SIGTRAP
*********************************************************************************************************/

#define  TRAP_BRKPT            1                                        /* Process breakpoint           */
#define  TRAP_TRACE            2                                        /* Process trap trace           */

/*********************************************************************************************************
  SIGCHLD
*********************************************************************************************************/

#define  CLD_EXITED            1                                        /* Child has exited             */
#define  CLD_KILLED            2                                        /* Child has terminated abnormally (no core) */
#define  CLD_DUMPED            3                                        /* Child has terminated abnormally           */
#define  CLD_TRAPPED           4                                        /* Traced child has trapped     */
#define  CLD_STOPPED           5                                        /* Child has stopped            */
#define  CLD_CONTINUED         6                                        /* Stopped child has continued  */

/*********************************************************************************************************
  SIGPOLL
*********************************************************************************************************/

#define  POLL_IN               1                                        /* Data input available         */
#define  POLL_OUT              2                                        /* Output buffers available     */
#define  POLL_MSG              3                                        /* Input message available      */
#define  POLL_ERR              4                                        /* I/O error                    */
#define  POLL_PRI              5                                        /* High priority input available*/
#define  POLL_HUP              6                                        /* Device disconnected          */

/*********************************************************************************************************
  ��������
*********************************************************************************************************/

#ifdef __SYLIXOS_KERNEL
#define  __issig(m)            (1 <= (m) && (m) <= NSIG)                /*  �ź��Ƿ���Ч                */
#define  __sigmask(m)          (sigset_t)((1 << ((m) - 1)))             /*  ����ź�����                */
#define  __sigindex(m)         (m - 1)                                  /*  �������±�                  */
#endif                                                                  /*  __SYLIXOS_KERNEL            */

/*********************************************************************************************************
  struct sigevent ��, sigev_notify ����
  
  SIGEV_THREAD ��Ҫ POSIX ��֧��.
*********************************************************************************************************/

#define SIGEV_NONE             1                                        /*  No asynchronous notification*/
#define SIGEV_SIGNAL           2                                        /*  A queued signal             */

#if LW_CFG_POSIX_EN > 0
#define SIGEV_THREAD           3                                        /*  invoke sigev_notify_function*/
                                                                        /*  as if it were the start     */
                                                                        /*  function of a new thread.   */
                                                                        
#define SIGEV_THREAD_ID        4                                        /*  As for SIGEV_SIGNAL, but the*/
                                                                        /*  signal is targeted at the   */
                                                                        /*  thread whose ID is given in */
                                                                        /*  sigev_notify_thread_id      */
#endif                                                                  /*  LW_CFG_POSIX_EN > 0         */
/*********************************************************************************************************
  SIGNAL FUNCTION 
*********************************************************************************************************/

LW_API  VOID      (*signal(INT   iSigNo, PSIGNAL_HANDLE  pfuncHandler))(INT);
LW_API  INT         raise(INT    iSigNo);
LW_API  INT         kill(LW_OBJECT_HANDLE  ulId, INT   iSigNo);
LW_API  INT         sigqueue(LW_OBJECT_HANDLE  ulId, INT   iSigNo, const union   sigval   sigvalue);

LW_API  INT         sigemptyset(sigset_t       *psigset);
LW_API  INT         sigfillset(sigset_t        *psigset);
LW_API  INT         sigaddset(sigset_t         *psigset, INT  iSigNo);
LW_API  INT         sigdelset(sigset_t         *psigset, INT  iSigNo);
LW_API  INT         sigismember(const sigset_t *psigset, INT  iSigNo);

LW_API  INT         sigaction(INT  iSigNo, const struct sigaction  *psigactionNew,
                              struct sigaction  *psigactionOld);

LW_API  INT         sigsetmask(INT  iMask);                         /*  ����ʹ�� sigprocmask       */
LW_API  INT         sigblock(INT    iBlock);                        /*  ����ʹ�� sigprocmask       */

LW_API  INT         sigprocmask(INT  iHow, const sigset_t *sigset, sigset_t *sigsetOld);

LW_API  INT         sigvec(INT  iSigNo, const struct sigvec *pvec, struct sigvec *pvecOld);

LW_API  INT         sigpending(sigset_t  *sigset);

LW_API  INT         sigsuspend(const sigset_t  *sigsetMask);
LW_API  INT         pause(VOID);
LW_API  INT         sigwait(const sigset_t      *sigset, INT  *piSig);
LW_API  INT         sigtimedwait(const sigset_t *sigset, struct siginfo *__value,
                                 const struct timespec *);
LW_API  INT         sigwaitinfo(const sigset_t *sigset, struct  siginfo  *psiginfo);

/*********************************************************************************************************
  SIGNAL �ں˺���
*********************************************************************************************************/

#ifdef __SYLIXOS_KERNEL
LW_API  INT         sigTrap(LW_OBJECT_HANDLE  ulId, const union sigval  sigvalue);
#endif                                                                  /*  __SYLIXOS_KERNEL            */

#endif                                                                  /*  LW_CFG_SIGNAL_EN > 0        */
#endif                                                                  /*  __SIGNAL_H                  */
/*********************************************************************************************************
  END
*********************************************************************************************************/
