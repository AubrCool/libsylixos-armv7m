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
** ��   ��   ��: HookList.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2007 �� 04 �� 20 ��
**
** ��        ��: ϵͳ���Ӻ�������, 

** BUG
2007.08.22  API_SystemHookAdd    ������ʱ��û���ͷŵ��ڴ档
2007.08.22  API_SystemHookDelete �ڲ����ؼ�������û�йر��жϡ�
2007.09.21  ���� _DebugHandle() ���ܡ�
2007.11.13  ʹ���������������������ȫ��װ.
2007.11.18  ����ע��.
2008.03.02  ����ϵͳ���������ص�.
2008.03.10  ���밲ȫ�������.
2008.05.18  ʹ�� __KERNEL_ENTER() ���� ThreadLock(); 
2009.12.09  �޸�ע��.
2010.08.03  ÿ���ص����ƿ�ʹ�ö����� spinlock.
2012.09.22  ��������Դ����� HOOK.
2012.09.23  ��ʼ��ʱ��������ϵͳ�ص�, ���ǵ��û���һ�ε��� hook add ����ʱ�ٰ�װ.
2012.12.08  ������Դ���յĹ���.
2013.03.16  ������̻ص�.
2013.05.02  �����Ѿ�������Դ����, ��������װ�ص�.
*********************************************************************************************************/
/*********************************************************************************************************
ע�⣺
      �û���ò�Ҫʹ���ں��ṩ�� hook ���ܣ��ں˵� hook ������Ϊϵͳ�� hook ����ģ�ϵͳ�� hook �ж�̬����
      �Ĺ��ܣ�һ��ϵͳ hook ���ܿ�����Ӷ������
      
      API_SystemHookDelete() ���õ�ʱ���ǳ���Ҫ�������� hook ɨ����ɨ��ʱ���ã����ܻᷢ��ɨ�������ѵ����
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
/*********************************************************************************************************
  �������
*********************************************************************************************************/
#if LW_CFG_MODULELOADER_EN > 0
#include "../SylixOS/loader/include/loader_vppatch.h"
#endif                                                                  /*  LW_CFG_MODULELOADER_EN > 0  */
/*********************************************************************************************************
** ��������: API_SystemHookAdd
** ��������: ���һ��ϵͳ hook ���ܺ���
** �䡡��  : 
**           hookfuncPtr                   HOOK ���ܺ���
**           ulOpt                         HOOK ����
** �䡡��  : ERROR
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
ULONG  API_SystemHookAdd (LW_HOOK_FUNC  hookfuncPtr, ULONG  ulOpt)
{
             INTREG           iregInterLevel;
             PLW_FUNC_NODE    pfuncnode;
    REGISTER PLW_LIST_LINE    pline;
    
    if (LW_CPU_GET_CUR_NESTING()) {                                     /*  �������ж��е���            */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "called from ISR.\r\n");
        _ErrorHandle(ERROR_KERNEL_IN_ISR);
        return  (ERROR_KERNEL_IN_ISR);
    }
             
#if LW_CFG_ARG_CHK_EN > 0
    if (!hookfuncPtr) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "hookfuncPtr invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HOOK_NULL);
        return  (ERROR_KERNEL_HOOK_NULL);
    }
