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
** ��   ��   ��: sysHookList.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2007 �� 04 �� 01 ��
**
** ��        ��: ϵͳ���Ӻ�����ͷ�ļ�
*********************************************************************************************************/
/*********************************************************************************************************
ע�⣺
      �û���ò�Ҫʹ���ں��ṩ�� hook ���ܣ��ں˵� hook ������Ϊϵͳ�� hook ����ģ�ϵͳ�� hook �ж�̬����
      �Ĺ��ܣ�һ��ϵͳ hook ���ܿ�����Ӷ������
      
      API_SystemHookDelete() ���õ�ʱ���ǳ���Ҫ�������� hook ɨ����ɨ��ʱ���ã����ܻᷢ��ɨ�������ѵ����
*********************************************************************************************************/

#ifndef  __SYSHOOKLIST_H
#define  __SYSHOOKLIST_H

/*********************************************************************************************************
  FUNCTION NODE
*********************************************************************************************************/

typedef struct {
    LW_LIST_LINE              FUNCNODE_lineManage;                      /*  ��������                    */
    LW_HOOK_FUNC              FUNCNODE_hookfuncPtr;                     /*  ���Ӻ���ָ��                */
    LW_RESOURCE_RAW           FUNCNODE_resraw;                          /*  ��Դ����ڵ�                */
} LW_FUNC_NODE;
typedef LW_FUNC_NODE         *PLW_FUNC_NODE;

/*********************************************************************************************************
  SYSTEM HOOK SERVICE ROUTINE
*********************************************************************************************************/

VOID  _SysCreateHook(LW_OBJECT_HANDLE  ulId, ULONG  ulOption);
VOID  _SysDeleteHook(LW_OBJECT_HANDLE  ulId, PVOID  pvReturnVal, PLW_CLASS_TCB  ptcb);
VOID  _SysSwapHook(LW_OBJECT_HANDLE   hOldThread, LW_OBJECT_HANDLE   hNewThread);
VOID  _SysTickHook(INT64   i64Tick);
VOID  _SysInitHook(LW_OBJECT_HANDLE  ulId, PLW_CLASS_TCB  ptcb);
VOID  _SysIdleHook(VOID);
VOID  _SysInitBeginHook(VOID);
VOID  _SysInitEndHook(INT  iError);
VOID  _SysRebootHook(INT  iRebootType);
VOID  _SysWatchDogHook(LW_OBJECT_HANDLE  ulId);

VOID  _SysObjectCreateHook(LW_OBJECT_HANDLE  ulId, ULONG  ulOption);
VOID  _SysObjectDeleteHook(LW_OBJECT_HANDLE  ulId);
VOID  _SysFdCreateHook(INT iFd, pid_t  pid);
VOID  _SysFdDeleteHook(INT iFd, pid_t  pid);

VOID  _SysCpuIdleEnterHook(LW_OBJECT_HANDLE  ulIdEnterFrom);
VOID  _SysCpuIdleExitHook(LW_OBJECT_HANDLE  ulIdExitTo);
VOID  _SysIntEnterHook(ULONG  ulVector, ULONG  ulNesting);
VOID  _SysIntExitHook(ULONG  ulVector, ULONG  ulNesting);

VOID  _SysStkOverflowHook(pid_t  pid, LW_OBJECT_HANDLE  ulId);
VOID  _SysFatalErrorHook(pid_t  pid, LW_OBJECT_HANDLE  ulId, struct siginfo *psiginfo);

VOID  _SysVpCreateHook(pid_t pid);
VOID  _SysVpDeleteHook(pid_t pid, INT iExitCode);

/*********************************************************************************************************
  GLOBAL VAR
*********************************************************************************************************/

#ifdef   __SYSHOOKLIST_MAIN_FILE
#define  __SYSHOOK_EXT
#else
#define  __SYSHOOK_EXT           extern
#endif

/*********************************************************************************************************
  SYSTEM HOOK NODE
*********************************************************************************************************/

typedef struct {
    PLW_LIST_LINE           HOOKCB_plineHookHeader;                     /*  ���Ӻ�����                  */
    PLW_LIST_LINE           HOOKCB_plineHookOp;                         /*  ��ǰ����������              */
    LW_SPINLOCK_DEFINE     (HOOKCB_slHook);                             /*  ������                      */
} LW_HOOK_CB;
typedef LW_HOOK_CB         *PLW_HOOK_CB;

/*********************************************************************************************************
  HOOK CONTROL BLOCK
*********************************************************************************************************/

__SYSHOOK_EXT LW_HOOK_CB         _G_hookcbCreate;                       /*  �߳̽�������                */
__SYSHOOK_EXT LW_HOOK_CB         _G_hookcbDelete;                       /*  �߳�ɾ������                */
__SYSHOOK_EXT LW_HOOK_CB         _G_hookcbSwap;                         /*  �߳��л�����                */
__SYSHOOK_EXT LW_HOOK_CB         _G_hookcbTick;                         /*  ʱ���жϹ���                */
__SYSHOOK_EXT LW_HOOK_CB         _G_hookcbInit;                         /*  �̳߳�ʼ������              */
__SYSHOOK_EXT LW_HOOK_CB         _G_hookcbIdle;                         /*  �����̹߳���                */
__SYSHOOK_EXT LW_HOOK_CB         _G_hookcbInitBegin;                    /*  ϵͳ��ʼ����ʼʱ����        */
__SYSHOOK_EXT LW_HOOK_CB         _G_hookcbInitEnd;                      /*  ϵͳ��ʼ������ʱ����        */
__SYSHOOK_EXT LW_HOOK_CB         _G_hookcbReboot;                       /*  ϵͳ������������            */
__SYSHOOK_EXT LW_HOOK_CB         _G_hookcbWatchDog;                     /*  �߳̿��Ź�����              */

__SYSHOOK_EXT LW_HOOK_CB         _G_hookcbObjectCreate;                 /*  �����ں˶�����            */
__SYSHOOK_EXT LW_HOOK_CB         _G_hookcbObjectDelete;                 /*  ɾ���ں˶�����            */
__SYSHOOK_EXT LW_HOOK_CB         _G_hookcbFdCreate;                     /*  �ļ���������������          */
__SYSHOOK_EXT LW_HOOK_CB         _G_hookcbFdDelete;                     /*  �ļ�������ɾ������          */

__SYSHOOK_EXT LW_HOOK_CB         _G_hookcbCpuIdleEnter;                 /*  CPU �������ģʽ            */
__SYSHOOK_EXT LW_HOOK_CB         _G_hookcbCpuIdleExit;                  /*  CPU �˳�����ģʽ            */
__SYSHOOK_EXT LW_HOOK_CB         _G_hookcbCpuIntEnter;                  /*  CPU �����ж�(�쳣)ģʽ      */
__SYSHOOK_EXT LW_HOOK_CB         _G_hookcbCpuIntExit;                   /*  CPU �˳��ж�(�쳣)ģʽ      */

__SYSHOOK_EXT LW_HOOK_CB         _G_hookcbStkOverflow;                  /*  ��ջ���                    */
__SYSHOOK_EXT LW_HOOK_CB         _G_hookcbFatalError;                   /*  ��������                    */

__SYSHOOK_EXT LW_HOOK_CB         _G_hookcbVpCreate;                     /*  ���̽�������                */
__SYSHOOK_EXT LW_HOOK_CB         _G_hookcbVpDelete;                     /*  ����ɾ������                */

#endif                                                                  /*  __SYSHOOKLIST_H             */
/*********************************************************************************************************
  END
*********************************************************************************************************/

