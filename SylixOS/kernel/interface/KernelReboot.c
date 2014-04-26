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
** ��   ��   ��: KernelReboot.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2008 �� 03 �� 02 ��
**
** ��        ��: �ں�������������.

** BUG:
2009.05.05  ��Ҫ���� CACHE ����Ӧ bootloader �Ĺ���.
2009.05.31  ������ except �߳�ִ��, ������Ҫ��ס�ں�.
2009.06.24  API_KernelRebootEx() ������ַ�еľֲ��������ܻᱻһЩ���ĵ� BSP CACHE ������д, ��������ȫ��
            ��������.
2011.03.08  ���� c++ ����ʱж�غ���.
2012.03.26  ���� __LOGMESSAGE_LEVEL ��Ϣ��ӡ.
2012.11.09  ����ϵͳ�����������Ͷ���, ���������ش���ʱ�����ûص���������������.
2013.12.03  ���ϵͳ����ʱ, ��Ҫ�� LW_NCPUS - 1 �� idle �߳����ȼ��ᵽ���, ��ռ�����˵� CPU Ȼ��ʵ����
            ��Ϊ����ϵͳִ�и�λ�����Ĳ���.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
  ϵͳ��������
*********************************************************************************************************/
#if LW_CFG_SIGNAL_EN > 0
INT  _excJobAdd(VOIDFUNCPTR  pfunc, 
                PVOID        pvArg0,
                PVOID        pvArg1,
                PVOID        pvArg2,
                PVOID        pvArg3,
                PVOID        pvArg4,
                PVOID        pvArg5);
#endif                                                                  /*  LW_CFG_SIGNAL_EN > 0        */
VOID _cppRtUninit(VOID);
/*********************************************************************************************************
  ȫ�ֱ���
*********************************************************************************************************/
static addr_t   _K_ulRebootStartAddress;                                /*  ����������ַ                */
/*********************************************************************************************************
** ��������: __makeOtherIdle
** ��������: ������ CPU ����Ϊ idle ����. (LW_NCPUS ������� 1)
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
#if LW_CFG_SMP_EN > 0

static VOID  __makeOtherIdle (VOID)
{
#define LW_CPU_TCB_ID(pcpu)  (pcpu->CPU_ptcbTCBCur->TCB_ulId)

    ULONG           i;
    ULONG           ulCPUCurId;
    BOOL            bSetIdlePrio = LW_FALSE;
    BOOL            bOk = LW_FALSE;
    PLW_CLASS_CPU   pcpu;
    
    do {
        __KERNEL_ENTER();                                               /*  �����ں�                    */
        ulCPUCurId = LW_CPU_GET_CUR_ID();
        
        if (bSetIdlePrio == LW_FALSE) {
            for (i = 0; i < LW_NCPUS; i++) {
                pcpu = LW_CPU_GET(i);
                if (LW_CPU_IS_ACTIVE(pcpu) && 
                    (pcpu->CPU_ulCPUId != ulCPUCurId)) {
                    _SchedSetPrio(_K_ptcbIdle[i], LW_PRIO_HIGHEST);     /*  idle ���ȼ�������ȼ�       */
                }
            }
            bSetIdlePrio = LW_TRUE;
        
        } else {
            for (i = 0; i < LW_NCPUS; i++) {
                pcpu = LW_CPU_GET(i);
                if (LW_CPU_IS_ACTIVE(pcpu) && 
                    (pcpu->CPU_ulCPUId != ulCPUCurId)) {
                    if (_ObjectGetIndex(LW_CPU_TCB_ID(pcpu)) >= LW_NCPUS) {  
                        break;                                          /*  �Ƿ�Ϊ idle �߳�            */
                    }
                }
            }
            if (i >= LW_NCPUS) {                                        /*  ���� CPU �Ѿ���Ϊ idle �߳� */
                bOk = LW_TRUE;
            }
        }
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
        
        if (bOk) {                                                      /*  һ��׼������                */
            break;
        }
        
        LW_SPINLOCK_DELAY();
    } while (1);
}

#endif                                                                  /*  LW_CFG_SMP_EN > 0           */
/*********************************************************************************************************
** ��������: API_KernelReboot
** ��������: �ں�������������
** �䡡��  : iRebootType        �������� 
                                LW_REBOOT_FORCE
                                LW_REBOOT_WARM
                                LW_REBOOT_COLD
                                LW_REBOOT_SHUTDOWN
                                LW_REBOOT_POWEROFF
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
VOID   API_KernelReboot (INT  iRebootType)
{
    API_KernelRebootEx(iRebootType, 0ull);
}
/*********************************************************************************************************
** ��������: API_KernelReboot
** ��������: �ں�������������
** �䡡��  : iRebootType        �������� 
                                LW_REBOOT_FORCE
                                LW_REBOOT_WARM
                                LW_REBOOT_COLD
                                LW_REBOOT_SHUTDOWN
                                LW_REBOOT_POWEROFF
**           ulStartAddress     ������ַ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
VOID   API_KernelRebootEx (INT  iRebootType, addr_t  ulStartAddress)
{
    INTREG  iregInterLevel;
    ULONG   ulI;

#if LW_CFG_SIGNAL_EN > 0
    if (LW_CPU_GET_CUR_NESTING() || (API_ThreadIdSelf() != API_KernelGetExc())) {
        _excJobAdd(API_KernelRebootEx, (PVOID)iRebootType, (PVOID)ulStartAddress, 0, 0, 0, 0);
        return;
    }
#endif                                                                  /*  LW_CFG_SIGNAL_EN > 0        */

    _DebugHandle(__LOGMESSAGE_LEVEL, "kernel rebooting...\r\n");
    
    _K_ulRebootStartAddress = ulStartAddress;                           /*  ��¼�ֲ�����, ��ֹ XXX      */
    
#if LW_CFG_SMP_EN > 0
    if (LW_NCPUS > 1) {
        __makeOtherIdle();                                              /*  ������ CPU ����Ϊ idle ģʽ */
    }
#endif                                                                  /*  LW_CFG_SMP_EN               */
    
    if (iRebootType != LW_REBOOT_FORCE) {
        __LW_KERNEL_REBOOT_HOOK(iRebootType);                           /*  ���ûص�����                */
        _cppRtUninit();                                                 /*  ж�� C++ ����ʱ             */
    }
    
    for (ulI = 0; ulI < LW_CFG_MAX_INTER_SRC; ulI++) {
        API_InterVectorDisable(ulI);                                    /*  �ر������ж�                */
    }
    
    iregInterLevel = __KERNEL_ENTER_IRQ();                              /*  �����ں�ͬʱ�ر��ж�        */

#if LW_CFG_CACHE_EN > 0
    API_CacheDisable(DATA_CACHE);                                       /*  ���� CACHE                  */
    API_CacheDisable(INSTRUCTION_CACHE);
#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */

#if LW_CFG_VMM_EN > 0
    __VMM_MMU_DISABLE();                                                /*  �ر� MMU                    */
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */

    archReboot(iRebootType, _K_ulRebootStartAddress);                   /*  ������ϵ�ܹ���������        */
    
    /*
     *  ���������в�������, CPU �ͻ��Զ���λ
     */
    __KERNEL_EXIT_IRQ(iregInterLevel);                                  /*  �˳��ں�ͬʱ���ж�        */
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