#endif
    
    pfuncnode = (PLW_FUNC_NODE)__SHEAP_ALLOC(sizeof(LW_FUNC_NODE));     /*  ������ƿ��ڴ�              */
    if (!pfuncnode) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "system low memory.\r\n");
        _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);                          /*  ȱ���ڴ�                    */
        return  (ERROR_SYSTEM_LOW_MEMORY);
    }
    
    pline = &pfuncnode->FUNCNODE_lineManage;                            /*  ����������ָ��              */
    pfuncnode->FUNCNODE_hookfuncPtr = hookfuncPtr;
    
    _ErrorHandle(ERROR_NONE);
    
    switch (ulOpt) {
    
    case LW_OPTION_THREAD_CREATE_HOOK:                                  /*  �߳̽�������                */
        LW_SPIN_LOCK_QUICK(&_G_hookcbCreate.HOOKCB_slHook, &iregInterLevel);
        if (_G_hookcbCreate.HOOKCB_plineHookHeader == LW_NULL) {
            _K_hookKernel.HOOK_ThreadCreate = _SysCreateHook;
        }
        _List_Line_Add_Ahead(pline, &_G_hookcbCreate.HOOKCB_plineHookHeader);
        LW_SPIN_UNLOCK_QUICK(&_G_hookcbCreate.HOOKCB_slHook, iregInterLevel);
        break;
        
    case LW_OPTION_THREAD_DELETE_HOOK:                                  /*  �߳�ɾ������                */
        LW_SPIN_LOCK_QUICK(&_G_hookcbDelete.HOOKCB_slHook, &iregInterLevel);
        if (_G_hookcbDelete.HOOKCB_plineHookHeader == LW_NULL) {
            _K_hookKernel.HOOK_ThreadDelete = _SysDeleteHook;
        }
        _List_Line_Add_Ahead(pline, &_G_hookcbDelete.HOOKCB_plineHookHeader);
        LW_SPIN_UNLOCK_QUICK(&_G_hookcbDelete.HOOKCB_slHook, iregInterLevel);
        break;
        
    case LW_OPTION_THREAD_SWAP_HOOK:                                    /*  �߳��л�����                */
        LW_SPIN_LOCK_QUICK(&_G_hookcbSwap.HOOKCB_slHook, &iregInterLevel);
        if (_G_hookcbSwap.HOOKCB_plineHookHeader == LW_NULL) {
            _K_hookKernel.HOOK_ThreadSwap = _SysSwapHook;
        }
        _List_Line_Add_Ahead(pline, &_G_hookcbSwap.HOOKCB_plineHookHeader);
        LW_SPIN_UNLOCK_QUICK(&_G_hookcbSwap.HOOKCB_slHook, iregInterLevel);
        break;
        
    case LW_OPTION_THREAD_TICK_HOOK:                                    /*  ϵͳʱ���жϹ���            */
        LW_SPIN_LOCK_QUICK(&_G_hookcbTick.HOOKCB_slHook, &iregInterLevel);
        if (_G_hookcbTick.HOOKCB_plineHookHeader == LW_NULL) {
            _K_hookKernel.HOOK_ThreadTick = _SysTickHook;
        }
        _List_Line_Add_Ahead(pline, &_G_hookcbTick.HOOKCB_plineHookHeader);
        LW_SPIN_UNLOCK_QUICK(&_G_hookcbTick.HOOKCB_slHook, iregInterLevel);
        break;
        
    case LW_OPTION_THREAD_INIT_HOOK:                                    /*  �̳߳�ʼ������              */
        LW_SPIN_LOCK_QUICK(&_G_hookcbInit.HOOKCB_slHook, &iregInterLevel);
        if (_G_hookcbInit.HOOKCB_plineHookHeader == LW_NULL) {
            _K_hookKernel.HOOK_ThreadInit = _SysInitHook;
        }
        _List_Line_Add_Ahead(pline, &_G_hookcbInit.HOOKCB_plineHookHeader);
        LW_SPIN_UNLOCK_QUICK(&_G_hookcbInit.HOOKCB_slHook, iregInterLevel);
        break;
        
    case LW_OPTION_THREAD_IDLE_HOOK:                                    /*  �����̹߳���                */
        if (API_KernelIsRunning()) {
            __SHEAP_FREE(pfuncnode);                                    /*  �ͷ��ڴ�                    */
            _DebugHandle(__ERRORMESSAGE_LEVEL, "can not add idle hook in running status.\r\n");
            _ErrorHandle(ERROR_KERNEL_RUNNING);
            return  (ERROR_KERNEL_RUNNING);
        }
        if (_G_hookcbIdle.HOOKCB_plineHookHeader == LW_NULL) {
            _K_hookKernel.HOOK_ThreadIdle = _SysIdleHook;
        }
        _List_Line_Add_Ahead(pline, &_G_hookcbIdle.HOOKCB_plineHookHeader);
        break;
        
    case LW_OPTION_KERNEL_INITBEGIN:                                    /*  �ں˳�ʼ����ʼ����          */
        LW_SPIN_LOCK_QUICK(&_G_hookcbInitBegin.HOOKCB_slHook, &iregInterLevel);
        if (_G_hookcbInitBegin.HOOKCB_plineHookHeader == LW_NULL) {
            _K_hookKernel.HOOK_KernelInitBegin = _SysInitBeginHook;
        }
        _List_Line_Add_Ahead(pline, &_G_hookcbInitBegin.HOOKCB_plineHookHeader);
        LW_SPIN_UNLOCK_QUICK(&_G_hookcbInitBegin.HOOKCB_slHook, iregInterLevel);
        break;
        
    case LW_OPTION_KERNEL_INITEND:                                      /*  �ں˳�ʼ����������          */
        LW_SPIN_LOCK_QUICK(&_G_hookcbInitEnd.HOOKCB_slHook, &iregInterLevel);
        if (_G_hookcbInitEnd.HOOKCB_plineHookHeader == LW_NULL) {
            _K_hookKernel.HOOK_KernelInitEnd = _SysInitEndHook;
        }
        _List_Line_Add_Ahead(pline, &_G_hookcbInitEnd.HOOKCB_plineHookHeader);
        LW_SPIN_UNLOCK_QUICK(&_G_hookcbInitEnd.HOOKCB_slHook, iregInterLevel);
        break;
        
    case LW_OPTION_KERNEL_REBOOT:                                       /*  �ں���������                */
        LW_SPIN_LOCK_QUICK(&_G_hookcbReboot.HOOKCB_slHook, &iregInterLevel);
        if (_G_hookcbReboot.HOOKCB_plineHookHeader == LW_NULL) {
            _K_hookKernel.HOOK_KernelReboot = _SysRebootHook;
        }
        _List_Line_Add_Ahead(pline, &_G_hookcbReboot.HOOKCB_plineHookHeader);
        LW_SPIN_UNLOCK_QUICK(&_G_hookcbReboot.HOOKCB_slHook, iregInterLevel);
        break;
        
    case LW_OPTION_WATCHDOG_TIMER:                                      /*  ���Ź���ʱ������            */
        LW_SPIN_LOCK_QUICK(&_G_hookcbWatchDog.HOOKCB_slHook, &iregInterLevel);
        if (_G_hookcbWatchDog.HOOKCB_plineHookHeader == LW_NULL) {
            _K_hookKernel.HOOK_WatchDogTimer = _SysWatchDogHook;
        }
        _List_Line_Add_Ahead(pline, &_G_hookcbWatchDog.HOOKCB_plineHookHeader);
        LW_SPIN_UNLOCK_QUICK(&_G_hookcbWatchDog.HOOKCB_slHook, iregInterLevel);
        break;
        
    case LW_OPTION_OBJECT_CREATE_HOOK:                                  /*  �����ں˶�����            */
        LW_SPIN_LOCK_QUICK(&_G_hookcbObjectCreate.HOOKCB_slHook, &iregInterLevel);
        if (_G_hookcbObjectCreate.HOOKCB_plineHookHeader == LW_NULL) {
            _K_hookKernel.HOOK_ObjectCreate = _SysObjectCreateHook;
        }
        _List_Line_Add_Ahead(pline, &_G_hookcbObjectCreate.HOOKCB_plineHookHeader);
        LW_SPIN_UNLOCK_QUICK(&_G_hookcbObjectCreate.HOOKCB_slHook, iregInterLevel);
        break;
        
    case LW_OPTION_OBJECT_DELETE_HOOK:                                  /*  ɾ���ں˶�����            */
        LW_SPIN_LOCK_QUICK(&_G_hookcbObjectDelete.HOOKCB_slHook, &iregInterLevel);
        if (_G_hookcbObjectDelete.HOOKCB_plineHookHeader == LW_NULL) {
            _K_hookKernel.HOOK_ObjectDelete = _SysObjectDeleteHook;
        }
        _List_Line_Add_Ahead(pline, &_G_hookcbObjectDelete.HOOKCB_plineHookHeader);
        LW_SPIN_UNLOCK_QUICK(&_G_hookcbObjectDelete.HOOKCB_slHook, iregInterLevel);
        break;
    
    case LW_OPTION_FD_CREATE_HOOK:                                      /*  �ļ���������������          */
        LW_SPIN_LOCK_QUICK(&_G_hookcbFdCreate.HOOKCB_slHook, &iregInterLevel);
        if (_G_hookcbFdCreate.HOOKCB_plineHookHeader == LW_NULL) {
            _K_hookKernel.HOOK_FdCreate = _SysFdCreateHook;
        }
        _List_Line_Add_Ahead(pline, &_G_hookcbFdCreate.HOOKCB_plineHookHeader);
        LW_SPIN_UNLOCK_QUICK(&_G_hookcbFdCreate.HOOKCB_slHook, iregInterLevel);
        break;
    
    case LW_OPTION_FD_DELETE_HOOK:                                      /*  �ļ�������ɾ������          */
        LW_SPIN_LOCK_QUICK(&_G_hookcbFdDelete.HOOKCB_slHook, &iregInterLevel);
        if (_G_hookcbFdDelete.HOOKCB_plineHookHeader == LW_NULL) {
            _K_hookKernel.HOOK_FdDelete = _SysFdDeleteHook;
        }
        _List_Line_Add_Ahead(pline, &_G_hookcbFdDelete.HOOKCB_plineHookHeader);
        LW_SPIN_UNLOCK_QUICK(&_G_hookcbFdDelete.HOOKCB_slHook, iregInterLevel);
        break;
    
    case LW_OPTION_CPU_IDLE_ENTER:                                      /*  CPU �������ģʽ            */
        LW_SPIN_LOCK_QUICK(&_G_hookcbCpuIdleEnter.HOOKCB_slHook, &iregInterLevel);
        if (_G_hookcbCpuIdleEnter.HOOKCB_plineHookHeader == LW_NULL) {
            _K_hookKernel.HOOK_CpuIdleEnter = _SysCpuIdleEnterHook;
        }
        _List_Line_Add_Ahead(pline, &_G_hookcbCpuIdleEnter.HOOKCB_plineHookHeader);
        LW_SPIN_UNLOCK_QUICK(&_G_hookcbCpuIdleEnter.HOOKCB_slHook, iregInterLevel);
        break;
    
    case LW_OPTION_CPU_IDLE_EXIT:                                       /*  CPU �˳�����ģʽ            */
        LW_SPIN_LOCK_QUICK(&_G_hookcbCpuIdleExit.HOOKCB_slHook, &iregInterLevel);
        if (_G_hookcbCpuIdleExit.HOOKCB_plineHookHeader == LW_NULL) {
            _K_hookKernel.HOOK_CpuIdleExit = _SysCpuIdleExitHook;
        }
        _List_Line_Add_Ahead(pline, &_G_hookcbCpuIdleExit.HOOKCB_plineHookHeader);
        LW_SPIN_UNLOCK_QUICK(&_G_hookcbCpuIdleExit.HOOKCB_slHook, iregInterLevel);
        break;
    
    case LW_OPTION_CPU_INT_ENTER:                                       /*  CPU �����ж�(�쳣)ģʽ      */
        LW_SPIN_LOCK_QUICK(&_G_hookcbCpuIntEnter.HOOKCB_slHook, &iregInterLevel);
        if (_G_hookcbCpuIntEnter.HOOKCB_plineHookHeader == LW_NULL) {
            _K_hookKernel.HOOK_CpuIntEnter = _SysIntEnterHook;
        }
        _List_Line_Add_Ahead(pline, &_G_hookcbCpuIntEnter.HOOKCB_plineHookHeader);
        LW_SPIN_UNLOCK_QUICK(&_G_hookcbCpuIntEnter.HOOKCB_slHook, iregInterLevel);
        break;
    
    case LW_OPTION_CPU_INT_EXIT:                                        /*  CPU �˳��ж�(�쳣)ģʽ      */
        LW_SPIN_LOCK_QUICK(&_G_hookcbCpuIntExit.HOOKCB_slHook, &iregInterLevel);
        if (_G_hookcbCpuIntExit.HOOKCB_plineHookHeader == LW_NULL) {
            _K_hookKernel.HOOK_CpuIntExit = _SysIntExitHook;
        }
        _List_Line_Add_Ahead(pline, &_G_hookcbCpuIntExit.HOOKCB_plineHookHeader);
        LW_SPIN_UNLOCK_QUICK(&_G_hookcbCpuIntExit.HOOKCB_slHook, iregInterLevel);
        break;
        
    case LW_OPTION_VPROC_CREATE_HOOK:                                   /*  ���̽�������                */
        LW_SPIN_LOCK_QUICK(&_G_hookcbVpCreate.HOOKCB_slHook, &iregInterLevel);
        if (_G_hookcbVpCreate.HOOKCB_plineHookHeader == LW_NULL) {
            _K_hookKernel.HOOK_VpCreate = _SysVpCreateHook;
        }
        _List_Line_Add_Ahead(pline, &_G_hookcbVpCreate.HOOKCB_plineHookHeader);
        LW_SPIN_UNLOCK_QUICK(&_G_hookcbVpCreate.HOOKCB_slHook, iregInterLevel);
        break;
        
    case LW_OPTION_VPROC_DELETE_HOOK:                                   /*  ����ɾ������                */
        LW_SPIN_LOCK_QUICK(&_G_hookcbVpDelete.HOOKCB_slHook, &iregInterLevel);
        if (_G_hookcbVpDelete.HOOKCB_plineHookHeader == LW_NULL) {
            _K_hookKernel.HOOK_VpDelete = _SysVpDeleteHook;
        }
        _List_Line_Add_Ahead(pline, &_G_hookcbVpDelete.HOOKCB_plineHookHeader);
        LW_SPIN_UNLOCK_QUICK(&_G_hookcbVpDelete.HOOKCB_slHook, iregInterLevel);
        break;
    
    default:
        __SHEAP_FREE(pfuncnode);                                        /*  �ͷ��ڴ�                    */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "option invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_OPT_NULL);
        return  (ERROR_KERNEL_OPT_NULL);
    }
    
