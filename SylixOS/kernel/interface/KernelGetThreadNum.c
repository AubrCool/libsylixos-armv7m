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
** ��   ��   ��: KernelGetThreadNum.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2007 �� 05 �� 11 ��
**
** ��        ��: �û����Ե������ API ��õ�ǰ�߳�����

** BUG
2008.05.18  ʹ�� __KERNEL_ENTER() ���� ThreadLock();
2008.05.31  ��Ϊ�ر��жϷ�ʽ.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** ��������: API_KernelGetThreadNum
** ��������: ��õ�ǰ�߳�����
** �䡡��  : 
** �䡡��  : ��ǰ�߳�����
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
UINT16  API_KernelGetThreadNum (VOID)
{
    REGISTER UINT16           usThreadNum;
    
    __KERNEL_ENTER();                                                   /*  �����ں�                    */
    usThreadNum = _K_usThreadCounter;
    __KERNEL_EXIT();                                                    /*  �˳��ں�                    */

    return  (usThreadNum);
}
/*********************************************************************************************************
** ��������: API_KernelGetThreadNumByPriority
** ��������: ����ں�ָ�����ȼ����߳�����
** �䡡��  : 
** �䡡��  : �ں�ָ�����ȼ����߳�����
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
UINT16  API_KernelGetThreadNumByPriority (UINT8  ucPriority)
{
             INTREG           iregInterLevel;
    REGISTER PLW_CLASS_PCB    ppcb;
    REGISTER UINT16           usThreadNum;

#if LW_CFG_ARG_CHK_EN > 0
    if (_PriorityCheck(ucPriority)) {                                   /*  ���ȼ�����                  */
        _ErrorHandle(ERROR_THREAD_PRIORITY_WRONG);
        return  (0);
    }
#endif
    
    ppcb = _GetPcbByPrio(ucPriority);                                   /*  ������ȼ����ƿ�            */
    
    iregInterLevel = __KERNEL_ENTER_IRQ();                              /*  �����ں�ͬʱ�ر��ж�        */
    usThreadNum = ppcb->PCB_usThreadCounter;                            /*  ��ø����ȼ��߳�����        */
    __KERNEL_EXIT_IRQ(iregInterLevel);                                  /*  �˳��ں�ͬʱ���ж�        */

    return  (usThreadNum);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
