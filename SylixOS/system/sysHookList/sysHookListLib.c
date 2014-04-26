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
** ��   ��   ��: sysHookListLib.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2007 �� 04 �� 01 ��
**
** ��        ��: ϵͳ���Ӻ������ڲ���

** BUG
2007.11.07  _SysCreateHook() ���뽨��ʱ�� option ѡ��.
2007.11.13  ʹ���������������������ȫ��װ.
2007.11.18  ����ע��.
2008.03.02  ����ϵͳ���������ص�.
2008.03.10  ʹ�ð�ȫ���ƵĻص�������.
2009.04.09  �޸Ļص�����.
2010.08.03  ÿ���ص����ƿ�ʹ�ö����� spinlock.
2012.09.23  ��ʼ��ʱ��������ϵͳ�ص�, ���ǵ��û���һ�ε��� hook add ����ʱ�ٰ�װ.
2013.03.16  ������̴�����ɾ���ص�.
2013.12.12  �ж� hook ����������Ƕ�ײ�������.
2014.01.07  �������� hook ��������.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#define  __SYSHOOKLIST_MAIN_FILE
#include "../SylixOS/system/include/s_system.h"
/*********************************************************************************************************
  hook ������ģ��
*********************************************************************************************************/
#define __HOOK_TEMPLATE(hookcb, param)  \
        INTREG           iregInterLevel;    \
        PLW_FUNC_NODE    pfuncnode; \
            \
        LW_SPIN_LOCK_IRQ(&hookcb.HOOKCB_slHook, &iregInterLevel);   \
        hookcb.HOOKCB_plineHookOp = hookcb.HOOKCB_plineHookHeader;  \
        while (hookcb.HOOKCB_plineHookOp) { \
            \
            pfuncnode = (PLW_FUNC_NODE)hookcb.HOOKCB_plineHookOp;    \
            hookcb.HOOKCB_plineHookOp =  \
                _list_line_get_next(hookcb.HOOKCB_plineHookOp);  \
                \
            KN_INT_ENABLE(iregInterLevel);  \
            pfuncnode->FUNCNODE_hookfuncPtr param;  \
            iregInterLevel = KN_INT_DISABLE();  \
        }   \
        LW_SPIN_UNLOCK_IRQ(&hookcb.HOOKCB_slHook, iregInterLevel);
