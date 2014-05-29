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
** ��   ��   ��: ioPowerM.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2009 �� 08 �� 31 ��
**
** ��        ��: ϵͳ IO �豸��Դ�������.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
/*********************************************************************************************************
  ����ü�֧��
*********************************************************************************************************/
#if LW_CFG_DEVICE_EN > 0
#if (LW_CFG_POWERM_EN > 0) && (LW_CFG_MAX_POWERM_NODES > 0)
/*********************************************************************************************************
** ��������: _IosDevPowerMCallback
** ��������: �豸��Դ����ڵ�ص�����
** �䡡��  : pdevpm                       �豸��Դ����ڵ�
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  _IosDevPowerMCallback (PLW_DEV_PM      pdevpm)
{
    if (pdevpm) {
        if (pdevpm->DEVPM_pfuncPowerOff) {
            pdevpm->DEVPM_pfuncPowerOff(pdevpm->DEVPM_pvArgPowerOff);   /*  ���ý��ܷ���                */
        }
    }
}
/*********************************************************************************************************
** ��������: API_IosDevPowerMAdd
** ��������: ��ϵͳ��һ���豸���һ����Դ����ڵ�
** �䡡��  : pcName                       �豸��
**           ulMaxIdleTime                �豸�����ʱ�� (ticks) (���һ�β�����, ������ô��ʱ��, ϵͳ
**                                        �ͻὫ���豸����͹���ģʽ)
**           pfuncPowerOff                �豸����͹���ģʽ�ص�
**           pvArgPowerOff                �͹���ģʽ�ص�����
**           pfuncPowerSignal             �����豸���빤��״̬ (ע��: ÿһ�� IO �������ᱻ����)
**           pvArgPowerSignal             �����豸���빤��״̬����
**           pfuncRemove                  ϵͳ�Ƴ����豸ʱ���õĻص�
**           pvArgRemove                  ϵͳ�Ƴ����豸�ص�����
**           ppvPowerM                    ��Դ�������ƾ��(����)
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  API_IosDevPowerMAdd (CPCHAR        pcName,
                          ULONG         ulMaxIdleTime,
                          FUNCPTR       pfuncPowerOff,
                          PVOID         pvArgPowerOff,
                          FUNCPTR       pfuncPowerSignal,
                          PVOID         pvArgPowerSignal,
                          FUNCPTR       pfuncRemove,
                          PVOID         pvArgRemove,
                          PVOID        *ppvPowerM)
{
    REGISTER PLW_DEV_HDR     pdevhdr;
             PCHAR           pcTail;
             PLW_DEV_PM      pdevpm;
             CHAR            cPowerMName[LW_CFG_OBJECT_NAME_SIZE];
    
    pdevhdr = iosDevFind(pcName, &pcTail);
    if (pdevhdr) {
        if (pcTail && (pcTail[0] == PX_EOS)) {
            pdevpm = (PLW_DEV_PM)__SHEAP_ALLOC(sizeof(LW_DEV_PM));
            if (pdevpm == LW_NULL) {
                _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);
                return  (PX_ERROR);
            }
            
            snprintf(cPowerMName, LW_CFG_OBJECT_NAME_SIZE, 
                     "%s_powerm", pcName);                              /*  ȷ���ڵ�����                */
            pdevpm->DEVPM_ulPowerM = API_PowerMCreate(cPowerMName, 
                                        LW_OPTION_OBJECT_GLOBAL,
                                        LW_NULL);                       /*  ������Դ����ڵ�            */
            if (pdevpm->DEVPM_ulPowerM == LW_OBJECT_HANDLE_INVALID) {
                __SHEAP_FREE(pdevpm);
                return  (PX_ERROR);
            }
            
            pdevpm->DEVPM_pfuncPowerOff    = pfuncPowerOff;
            pdevpm->DEVPM_pvArgPowerOff    = pvArgPowerOff;
            pdevpm->DEVPM_pfuncPowerSignal = pfuncPowerSignal;
            pdevpm->DEVPM_pvArgPowerSignal = pvArgPowerSignal;
            pdevpm->DEVPM_pfuncRemove      = pfuncRemove;
            pdevpm->DEVPM_pvArgRemove      = pvArgRemove;
            
            _IosLock();                                                 /*  ���� IO �ٽ���              */
            _List_Line_Add_Ahead(&pdevpm->DEVPM_lineManage,
                                 &pdevhdr->DEVHDR_plinePowerMHeader);
            _IosUnlock();                                               /*  �˳� IO �ٽ���              */
            
            API_PowerMStart(pdevpm->DEVPM_ulPowerM, ulMaxIdleTime, 
                            _IosDevPowerMCallback, (PVOID)pdevpm);      /*  ������Դ����ʱ��          */
            
            if (ppvPowerM) {
                *ppvPowerM = (PVOID)pdevpm;                             /*  ������ƾ��                */
            }
            
            return  (ERROR_NONE);
        }
    }
    
    _ErrorHandle(ERROR_IOS_DEVICE_NOT_FOUND);
    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: API_IosDevPowerMDeleteAll
