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
** ��   ��   ��: InterVectorIsr.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2007 �� 02 �� 02 ��
**
** ��        ��: �����ж��ܷ���

** BUG
2007.06.06  �������Ƕ����Ϣ��¼���ܡ�
2010.08.03  ʹ�� interrupt vector spinlock ��Ϊ�����.
2011.03.31  ���� vector queue �����ж�����֧��.
2013.07.19  ����˼��жϴ����֧.
2013.08.28  �����ں��¼�������.
2014.04.21  �жϱ�������ڱ���������жϷ�����.
2014.05.09  ������жϼ������Ĵ���.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** ��������: API_InterVectorIsr
** ��������: �����ж��ܷ���
** �䡡��  : ulVector                      �ж������� (arch �㺯����Ҫ��֤�˲�����ȷ)
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : ���ﲢ�������ж�Ƕ��, ����Ҫ arch ����ֲ����֧��.
                                           API ����
*********************************************************************************************************/
LW_API
VOID  API_InterVectorIsr (ULONG  ulVector)
{
    INTREG              iregInterLevel;
    PLW_CLASS_CPU       pcpu;
    
    PLW_LIST_LINE       plineTemp;
    PLW_CLASS_INTDESC   pidesc;
    PLW_CLASS_INTACT    piaction;
    irqreturn_t         irqret;
           
    pcpu = LW_CPU_GET_CUR();                                            /*  �жϴ��������, ����ı� CPU*/
    
    __LW_CPU_INT_ENTER_HOOK(ulVector, pcpu->CPU_ulInterNesting);
    
    MONITOR_EVT_LONG2(MONITOR_EVENT_ID_INT, MONITOR_EVENT_INT_ENTER, 
                      ulVector, pcpu->CPU_ulInterNesting, LW_NULL);
    
#if LW_CFG_SMP_EN > 0
    if ((pcpu->CPU_ulIPIVector != __ARCH_ULONG_MAX) && 
        (pcpu->CPU_ulIPIVector == ulVector)) {                          /*  �˼��ж�                    */
        _SmpProcIpi(pcpu);
    }
#endif                                                                  /*  LW_CFG_SMP_EN               */

    pidesc = LW_IVEC_GET_IDESC(ulVector);
    
    LW_SPIN_LOCK(&pidesc->IDESC_slLock);                                /*  ��ס spinlock               */
    
    for (plineTemp  = pidesc->IDESC_plineAction;
         plineTemp != LW_NULL;
         plineTemp  = _list_line_get_next(plineTemp)) {
        
        piaction = _LIST_ENTRY(plineTemp, LW_CLASS_INTACT, IACT_plineManage);
        if (piaction->IACT_pfuncIsr) {
            irqret = piaction->IACT_pfuncIsr(piaction->IACT_pvArg, ulVector);
            if (LW_IRQ_RETVAL(irqret)) {                                /*  �ж��Ƿ��Ѿ�������          */
                piaction->IACT_iIntCnt[pcpu->CPU_ulCPUId]++;
                if (piaction->IACT_pfuncClear) {
                    piaction->IACT_pfuncClear(piaction->IACT_pvArg, ulVector);
                }
                break;
            }
        }
    }
    
#if LW_CFG_INTER_INFO > 0
    iregInterLevel = KN_INT_DISABLE();
    if (pcpu->CPU_ulInterNestingMax < pcpu->CPU_ulInterNesting) {
        pcpu->CPU_ulInterNestingMax = pcpu->CPU_ulInterNesting;
    }
    KN_INT_ENABLE(iregInterLevel);
#endif
    
    LW_SPIN_UNLOCK(&pidesc->IDESC_slLock);                              /*  ���� spinlock               */
    
    __LW_CPU_INT_EXIT_HOOK(ulVector, pcpu->CPU_ulInterNesting);
    
    MONITOR_EVT_LONG2(MONITOR_EVENT_ID_INT, MONITOR_EVENT_INT_EXIT, 
                      ulVector, pcpu->CPU_ulInterNesting, LW_NULL);
}
/*********************************************************************************************************
** ��������: API_InterVectorIpi
** ��������: ���ú˼��ж�����   
             BSP ��ϵͳ����ǰ���ô˺���, ÿ�� CPU ��Ҫ����, SylixOS ����ͬ�� CPU �˼��ж�������ͬ.
** �䡡��  : 
**           ulCPUId                       CPU ID
**           ulIPIVector                   �˼��ж�������
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
#if LW_CFG_SMP_EN > 0

LW_API
VOID  API_InterVectorIpi (ULONG  ulCPUId, ULONG  ulIPIVector)
{
    if (ulCPUId < LW_CFG_MAX_PROCESSORS) {
        LW_CPU_GET(ulCPUId)->CPU_ulIPIVector = ulIPIVector;
    }
}

#endif                                                                  /*  LW_CFG_SMP_EN               */
/*********************************************************************************************************
  END
*********************************************************************************************************/
