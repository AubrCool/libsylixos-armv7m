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
** ��   ��   ��: powerMShow.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2008 �� 03 �� 06 ��
**
** ��        ��: ��ʾ��ǰ���ڼ�����ϵͳ�����Ĺ�������Ϣ.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
/*********************************************************************************************************
  ����ü�֧��
*********************************************************************************************************/
#if (LW_CFG_FIO_LIB_EN > 0) && (LW_CFG_POWERM_EN > 0) && (LW_CFG_MAX_POWERM_NODES > 0)
/*********************************************************************************************************
  ȫ�ֱ���
*********************************************************************************************************/
extern LW_OBJECT_HANDLE     _G_ulPowerMLock;
extern LW_CLASS_WAKEUP      _G_wuPowerM;
/*********************************************************************************************************
  ȫ�ֱ���
*********************************************************************************************************/
static const CHAR   _G_cPowerMInfoHdr[] = "\n\
     NAME       COUNT   MAX IDLE    CB     PARAM\n\
-------------- -------- -------- -------- --------\n";
/*********************************************************************************************************
** ��������: API_PowerMShow
** ��������: ��ʾ��ǰ���ڼ�����ϵͳ�����Ĺ�������Ϣ
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
                                           
                                       (�������ж��е���)
*********************************************************************************************************/
LW_API  
VOID  API_PowerMShow (VOID)
{
    REGISTER PLW_CLASS_POWERM_NODE  p_pmnTempNode;
    REGISTER PLW_LIST_LINE          plinePowerMOp;
             ULONG                  ulCounter;
    
    printf("active power management node show >>\n");
    printf(_G_cPowerMInfoHdr);                                          /*  ��ӡ��ӭ��Ϣ                */
    
    __POWERM_LOCK();                                                    /*  ����                        */
        
    for (plinePowerMOp  = _G_wuPowerM.WU_plineHeader;
         plinePowerMOp != LW_NULL;
         plinePowerMOp  = _list_line_get_next(plinePowerMOp)) {
    
        PLW_CLASS_WAKEUP_NODE   pwun = _LIST_ENTRY(plinePowerMOp,
                                                   LW_CLASS_WAKEUP_NODE,
                                                   WUN_lineManage);
        p_pmnTempNode = _LIST_ENTRY(pwun,
                                    LW_CLASS_POWERM_NODE,
                                    PMN_wunTimer);

        if (p_pmnTempNode->PMN_bInQ) {
            _WakeupStatus(&_G_wuPowerM, &p_pmnTempNode->PMN_wunTimer, &ulCounter);
        } else {
            ulCounter = 0ul;
        }
                                    
        printf("%-14s %8lx %8lx %8lx %8lx\n",
                      p_pmnTempNode->PMN_cPowerMName,
                      ulCounter,
                      p_pmnTempNode->PMN_ulCounterSave,
                      (addr_t)p_pmnTempNode->PMN_pfuncCallback,
                      (addr_t)p_pmnTempNode->PMN_pvArg);
    }
    
    __POWERM_UNLOCK();                                                  /*  ����                        */

    printf("\n");
}
#endif                                                                  /*  LW_CFG_FIO_LIB_EN           */
                                                                        /*  LW_CFG_POWERM_EN            */
                                                                        /*  LW_CFG_MAX_POWERM_NODES     */
/*********************************************************************************************************
  END
*********************************************************************************************************/