** ��������: ��ϵͳ��һ���豸ɾ�����е�Դ����ڵ�
** �䡡��  : pcName                       �豸��
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  API_IosDevPowerMDeleteAll (CPCHAR  pcName)
{
    REGISTER PLW_DEV_HDR     pdevhdr;
             PCHAR           pcTail;
             PLW_DEV_PM      pdevpm;
    
    pdevhdr = iosDevFind(pcName, &pcTail);
    if (pdevhdr) {
        if (pcTail && (pcTail[0] == PX_EOS)) {
            
            _IosLock();                                                 /*  ���� IO �ٽ���              */
            while (pdevhdr->DEVHDR_plinePowerMHeader) {
                pdevpm = _LIST_ENTRY(pdevhdr->DEVHDR_plinePowerMHeader, 
                                     LW_DEV_PM, 
                                     DEVPM_lineManage);
                _List_Line_Del(&pdevpm->DEVPM_lineManage,
                               &pdevhdr->DEVHDR_plinePowerMHeader);     /*  ��������ɾ��                */
                
                _IosUnlock();                                           /*  �˳� IO �ٽ���              */
                API_PowerMDelete(&pdevpm->DEVPM_ulPowerM);              /*  ɾ����Դ����ڵ�            */
                if (pdevpm->DEVPM_pfuncRemove) {                        /*  �����Ƴ�����                */
                    pdevpm->DEVPM_pfuncRemove(pdevpm->DEVPM_pvArgRemove);
                }
                __SHEAP_FREE(pdevpm);                                   /*  �ͷŽڵ��ڴ�                */
                _IosLock();                                             /*  ���� IO �ٽ���              */
            }
            _IosUnlock();                                               /*  �˳� IO �ٽ���              */
            
            return  (ERROR_NONE);
        }
    }
    
    _ErrorHandle(ERROR_IOS_DEVICE_NOT_FOUND);
    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: API_IosDevPowerMMaxIdleTimeSet
** ��������: �����豸��Դ����ڵ����������
** �䡡��  : pvPowerM                     ��Դ����ڵ�
**           ulMaxIdleTime                �豸�����ʱ�� (ticks) (���һ�β�����, ������ô��ʱ��, ϵͳ
**                                        �ͻὫ���豸����͹���ģʽ)
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  API_IosDevPowerMMaxIdleTimeSet (PVOID  pvPowerM, ULONG  ulMaxIdleTime)
{
    PLW_DEV_PM      pdevpm = (PLW_DEV_PM)pvPowerM;

    if (pdevpm) {
        API_PowerMCancel(pdevpm->DEVPM_ulPowerM);
        API_PowerMStart(pdevpm->DEVPM_ulPowerM, ulMaxIdleTime, 
                        _IosDevPowerMCallback, (PVOID)pdevpm);          /*  ����������Դ����ʱ��      */
        return  (ERROR_NONE);
    }
    
    _ErrorHandle(EINVAL);
    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: API_IosDevPowerMMaxIdleTimeGet
** ��������: ��ȡ�豸��Դ����ڵ����������
** �䡡��  : pvPowerM                     ��Դ����ڵ�
**           pulMaxIdleTime               �豸�����ʱ�� (ticks) (���һ�β�����, ������ô��ʱ��, ϵͳ
**                                        �ͻὫ���豸����͹���ģʽ)
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  API_IosDevPowerMMaxIdleTimeGet (PVOID  pvPowerM, ULONG  *pulMaxIdleTime)
{
    PLW_DEV_PM      pdevpm = (PLW_DEV_PM)pvPowerM;
    
    if (pdevpm && pulMaxIdleTime) {
        API_PowerMStatus(pdevpm->DEVPM_ulPowerM, LW_NULL,
                         pulMaxIdleTime,
                         LW_NULL, LW_NULL);
        return  (ERROR_NONE);
    }
    
    _ErrorHandle(EINVAL);
    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: API_IosDevPowerMCancel
** ��������: ֹͣ�豸��Դ����ڵ�
** �䡡��  : pvPowerM                     ��Դ����ڵ�
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
INT  API_IosDevPowerMCancel (PVOID  pvPowerM)
{
    PLW_DEV_PM      pdevpm = (PLW_DEV_PM)pvPowerM;

    if (pdevpm) {
        API_PowerMCancel(pdevpm->DEVPM_ulPowerM);
        return  (ERROR_NONE);
    }
    
    _ErrorHandle(EINVAL);
    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: API_IosDevPowerMSignal
** ��������: �����豸��Դ����ڵ�(��ʾ�豸��������, ��λ��Դ����ʱ��)
** �䡡��  : pvPowerM                     ��Դ����ڵ�
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
VOID  API_IosDevPowerMSignal (PVOID  pvPowerM)
{
    PLW_DEV_PM      pdevpm = (PLW_DEV_PM)pvPowerM;
    
    if (pdevpm) {
        if (pdevpm->DEVPM_ulPowerM) {
            API_PowerMSignalFast(pdevpm->DEVPM_ulPowerM);
        }
    }
}
#endif                                                                  /*  LW_CFG_POWERM_EN            */
                                                                        /*  LW_CFG_MAX_POWERM_NODES     */
#endif                                                                  /*  LW_CFG_DEVICE_EN            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