/*********************************************************************************************************
** ��������: _HookCbInit
** ��������: ��ʼ�����ӿ��ƿ�
** �䡡��  : phookcb       hook ���ƿ�
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID _HookCbInit (PLW_HOOK_CB   phookcb)
{
    phookcb->HOOKCB_plineHookHeader = LW_NULL;
    phookcb->HOOKCB_plineHookOp     = LW_NULL;
    LW_SPIN_INIT(&phookcb->HOOKCB_slHook);
}
/*********************************************************************************************************
** ��������: _HookListInit
** ��������: ��ʼ�����Ӻ�����
** �䡡��  : 
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID _HookListInit (VOID)
{
    _HookCbInit(&_G_hookcbCreate);
    _HookCbInit(&_G_hookcbDelete);
    _HookCbInit(&_G_hookcbSwap);
    _HookCbInit(&_G_hookcbTick);
    _HookCbInit(&_G_hookcbInit);
    _HookCbInit(&_G_hookcbIdle);
    _HookCbInit(&_G_hookcbInitBegin);
    _HookCbInit(&_G_hookcbInitEnd);
    _HookCbInit(&_G_hookcbReboot);
    _HookCbInit(&_G_hookcbWatchDog);
    
    _HookCbInit(&_G_hookcbObjectCreate);
    _HookCbInit(&_G_hookcbObjectDelete);
    _HookCbInit(&_G_hookcbFdCreate);
    _HookCbInit(&_G_hookcbFdDelete);
    
    _HookCbInit(&_G_hookcbCpuIdleEnter);
    _HookCbInit(&_G_hookcbCpuIdleExit);
    _HookCbInit(&_G_hookcbCpuIntEnter);
    _HookCbInit(&_G_hookcbCpuIntExit);
    
    _HookCbInit(&_G_hookcbVpCreate);
    _HookCbInit(&_G_hookcbVpDelete);
}
/*********************************************************************************************************
** ��������: _SysCreateHook
** ��������: �߳̽������ӷ�����
** �䡡��  : ulId                      �߳� Id
             ulOption                  ����ѡ��
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SysCreateHook (LW_OBJECT_HANDLE  ulId, ULONG   ulOption)
{
    __HOOK_TEMPLATE(_G_hookcbCreate, (ulId, ulOption));
}
/*********************************************************************************************************
** ��������: _SysDeleteHook
** ��������: �߳�ɾ�����ӷ�����
** �䡡��  : 
**           ulId                      �߳� Id
**           pvReturnVal               �̷߳���ֵ
**           ptcb                      �߳� TCB
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SysDeleteHook (LW_OBJECT_HANDLE  ulId, PVOID  pvReturnVal, PLW_CLASS_TCB  ptcb)
{
    __HOOK_TEMPLATE(_G_hookcbDelete, (ulId, pvReturnVal, ptcb));
}
/*********************************************************************************************************
** ��������: _SysSwapHook
** ��������: �߳�ɾ�����ӷ�����
** �䡡��  : hOldThread        ���߳�
**           hNewThread        ���߳�
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SysSwapHook (LW_OBJECT_HANDLE   hOldThread, LW_OBJECT_HANDLE   hNewThread)
{
    INTREG          iregInterLevel;
    PLW_FUNC_NODE   pfuncnode;
    
    LW_SPIN_LOCK_QUICK(&_G_hookcbSwap.HOOKCB_slHook, &iregInterLevel);
    _G_hookcbSwap.HOOKCB_plineHookOp = _G_hookcbSwap.HOOKCB_plineHookHeader;
    while (_G_hookcbSwap.HOOKCB_plineHookOp) {
        
        pfuncnode = (PLW_FUNC_NODE)_G_hookcbSwap.HOOKCB_plineHookOp;
        _G_hookcbSwap.HOOKCB_plineHookOp = 
            _list_line_get_next(_G_hookcbSwap.HOOKCB_plineHookOp);
        
        KN_INT_ENABLE(iregInterLevel);
        (pfuncnode->FUNCNODE_hookfuncPtr)(hOldThread, hNewThread);      /*  ������ı�����״̬          */
        iregInterLevel = KN_INT_DISABLE();
    }
    LW_SPIN_UNLOCK_QUICK(&_G_hookcbSwap.HOOKCB_slHook, iregInterLevel);
}
/*********************************************************************************************************
** ��������: _SysTickHook
** ��������: ʱ���жϹ��ӷ�����
** �䡡��  : i64Tick   ��ǰ tick
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SysTickHook (INT64   i64Tick)
{
    INTREG           iregInterLevel;
    PLW_FUNC_NODE    pfuncnode;
    
    LW_SPIN_LOCK_QUICK(&_G_hookcbTick.HOOKCB_slHook, &iregInterLevel);
    _G_hookcbTick.HOOKCB_plineHookOp = _G_hookcbTick.HOOKCB_plineHookHeader;
    while (_G_hookcbTick.HOOKCB_plineHookOp) {
        
        pfuncnode = (PLW_FUNC_NODE)_G_hookcbTick.HOOKCB_plineHookOp;
        _G_hookcbTick.HOOKCB_plineHookOp = 
            _list_line_get_next(_G_hookcbTick.HOOKCB_plineHookOp);
        
        KN_INT_ENABLE(iregInterLevel);
        (pfuncnode->FUNCNODE_hookfuncPtr)(i64Tick);                     /*  ������ı�����״̬          */
        iregInterLevel = KN_INT_DISABLE();
    }
    LW_SPIN_UNLOCK_QUICK(&_G_hookcbTick.HOOKCB_slHook, iregInterLevel);
}
/*********************************************************************************************************
** ��������: _SysInitHook
** ��������: �̳߳�ʼ�����Ӻ�����
** �䡡��  : ulId                      �߳� Id
**           ptcb                      �߳� TCB
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SysInitHook (LW_OBJECT_HANDLE  ulId, PLW_CLASS_TCB  ptcb)
{
    __HOOK_TEMPLATE(_G_hookcbInit, (ulId, ptcb));
}
/*********************************************************************************************************
** ��������: _SysIdleHook
** ��������: �����̹߳��Ӻ�����
** �䡡��  : 
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SysIdleHook (VOID)
{
    PLW_FUNC_NODE    pfuncnode;
    
    _G_hookcbIdle.HOOKCB_plineHookOp = _G_hookcbIdle.HOOKCB_plineHookHeader;
    while (_G_hookcbIdle.HOOKCB_plineHookOp) {
    
        pfuncnode = (PLW_FUNC_NODE)_G_hookcbIdle.HOOKCB_plineHookOp;
        _G_hookcbIdle.HOOKCB_plineHookOp = 
            _list_line_get_next(_G_hookcbIdle.HOOKCB_plineHookOp);
        
        (pfuncnode->FUNCNODE_hookfuncPtr)();
    }
}
/*********************************************************************************************************
** ��������: _SysInitBeginHook
** ��������: ϵͳ��ʼ����ʼʱ����
** �䡡��  : 
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SysInitBeginHook (VOID)
{
    __HOOK_TEMPLATE(_G_hookcbInitBegin, ());
}
/*********************************************************************************************************
** ��������: _SysInitEndHook
** ��������: ϵͳ��ʼ������ʱ����
** �䡡��  : iError                    ����ϵͳ��ʼ���Ƿ���ִ���   0 �޴���   1 ����
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SysInitEndHook (INT  iError)
{
    __HOOK_TEMPLATE(_G_hookcbInitEnd, (iError));
}
/*********************************************************************************************************
** ��������: _SysRebootHook
** ��������: ϵͳ��������
** �䡡��  : iRebootType                ϵͳ������������
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SysRebootHook (INT  iRebootType)
{
    __HOOK_TEMPLATE(_G_hookcbReboot, (iRebootType));
}
/*********************************************************************************************************
** ��������: _SysWatchDogHook
** ��������: �߳̿��Ź����Ӻ�����
** �䡡��  : ulId                      �߳� Id
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SysWatchDogHook (LW_OBJECT_HANDLE  ulId)
{
    __HOOK_TEMPLATE(_G_hookcbWatchDog, (ulId));
}
/*********************************************************************************************************
** ��������: _SysObjectCreateHook
** ��������: �����ں˶�����
** �䡡��  : ulId                      �߳� Id
**           ulOption                  ����ѡ��
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SysObjectCreateHook (LW_OBJECT_HANDLE  ulId, ULONG  ulOption)
{
    __HOOK_TEMPLATE(_G_hookcbObjectCreate, (ulId, ulOption));
}
/*********************************************************************************************************
** ��������: _SysObjectDeleteHook
** ��������: ɾ���ں˶�����
** �䡡��  : ulId                      �߳� Id
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SysObjectDeleteHook (LW_OBJECT_HANDLE  ulId)
{
    __HOOK_TEMPLATE(_G_hookcbObjectDelete, (ulId));
}
/*********************************************************************************************************
** ��������: _SysFdCreateHook
** ��������: �ļ���������������
** �䡡��  : iFd                       �ļ�������
**           pid                       ����id
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SysFdCreateHook (INT iFd, pid_t  pid)
{
    __HOOK_TEMPLATE(_G_hookcbFdCreate, (iFd, pid));
}
/*********************************************************************************************************
** ��������: _SysFdDeleteHook
** ��������: �ļ�������ɾ������
** �䡡��  : iFd                       �ļ�������
**           pid                       ����id
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SysFdDeleteHook (INT iFd, pid_t  pid)
{
    __HOOK_TEMPLATE(_G_hookcbFdDelete, (iFd, pid));
}
/*********************************************************************************************************
** ��������: _SysCpuIdleEnterHook
** ��������: CPU �������ģʽ
** �䡡��  : ulIdEnterFrom             ���ĸ��߳̽��� idle
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SysCpuIdleEnterHook (LW_OBJECT_HANDLE  ulIdEnterFrom)
{
    __HOOK_TEMPLATE(_G_hookcbCpuIdleEnter, (ulIdEnterFrom));
}
/*********************************************************************************************************
** ��������: _SysCpuIdleExitHook
** ��������: CPU �˳�����ģʽ
** �䡡��  : ulIdExitTo                �˳� idle �߳̽����ĸ��߳�
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SysCpuIdleExitHook (LW_OBJECT_HANDLE  ulIdExitTo)
{
    __HOOK_TEMPLATE(_G_hookcbCpuIdleExit, (ulIdExitTo));
}
/*********************************************************************************************************
** ��������: _SysIntEnterHook
** ��������: CPU �����ж�(�쳣)ģʽ
** �䡡��  : ulVector      �ж�����
**           ulNesting     ��ǰǶ�ײ���
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SysIntEnterHook (ULONG  ulVector, ULONG  ulNesting)
{
    __HOOK_TEMPLATE(_G_hookcbCpuIntEnter, (ulVector, ulNesting));
}
/*********************************************************************************************************
** ��������: _SysIntExitHook
** ��������: CPU �˳��ж�(�쳣)ģʽ
** �䡡��  : ulVector      �ж�����
**           ulNesting     ��ǰǶ�ײ���
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SysIntExitHook (ULONG  ulVector, ULONG  ulNesting)
{
    __HOOK_TEMPLATE(_G_hookcbCpuIntExit, (ulVector, ulNesting));
}
/*********************************************************************************************************
** ��������: _SysVpCreateHook
** ��������: ���̽������ӷ�����
** �䡡��  : pid                       ���� id
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SysVpCreateHook (pid_t pid)
{
    __HOOK_TEMPLATE(_G_hookcbVpCreate, (pid));
}
/*********************************************************************************************************
** ��������: _SysVpDeleteHook
** ��������: ����ɾ�����ӷ�����
** �䡡��  : pid                       ���� id
**           iExitCode                 ���̷���ֵ
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SysVpDeleteHook (pid_t pid, INT iExitCode)
{
    __HOOK_TEMPLATE(_G_hookcbVpDelete, (pid, iExitCode));
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
