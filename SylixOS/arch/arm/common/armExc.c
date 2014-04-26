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
** ��   ��   ��: armExc.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2013 �� 12 �� 09 ��
**
** ��        ��: ARM ��ϵ�����쳣����.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
#include "../mm/mmu/armMmuCommon.h"
/*********************************************************************************************************
** ��������: archIntHandle
** ��������: bspIntHandle ��Ҫ���ô˺��������ж�
** �䡡��  : ulVector         �ж�����
**           bPreemptive      �ж��Ƿ����ռ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : �����Ӧ vector �ж�����Ƕ��, ����Ҫ�ڶ�Ӧ���жϷ��������ٴ�ʹ�ܶ�Ӧ�ж� vector.
*********************************************************************************************************/
VOID  archIntHandle (ULONG  ulVector, BOOL  bPreemptive)
{
    if (_Inter_Vector_Invalid(ulVector)) {
        return;                                                         /*  �����Ų���ȷ                */
    }

    if (LW_IVEC_GET_FLAG(ulVector) & LW_IRQ_FLAG_PREEMPTIVE) {
        bPreemptive = LW_TRUE;
    }

    if (bPreemptive) {
        __ARCH_INT_VECTOR_DISABLE(ulVector);                            /*  ���δ��ж�Դ                */
        KN_INT_ENABLE_FORCE();                                          /*  �����ж�                    */
    }

    API_InterVectorIsr(ulVector);
    
    if (bPreemptive) {
        KN_INT_DISABLE();                                               /*  �����ж�                    */
    }
}
/*********************************************************************************************************
** ��������: archAbtHandle
** ��������: ϵͳ���� data abort ���� prefetch_abort �쳣ʱ����ô˺���
** �䡡��  : ulAbortAddr   ���ַ����쳣���ڴ��ַ.
**           ulAbortType   �쳣����
**           ptcbCur       ��ǰ�߳̿��ƿ�
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  archAbtHandle (UINT32  uiRetAddr, UINT32  uiArmExcType, PLW_CLASS_TCB   ptcbCur)
{
#define ARM_EXC_TYPE_ABT    8
#define ARM_EXC_TYPE_PRE    4

    addr_t ulAbortAddr;
    ULONG  ulAbortType;

    if (ptcbCur == LW_NULL) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "abort exception ptcbCur == NULL.\r\n");
        API_KernelReboot(LW_REBOOT_FORCE);                              /*  ֱ��������������ϵͳ        */
        return;
    }
    
    if (uiArmExcType == ARM_EXC_TYPE_ABT) {
        ulAbortAddr = armGetAbtAddr();
        ulAbortType = armGetAbtType();
    
    } else {
        ulAbortAddr = armGetPreAddr(uiRetAddr);
        ulAbortType = armGetPreType();
    }

    API_VmmAbortIsr(ulAbortAddr, ulAbortType, ptcbCur);
}
/*********************************************************************************************************
** ��������: archUndHandle
** ��������: archUndEntry ��Ҫ���ô˺�������δ����ָ��
** �䡡��  : ulAddr           ��Ӧ�ĵ�ַ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  archUndHandle (addr_t  ulAddr)
{
    PLW_CLASS_TCB   ptcbCur;
    
    if (LW_CPU_GET_CUR_NESTING()) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "undefined instruction occur in ISR mode.\r\n");
        return;
    }
    
#if LW_CFG_CPU_FPU_EN > 0
    if (archFpuUndHandle() == ERROR_NONE) {                             /*  �Ƚ��� FPU ָ��̽��         */
        return;
    }
#endif                                                                  /*  LW_CFG_CPU_FPU_EN > 0       */

    LW_TCB_GET_CUR_SAFE(ptcbCur);
    
    API_VmmAbortIsr(ulAddr, LW_VMM_ABORT_TYPE_UNDEF, ptcbCur);
}
/*********************************************************************************************************
** ��������: archSwiHandle
** ��������: archSwiEntry ��Ҫ���ô˺����������ж�
** �䡡��  : uiSwiNo       ���жϺ�
**           puiRegs       �Ĵ�����ָ��, ǰ 14 ��Ϊ R0-R12 LR �������Ϊ���� 4 ��������ѹջ��.
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : ���´���Ϊ��������, SylixOS Ŀǰδʹ��.
*********************************************************************************************************/
VOID  archSwiHandle (UINT32  uiSwiNo, UINT32  *puiRegs)
{
#ifdef __SYLIXOS_DEBUG
    UINT32  uiArg[10];
    
    uiArg[0] = puiRegs[0];                                              /*  ǰ�ĸ�����ʹ�� R0-R4 ����   */
    uiArg[1] = puiRegs[1];
    uiArg[2] = puiRegs[2];
    uiArg[3] = puiRegs[3];
    
    uiArg[4] = puiRegs[0 + 14];                                         /*  ����Ĳ���Ϊ��ջ����        */
    uiArg[5] = puiRegs[1 + 14];
    uiArg[6] = puiRegs[2 + 14];
    uiArg[7] = puiRegs[3 + 14];
    uiArg[8] = puiRegs[4 + 14];
    uiArg[9] = puiRegs[5 + 14];
#endif
    
    (VOID)uiSwiNo;
    puiRegs[0] = 0x0;                                                   /*  R0 Ϊ����ֵ                 */
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