#if LW_CFG_MODULELOADER_EN > 0
    if (__PROC_GET_PID_CUR() && vprocFindProc((PVOID)hookfuncPtr)) {
        __resAddRawHook(&pfuncnode->FUNCNODE_resraw, (VOIDFUNCPTR)API_SystemHookDelete, 
                        (PVOID)pfuncnode->FUNCNODE_hookfuncPtr, (PVOID)ulOpt, 0, 0, 0, 0);
    } else {
        pfuncnode->FUNCNODE_resraw.RESRAW_bIsInstall = LW_FALSE;        /*  ����Ҫ���ղ���             */
    }
#else
    __resAddRawHook(&pfuncnode->FUNCNODE_resraw, (VOIDFUNCPTR)API_SystemHookDelete, 
                    (PVOID)pfuncnode->FUNCNODE_hookfuncPtr, (PVOID)ulOpt, 0, 0, 0, 0);
#endif
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_SystemHookDelete
** ��������: ɾ��һ��ϵͳ hook ���ܺ���
** �䡡��  : 
**           hookfuncPtr                   HOOK ���ܺ���
**           ulOpt                         HOOK ����
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
ULONG  API_SystemHookDelete (LW_HOOK_FUNC  hookfuncPtr, ULONG  ulOpt)
{
             INTREG                 iregInterLevel;
             PLW_HOOK_CB            phookcb;
             
             PLW_FUNC_NODE          pfuncnode;
    REGISTER PLW_LIST_LINE          plinePtr;
    
    if (LW_CPU_GET_CUR_NESTING()) {                                     /*  �������ж��е���            */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "called from ISR.\r\n");
        _ErrorHandle(ERROR_KERNEL_IN_ISR);
        return  (ERROR_KERNEL_IN_ISR);
    }
    
