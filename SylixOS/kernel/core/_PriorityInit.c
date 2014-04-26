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
** ��   ��   ��: _PriorityInit.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2006 �� 12 �� 14 ��
**
** ��        ��: ����ϵͳ���ȼ����ƿ��ʼ�������⡣
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** ��������: _PriorityInit
** ��������: ��ʼ�����ȼ����ƿ�(���ӳ�ʼ�� _K_ptcbTCBIdTable )  һ����ʼ�� LW_CFG_LOWEST_PRIO + 1 ��Ԫ��
** �䡡��  : 
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _PriorityInit (VOID)
{
    REGISTER ULONG    ulI;
    
    for (ulI = 0; ulI < (LW_PRIO_LOWEST + 1); ulI++) {
        _K_pcbPriorityTable[ulI].PCB_pringReadyHeader     = LW_NULL;
        
        _K_pcbPriorityTable[ulI].PCB_ucPriority           = (UINT8)ulI;
        _K_pcbPriorityTable[ulI].PCB_usThreadCounter      = 0;
        _K_pcbPriorityTable[ulI].PCB_usThreadReadyCounter = 0;
        _K_pcbPriorityTable[ulI].PCB_usThreadCandCounter  = 0;
        
        _K_pcbPriorityTable[ulI].PCB_ucZ        = (UINT8)(ulI >> 6);
        _K_pcbPriorityTable[ulI].PCB_ucMaskBitZ = (UINT8)(1 << _K_pcbPriorityTable[ulI].PCB_ucZ);

        _K_pcbPriorityTable[ulI].PCB_ucY        = (UINT8)((ulI >> 3) & 0x07);
        _K_pcbPriorityTable[ulI].PCB_ucMaskBitY = (UINT8)(1 << _K_pcbPriorityTable[ulI].PCB_ucY);
        
        _K_pcbPriorityTable[ulI].PCB_ucX        = (UINT8)(ulI & 0x07);
        _K_pcbPriorityTable[ulI].PCB_ucMaskBitX = (UINT8)(1 << _K_pcbPriorityTable[ulI].PCB_ucX);
    }
    
    for (ulI = 0; ulI < LW_CFG_MAX_THREADS; ulI++) {
        _K_ptcbTCBIdTable[ulI] = LW_NULL;
    }
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