#if LW_CFG_ARG_CHK_EN > 0
    if (!hookfuncPtr) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "hookfuncPtr invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HOOK_NULL);
        return  (ERROR_KERNEL_HOOK_NULL);
    }
#endif

    _ErrorHandle(ERROR_NONE);
    
    switch (ulOpt) {
    
    case LW_OPTION_THREAD_CREATE_HOOK:                                  /*  �߳̽�������                */
        phookcb = &_G_hookcbCreate;
        break;
        
    case LW_OPTION_THREAD_DELETE_HOOK:                                  /*  �߳�ɾ������                */
        phookcb = &_G_hookcbDelete;
        break;
        
    case LW_OPTION_THREAD_SWAP_HOOK:                                    /*  �߳��л�����                */
        phookcb = &_G_hookcbSwap;
        break;
        
    case LW_OPTION_THREAD_TICK_HOOK:                                    /*  ϵͳʱ���жϹ���            */
        phookcb = &_G_hookcbTick;
        break;
        
    case LW_OPTION_THREAD_INIT_HOOK:                                    /*  �̳߳�ʼ������              */
        phookcb = &_G_hookcbInit;
        break;
        
    case LW_OPTION_THREAD_IDLE_HOOK:                                    /*  �����̹߳���                */
        phookcb = &_G_hookcbIdle;
        break;
        
    case LW_OPTION_KERNEL_INITBEGIN:                                    /*  �ں˳�ʼ����ʼ����          */
        phookcb = &_G_hookcbInitBegin;
        break;
        
    case LW_OPTION_KERNEL_INITEND:                                      /*  �ں˳�ʼ����������          */
        phookcb = &_G_hookcbInitEnd;
        break;
        
    case LW_OPTION_KERNEL_REBOOT:                                       /*  �ں���������                */
        phookcb = &_G_hookcbReboot;
        break;
        
    case LW_OPTION_WATCHDOG_TIMER:                                      /*  ���Ź���ʱ������            */
        phookcb = &_G_hookcbWatchDog;
        break;
        
    case LW_OPTION_OBJECT_CREATE_HOOK:                                  /*  �����ں˶�����            */
        phookcb = &_G_hookcbObjectCreate;
        break;
    
    case LW_OPTION_OBJECT_DELETE_HOOK:                                  /*  ɾ���ں˶�����            */
        phookcb = &_G_hookcbObjectDelete;
        break;
    
    case LW_OPTION_FD_CREATE_HOOK:                                      /*  �ļ���������������          */
        phookcb = &_G_hookcbFdCreate;
        break;
    
    case LW_OPTION_FD_DELETE_HOOK:                                      /*  �ļ�������ɾ������          */
        phookcb = &_G_hookcbFdDelete;
        break;
        
    case LW_OPTION_CPU_IDLE_ENTER:                                      /*  CPU �������ģʽ            */
        phookcb = &_G_hookcbCpuIdleEnter;
        break;
    
    case LW_OPTION_CPU_IDLE_EXIT:                                       /*  CPU �˳�����ģʽ            */
        phookcb = &_G_hookcbCpuIdleExit;
        break;
    
    case LW_OPTION_CPU_INT_ENTER:                                       /*  CPU �����ж�(�쳣)ģʽ      */
        phookcb = &_G_hookcbCpuIntEnter;
        break;
    
    case LW_OPTION_CPU_INT_EXIT:                                        /*  CPU �˳��ж�(�쳣)ģʽ      */
        phookcb = &_G_hookcbCpuIntExit;
        break;
        
    case LW_OPTION_VPROC_CREATE_HOOK:                                   /*  ���̽�������                */
        phookcb = &_G_hookcbVpCreate;
        break;
        
    case LW_OPTION_VPROC_DELETE_HOOK:                                   /*  ����ɾ������                */
        phookcb = &_G_hookcbVpDelete;
        break;
    
    default:
        _DebugHandle(__ERRORMESSAGE_LEVEL, "option invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_OPT_NULL);
        return  (ERROR_KERNEL_OPT_NULL);
    }
    
    LW_SPIN_LOCK(&phookcb->HOOKCB_slHook);
    __KERNEL_ENTER();
    for (plinePtr  = phookcb->HOOKCB_plineHookHeader;
         plinePtr != LW_NULL;
         plinePtr  = _list_line_get_next(plinePtr)) {                   /*  ��ʼ��ѯ                    */
         
        pfuncnode = (PLW_FUNC_NODE)plinePtr;
         
        if (pfuncnode->FUNCNODE_hookfuncPtr == hookfuncPtr) {
            
            iregInterLevel = KN_INT_DISABLE();                          /*  �ر��ж�                    */
            if (plinePtr == phookcb->HOOKCB_plineHookOp) {              /*  �Ƿ�Ϊ��ǰ������ָ��        */
                phookcb->HOOKCB_plineHookOp = _list_line_get_next(plinePtr);
            }
            _List_Line_Del(plinePtr, &phookcb->HOOKCB_plineHookHeader);
            KN_INT_ENABLE(iregInterLevel);                              /*  ���ж�                    */
             
            LW_SPIN_UNLOCK(&phookcb->HOOKCB_slHook);
            __KERNEL_EXIT();
             
            __resDelRawHook(&pfuncnode->FUNCNODE_resraw);
            
            __SHEAP_FREE(pfuncnode);
            return  (ERROR_NONE);
        }
    }
    LW_SPIN_UNLOCK(&phookcb->HOOKCB_slHook);
    __KERNEL_EXIT();

    return  (ERROR_SYSTEM_HOOK_NULL);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
