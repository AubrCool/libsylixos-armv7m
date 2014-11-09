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
** ��   ��   ��: sdhci.h
**
** ��   ��   ��: Zeng.Bo (����)
**
** �ļ���������: 2011 �� 01 �� 17 ��
**
** ��        ��: sd��׼������������Դ�ļ�(SD Host Controller Simplified Specification Version 2.00).

** BUG:
2011.03.02  �������ش���ģʽ�鿴\���ú���.����̬�ı䴫��ģʽ(�����ϲ������豸�������).
2011.04.07  ���� SDMA ���书��.
2011.04.07  ���ǵ� SD �������ڲ�ͬƽ̨����Ĵ��������� IO �ռ�,Ҳ�������ڴ�ռ�,���Զ�д�Ĵ�����
            6����������Ϊ�ⲿ����,�ɾ���ƽ̨������ʵ��.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
/*********************************************************************************************************
  ����ü�֧��
*********************************************************************************************************/
#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_SDCARD_EN > 0)
#include "sdhci.h"
/*********************************************************************************************************
  �ڲ���
*********************************************************************************************************/
#define __SDHCI_HOST_DEVNUM_MAX   1                                     /*  һ��������֧�ֵĲ�����      */
#define __SDHCI_MAX_BASE_CLK      102400000                             /*  ������������ʱ��102.4Mhz  */
#define __SDHCI_MAX_CLK           48000000                              /*  ���ʱ��25Mhz               */
#define __SDHCI_NOR_CLK           100000                                /*  һ��ʱ��400Khz              */

#define __SDHCI_CMD_TOUT_SEC      2                                     /*  ���ʱ����ʱ��(����ֵ)    */
#define __SDHCI_READ_TOUT_SEC     2                                     /*  ����ʱ����ʱ��(����ֵ)      */
#define __SDHCI_WRITE_TOUT_SEC    2                                     /*  д��ʱ����ʱ��(����ֵ)      */
#define __SDHCI_SDMARW_TOUT_SEC   2                                     /*  SDMA��ʱ����ʱ��(����ֵ)    */

#define __SDHCI_CMD_RETRY         0x40000000                            /*  ���ʱ����                */
#define __SDHCI_READ_RETRY        0x40000000                            /*  ����ʱ����                  */
#define __SDHCI_WRITE_RETRY       0x40000000                            /*  д��ʱ����                  */
#define __SDHCI_SDMARW_RETRY      50                                    /*  SDMA��д��ʱ����            */

#define __SDHCI_DMA_BOUND_LEN     (2 << (12 + __SDHCI_DMA_BOUND_NBITS))
#define __SDHCI_DMA_BOUND_NBITS   7                                     /*  ϵͳ�����ڴ�߽�λָʾ      */
                                                                        /*  ��ǰ�趨ֵΪ 7 (���):      */
                                                                        /*  2 << (12 + 7) = 512k        */
                                                                        /*  �����ݰ���ʱ����512K�߽�,�� */
                                                                        /*  Ҫ����DMA��ַ               */
#define __SDHCI_HOST_NAME(phs)    \
        (phs->SDHCIHS_psdadapter->SDADAPTER_busadapter.BUSADAPTER_cName)
/*********************************************************************************************************
  ��׼���������������ṹ
*********************************************************************************************************/
typedef struct __sdhci_capab {
    UINT32      SDHCICAP_uiBaseClkFreq;                                 /*  ��ʱ��Ƶ��                  */
    UINT32      SDHCICAP_uiMaxBlkSize;                                  /*  ֧�ֵ����鳤��            */
    UINT32      SDHCICAP_uiVoltage;                                     /*  ��ѹ֧�����                */

    BOOL        SDHCICAP_bCanSdma;                                      /*  �Ƿ�֧��SDMA                */
    BOOL        SDHCICAP_bCanAdma;                                      /*  �Ƿ�֧��ADMA                */
    BOOL        SDHCICAP_bCanHighSpeed;                                 /*  �Ƿ�֧�ָ��ٴ���            */
    BOOL        SDHCICAP_bCanSusRes;                                    /*  �Ƿ�֧�ֹ���\�ָ�����       */
}__SDHCI_CAPAB, *__PSDHCI_CAPAB;
/*********************************************************************************************************
  ��׼������HOST�ṹ
*********************************************************************************************************/
typedef struct __sdhci_host {
    LW_SD_FUNCS         SDHCIHS_sdfunc;                                 /*  ��Ӧ����������              */
    PLW_SD_ADAPTER      SDHCIHS_psdadapter;                             /*  ��Ӧ������������            */
    LW_SDHCI_HOST_ATTR  SDHCIHS_hostattr;                               /*  ��������                    */
    __SDHCI_CAPAB       SDHCIHS_sdhcicap;                               /*  ���ع���                    */
    atomic_t            SDHCIHS_atomicDevCnt;                           /*  �豸����                    */

    UINT32              SDHCIHS_ucTransferMod;                          /*  ����ʹ�õĴ���ģʽ          */

#if LW_CFG_VMM_EN
    UINT8              *SDHCIHS_pucDmaBuf;                              /*  ʹ��DMA���俪��cacheʱ��Ҫ  */
#endif

    INT               (*SDHCIHS_pfuncMasterXfer)                        /*  ���ص�ǰʹ�õĴ��亯��      */
                      (
                        struct __sdhci_host *psdhcihost,
                        PLW_SD_DEVICE        psddev,
                        PLW_SD_MESSAGE       psdmsg,
                        INT                  iNum
                      );
} __SDHCI_HOST, *__PSDHCI_HOST;
/*********************************************************************************************************
  ��׼�������豸�ڲ��ṹ
*********************************************************************************************************/
typedef struct __sdhci_dev {
    PLW_SD_DEVICE       SDHCIDEV_psddevice;                             /*  ��Ӧ��SD�豸                */
    __PSDHCI_HOST       SDHCIDEV_psdhcihost;                            /*  ��Ӧ��HOST                  */
    CHAR                SDHCIDEV_pcDevName[LW_CFG_OBJECT_NAME_SIZE];    /*  �豸����(���ӦSD�豸��ͬ)  */
} __SDHCI_DEVICE, *__PSDHCI_DEVICE;
/*********************************************************************************************************
  ˽�к�������
*********************************************************************************************************/
static VOID __sdhciHostCapDecode(PLW_SDHCI_HOST_ATTR psdhcihostattr, __PSDHCI_CAPAB psdhcicap);
/*********************************************************************************************************
  CALLBACK FOR SD BUS LAYER
*********************************************************************************************************/
static INT __sdhciTransfer(PLW_SD_ADAPTER      psdadapter,
                           PLW_SD_DEVICE       psddev,
                           PLW_SD_MESSAGE      psdmsg,
                           INT                 iNum);
static INT __sdhciIoCtl(PLW_SD_ADAPTER  psdadapter,
                        INT             iCmd,
                        LONG            lArg);
/*********************************************************************************************************
  FOR I\O CONTROL PRIVATE
*********************************************************************************************************/
static INT __sdhciClockSet(__PSDHCI_HOST     psdhcihost, UINT32 uiSetClk);
static INT __sdhciBusWidthSet(__PSDHCI_HOST  psdhcihost, UINT32 uiBusWidth);
static INT __sdhciPowerOn(__PSDHCI_HOST      psdhcihost);
static INT __sdhciPowerOff(__PSDHCI_HOST     psdhcihost);
static INT __sdhciPowerSetVol(__PSDHCI_HOST  psdhcihost,
                              UINT8          ucVol,
                              BOOL           uiVol);
/*********************************************************************************************************
  FOR TRANSFER PRIVATE
*********************************************************************************************************/
static INT __sdhciTransferNorm(__PSDHCI_HOST       psdhcihost,
                               PLW_SD_DEVICE       psddev,
                               PLW_SD_MESSAGE      psdmsg,
                               INT                 iNum);
static INT __sdhciTransferSdma(__PSDHCI_HOST       psdhcihost,
                               PLW_SD_DEVICE       psddev,
                               PLW_SD_MESSAGE      psdmsg,
                               INT                 iNum);
static INT __sdhciTransferAdma(__PSDHCI_HOST       psdhcihost,
                               PLW_SD_DEVICE       psddev,
                               PLW_SD_MESSAGE      psdmsg,
                               INT                 iNum);

static INT  __sdhciSendCmd(__PSDHCI_HOST   psdhcihost,
                           PLW_SD_COMMAND  psdcmd,
                           PLW_SD_DATA     psddat);

static VOID __sdhciTransferModSet(__PSDHCI_HOST   psdhcihost, PLW_SD_DATA    psddat);
static VOID __sdhciDataPrepareNorm(__PSDHCI_HOST  psdhcihost, PLW_SD_DATA    psddat);
static VOID __sdhciDataPrepareSdma(__PSDHCI_HOST  psdhcihost, PLW_SD_MESSAGE psdmsg);
static VOID __sdhciDataPrepareAdma(__PSDHCI_HOST  psdhcihost, PLW_SD_DATA    psddat);

static INT __sdhciDataReadNorm(__PSDHCI_HOST  psdhcihost,
                               UINT32         uiBlkSize,
                               UINT32         uiBlkNum,
                               PUCHAR         pucRdBuff);
static INT __sdhciDataWriteNorm(__PSDHCI_HOST  psdhcihost,
                                UINT32         uiBlkSize,
                                UINT32         uiBlkNum,
                                PUCHAR         pucWrtBuff);

static INT __sdhciDataFinishSdma(__PSDHCI_HOST   psdhcihost, PLW_SD_MESSAGE psdmsg);

static VOID __sdhciTransferIntSet(__PSDHCI_HOST  psdhcihost);
static VOID __sdhciIntDisAndEn(__PSDHCI_HOST     psdhcihost,
                               UINT32            uiDisMask,
                               UINT32            uiEnMask);
/*********************************************************************************************************
  FOR DEBUG
*********************************************************************************************************/
#ifdef __SYLIXOS_DEBUG
static VOID __sdhciPreStaShow(PLW_SDHCI_HOST_ATTR psdhcihostattr);
static VOID __sdhciIntStaShow(PLW_SDHCI_HOST_ATTR psdhcihostattr);
#else
#define     __sdhciPreStaShow(x)
#define     __sdhciIntStaShow(x)
#endif                                                                  /*  __SYLIXOS_DEBUG             */
/*********************************************************************************************************
  INLINE FUNCTIONS
*********************************************************************************************************/
static LW_INLINE INT __sdhciCmdRespType (PLW_SD_COMMAND psdcmd)
{
    UINT32  uiRespFlag = SD_RESP_TYPE(psdcmd);
    INT     iType      = 0;

    if (!(uiRespFlag & SD_RSP_PRESENT)) {
        iType = SDHCI_CMD_RESP_TYPE_NONE;
    } else if (uiRespFlag & SD_RSP_136) {
        iType = SDHCI_CMD_RESP_TYPE_LONG;
    } else if (uiRespFlag & SD_RSP_BUSY) {
        iType = SDHCI_CMD_RESP_TYPE_SHORT_BUSY;
    } else {
        iType = SDHCI_CMD_RESP_TYPE_SHORT;
    }

    return  (iType);
}
static LW_INLINE VOID __sdhciSdmaAddrUpdate (__PSDHCI_HOST psdhcihost, LONG lSysAddr)
{
    SDHCI_WRITEL(&psdhcihost->SDHCIHS_hostattr,
                 SDHCI_SYS_SDMA,
                 (UINT32)lSysAddr);
}
static LW_INLINE VOID __sdhciHostRest (__PSDHCI_HOST psdhcihost)
{
    SDHCI_WRITEB(&psdhcihost->SDHCIHS_hostattr,
                 SDHCI_SOFTWARE_RESET,
                 (SDHCI_SFRST_CMD | SDHCI_SFRST_DATA));
}
/*********************************************************************************************************
** ��������: API_SdhciHostCreate
** ��������: ����SD��׼��������
** ��    ��: psdAdapter     ����Ӧ��SD��������������
**           psdhcihostattr ����������
** ��    ��: NONE
** ��    ��: �ɹ�,��������ָ��.ʧ��,����NULL.
*********************************************************************************************************/
LW_API PVOID  API_SdhciHostCreate (CPCHAR               pcAdapterName,
                                   PLW_SDHCI_HOST_ATTR  psdhcihostattr)
{
    PLW_SD_ADAPTER  psdadapter  = LW_NULL;
    __PSDHCI_HOST   psdhcihost  = LW_NULL;
    PVOID           pvDmaBuf;
    INT             iError;

    if (!pcAdapterName || !psdhcihostattr) {
        _ErrorHandle(EINVAL);
        return  ((PVOID)LW_NULL);
    }

    if (psdhcihostattr->SDHCIHOST_uiMaxClock > __SDHCI_MAX_BASE_CLK) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "max clock must be less than 102.4Mhz.\r\n");
        _ErrorHandle(EINVAL);
        return  ((PVOID)LW_NULL);
    }

    psdhcihost = (__PSDHCI_HOST)__SHEAP_ALLOC(sizeof(__SDHCI_HOST));   /*  ����HOST                     */
    if (!psdhcihost) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "system low memory.\r\n");
        _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);
        return  ((PVOID)LW_NULL);
    }
    lib_bzero(psdhcihost, sizeof(__SDHCI_HOST));

    __sdhciHostCapDecode(psdhcihostattr, &psdhcihost->SDHCIHS_sdhcicap);/*  ���ع��ܽ���                */

    API_SdLibInit();                                                    /*  ��ʼ��sd�齨��              */

    psdhcihost->SDHCIHS_sdfunc.SDFUNC_pfuncMasterXfer = __sdhciTransfer;
    psdhcihost->SDHCIHS_sdfunc.SDFUNC_pfuncMasterCtl  = __sdhciIoCtl;   /*  ��ʼ����������              */

    iError = API_SdAdapterCreate(pcAdapterName, &psdhcihost->SDHCIHS_sdfunc);
    if (iError != ERROR_NONE) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "create sd adapter error.\r\n");
        __SHEAP_FREE(psdhcihost);
        return  ((PVOID)LW_NULL);
    }

    psdadapter = API_SdAdapterGet(pcAdapterName);                       /*  ���������                  */

    psdhcihost->SDHCIHS_psdadapter             = psdadapter;
    psdhcihost->SDHCIHS_atomicDevCnt.counter   = 0;                     /*  ��ʼ�豸��Ϊ0               */
    psdhcihost->SDHCIHS_pfuncMasterXfer        = __sdhciTransferNorm;
    psdhcihost->SDHCIHS_ucTransferMod          = SDHCIHOST_TMOD_SET_NORMAL;
                                                                        /*  Ĭ��ʹ��һ�㴫��            */
#if LW_CFG_VMM_EN
    pvDmaBuf = API_VmmDmaAlloc(__SDHCI_DMA_BOUND_LEN);
    if (!pvDmaBuf) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "system low memory.\r\n");
        __SHEAP_FREE(psdhcihost);
        return  ((PVOID)LW_NULL);
    }
    psdhcihost->SDHCIHS_pucDmaBuf = (UINT8 *)pvDmaBuf;
#endif

    lib_memcpy(&psdhcihost->SDHCIHS_hostattr, psdhcihostattr, sizeof(LW_SDHCI_HOST_ATTR));
                                                                        /*  ����������                  */
    __sdhciHostRest(psdhcihost);
    __sdhciPowerSetVol(psdhcihost, SDHCI_POWCTL_330, LW_TRUE);          /*  3.3v                        */
    SDHCI_WRITEB(&psdhcihost->SDHCIHS_hostattr, SDHCI_TIMEOUT_CONTROL, 0xe);
                                                                        /*  ʹ�����ʱ                */
    __sdhciIntDisAndEn(psdhcihost,
                       SDHCI_INT_ALL_MASK,
                       SDHCI_INT_BUS_POWER | SDHCI_INT_DATA_END_BIT |
                       SDHCI_INT_DATA_CRC  | SDHCI_INT_DATA_TIMEOUT |
                       SDHCI_INT_INDEX     | SDHCI_INT_END_BIT      |
                       SDHCI_INT_CRC       | SDHCI_INT_TIMEOUT      |
                       SDHCI_INT_DATA_END  | SDHCI_INT_RESPONSE);       /*  ��ʼ��ʱ���������ж�        */

    return  ((PVOID)psdhcihost);
}
/*********************************************************************************************************
** ��������: API_SdhciHostDelete
** ��������: ɾ��SD��׼��������
** ��    ��: pvHost    ������ָ��
** ��    ��: NONE
** ��    ��: ERROR CODE
*********************************************************************************************************/
LW_API INT  API_SdhciHostDelete (PVOID  pvHost)
{
    __PSDHCI_HOST    psdhcihost = LW_NULL;

    if (!pvHost) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    psdhcihost   = (__PSDHCI_HOST)pvHost;

    if (API_AtomicGet(&psdhcihost->SDHCIHS_atomicDevCnt)) {
        _ErrorHandle(EBUSY);
        return  (PX_ERROR);
    }

    API_SdAdapterDelete(__SDHCI_HOST_NAME(psdhcihost));

#if LW_CFG_VMM_EN
    API_VmmDmaFree(psdhcihost->SDHCIHS_pucDmaBuf);
#endif

    __SHEAP_FREE(psdhcihost);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_SdhciHostTransferModGet
** ��������: �������֧�ֵĴ���ģʽ
** ��    ��: pvHost    ������ָ��
** ��    ��: NONE
** ��    ��: �ɹ�,���ش���ģʽ֧�����; ʧ��, ���� 0;
*********************************************************************************************************/
LW_API INT  API_SdhciHostTransferModGet (PVOID    pvHost)
{
    __PSDHCI_HOST   psdhcihost  = LW_NULL;
    INT             iTmodFlg    = SDHCIHOST_TMOD_CAN_NORMAL;            /*  ����֧��һ�㴫��            */

    if (!pvHost) {
        _ErrorHandle(EINVAL);
        return  (0);
    }

    psdhcihost   = (__PSDHCI_HOST)pvHost;

    if (psdhcihost->SDHCIHS_sdhcicap.SDHCICAP_bCanSdma) {
        iTmodFlg |= SDHCIHOST_TMOD_CAN_SDMA;
    }

    if (psdhcihost->SDHCIHS_sdhcicap.SDHCICAP_bCanAdma) {
        iTmodFlg |= SDHCIHOST_TMOD_CAN_ADMA;
    }

    return (iTmodFlg);
}
/*********************************************************************************************************
** ��������: API_SdhciHostTransferModSet
** ��������: �������صĴ���ģʽ
** ��    ��: pvHost    ������ָ��
**           iTmod     Ҫ���õĴ���ģʽ
** ��    ��: NONE
** ��    ��: ERROR CODE
*********************************************************************************************************/
LW_API INT  API_SdhciHostTransferModSet (PVOID    pvHost, INT   iTmod)
{
    __PSDHCI_HOST    psdhcihost = LW_NULL;

    if (!pvHost) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    psdhcihost   = (__PSDHCI_HOST)pvHost;

    /*
     * ���뱣֤������û���豸,���ܸ������ش���ģʽ
     */
    if (API_AtomicGet(&psdhcihost->SDHCIHS_atomicDevCnt)) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "device exist.\r\n");
        _ErrorHandle(EBUSY);
        return  (PX_ERROR);
    }

    switch (iTmod) {
    
    case SDHCIHOST_TMOD_SET_NORMAL:
        psdhcihost->SDHCIHS_ucTransferMod    = SDHCIHOST_TMOD_SET_NORMAL;
        psdhcihost->SDHCIHS_pfuncMasterXfer  = __sdhciTransferNorm;
        break;

    case SDHCIHOST_TMOD_SET_SDMA:
        if (psdhcihost->SDHCIHS_sdhcicap.SDHCICAP_bCanSdma) {
            psdhcihost->SDHCIHS_ucTransferMod    = SDHCIHOST_TMOD_SET_SDMA;
            psdhcihost->SDHCIHS_pfuncMasterXfer  = __sdhciTransferSdma;
        } else {
            return  (PX_ERROR);
        }
        break;

    case SDHCIHOST_TMOD_SET_ADMA:
        if (psdhcihost->SDHCIHS_sdhcicap.SDHCICAP_bCanAdma) {
            psdhcihost->SDHCIHS_ucTransferMod     = SDHCIHOST_TMOD_SET_ADMA;
            psdhcihost->SDHCIHS_pfuncMasterXfer   = __sdhciTransferAdma;
        } else {
            return  (PX_ERROR);
        }
        break;

    default:
        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_SdhciDeviceAdd
** ��������: ��SD��׼�������������һ��SD�豸
** ��    ��: pvHost      ������ָ��
**           pcDevice    Ҫ��ӵ�SD�豸����
** ��    ��: NONE
** ��    ��: �ɹ�,�����豸ָ��.ʧ��,����NULL.
*********************************************************************************************************/
LW_API PVOID  API_SdhciDeviceAdd (PVOID     pvHost,
                                  CPCHAR    pcDeviceName)
{
    __PSDHCI_HOST   psdhcihost    = LW_NULL;
    __PSDHCI_DEVICE psdhcidevice  = LW_NULL;
    PLW_SD_DEVICE   psddevice     = LW_NULL;

    if (!pvHost) {
        _ErrorHandle(EINVAL);
        return  ((PVOID)0);
    }

    psdhcihost = (__PSDHCI_HOST)pvHost;

    if (API_AtomicGet(&psdhcihost->SDHCIHS_atomicDevCnt) >= __SDHCI_HOST_DEVNUM_MAX) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "can not add more devices.\r\n");
        return  ((PVOID)0);
    }

    psddevice = API_SdDeviceGet(__SDHCI_HOST_NAME(psdhcihost), pcDeviceName);
    if (!psddevice) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "find sd device failed.\r\n");
       return  ((PVOID)0);
    }

    /*
     * ����һ��SDHCI DEVICE �ṹ
     */
    psdhcidevice = (__PSDHCI_DEVICE)__SHEAP_ALLOC(sizeof(__SDHCI_DEVICE));
    if (!psdhcidevice) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "system low memory.\r\n");
        _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);
        return  ((PVOID)0);
    }

    lib_bzero(psdhcidevice, sizeof(__SDHCI_DEVICE));

    psdhcidevice->SDHCIDEV_psddevice  = psddevice;
    psdhcidevice->SDHCIDEV_psdhcihost = psdhcihost;

    psddevice->SDDEV_pvUsr = psdhcidevice;                              /*  ���û�����                */

    API_AtomicInc(&psdhcihost->SDHCIHS_atomicDevCnt);                   /*  �豸++                      */

    return  ((PVOID)psdhcidevice);
}
/*********************************************************************************************************
** ��������: API_SdhciDeviceRemove
** ��������: ��SD��׼�����������Ƴ�һ��SD�豸
** ��    ��: pvDevice  Ҫ�Ƴ���SD�豸ָ��
** ��    ��: NONE
** ��    ��: ERROR CODE
*********************************************************************************************************/
LW_API INT  API_SdhciDeviceRemove (PVOID pvDevice)
{
    __PSDHCI_HOST   psdhcihost    = LW_NULL;
    __PSDHCI_DEVICE psdhcidevice  = LW_NULL;

    if (!pvDevice) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    psdhcidevice  = (__PSDHCI_DEVICE)pvDevice;
    psdhcihost    = psdhcidevice->SDHCIDEV_psdhcihost;

    psdhcidevice->SDHCIDEV_psddevice->SDDEV_pvUsr = LW_NULL;            /*  �û�������Ч                */
    __SHEAP_FREE(psdhcidevice);
    API_AtomicDec(&psdhcihost->SDHCIHS_atomicDevCnt);                   /*  �豸--                      */

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_SdhciDeviceUsageInc
** ��������: �豸��ʹ�ü�������һ
** ��    ��: pvDevice  SD�豸ָ��
** ��    ��: NONE
** ��    ��: ERROR CODE
*********************************************************************************************************/
LW_API INT  API_SdhciDeviceUsageInc (PVOID  pvDevice)
{
    __PSDHCI_DEVICE psdhcidevice  = LW_NULL;
    INT             iError;

    if (!pvDevice) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    psdhcidevice  = (__PSDHCI_DEVICE)pvDevice;

    iError = API_SdDeviceUsageInc(psdhcidevice->SDHCIDEV_psddevice);

    return  (iError);
}
/*********************************************************************************************************
** ��������: API_SdhciDeviceUsageInc
** ��������: �豸��ʹ�ü�����һ
** ��    ��: pvDevice  SD�豸ָ��
** ��    ��: NONE
** ��    ��: ERROR CODE
*********************************************************************************************************/
LW_API INT  API_SdhciDeviceUsageDec (PVOID  pvDevice)
{
    __PSDHCI_DEVICE psdhcidevice  = LW_NULL;
    INT             iError;

    if (!pvDevice) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    psdhcidevice  = (__PSDHCI_DEVICE)pvDevice;

    iError = API_SdDeviceUsageDec(psdhcidevice->SDHCIDEV_psddevice);

    return  (iError);
}
/*********************************************************************************************************
** ��������: API_SdhciDeviceUsageInc
** ��������: ����豸��ʹ�ü���
** ��    ��: pvDevice  SD�豸ָ��
** ��    ��: NONE
** ��    ��: ERROR CODE
*********************************************************************************************************/
LW_API INT  API_SdhciDeviceUsageGet (PVOID  pvDevice)
{
    __PSDHCI_DEVICE psdhcidevice  = LW_NULL;
    INT             iError;

    if (!pvDevice) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    psdhcidevice  = (__PSDHCI_DEVICE)pvDevice;

    iError = API_SdDeviceUsageGet(psdhcidevice->SDHCIDEV_psddevice);

    return  (iError);
}
/*********************************************************************************************************
** ��������: __sdhciTransferNorm
** ��������: һ�㴫��
** ��    ��: psdhcihost  SD��������
**           psddev      SD�豸
**           psdmsg      SD���������Ϣ��
**           iNum        ��Ϣ����
** ��    ��: NONE
** ��    ��: ERROR CODE
*********************************************************************************************************/
static INT __sdhciTransferNorm (__PSDHCI_HOST       psdhcihost,
                                PLW_SD_DEVICE       psddev,
                                PLW_SD_MESSAGE      psdmsg,
                                INT                 iNum)
{
    PLW_SD_COMMAND psdcmd  = LW_NULL;
    PLW_SD_DATA    psddat  = LW_NULL;
    INT            iError  = ERROR_NONE;
    INT            i       = 0;

    /*
     * ������Ϣ
     */
    while ((i < iNum) && (psdmsg != LW_NULL)) {
        psdcmd = psdmsg->SDMSG_psdcmdCmd;
        psddat = psdmsg->SDMSG_psdData;

        __sdhciDataPrepareNorm(psdhcihost, psddat);                     /*  ����׼��                    */
        iError = __sdhciSendCmd(psdhcihost, psdcmd, psddat);            /*  ��������                    */
        if (iError != ERROR_NONE) {
            _DebugHandle(__ERRORMESSAGE_LEVEL, "send cmd error.\r\n");
            return  (PX_ERROR);
        }

        /*
         * ���ݴ���
         */
        if (psddat) {

            /*
             * TODO: ��ģʽ..
             */
            if (SD_DAT_IS_STREAM(psddat)) {
                _DebugHandle(__ERRORMESSAGE_LEVEL, "don't support stream mode.\r\n");
                return  (PX_ERROR);

            }

            if (SD_DAT_IS_READ(psddat)) {
                iError = __sdhciDataReadNorm(psdhcihost,
                                             psddat->SDDAT_uiBlkSize,
                                             psddat->SDDAT_uiBlkNum,
                                             psdmsg->SDMSG_pucRdBuffer);
                if (iError != ERROR_NONE) {
                    _DebugHandle(__ERRORMESSAGE_LEVEL, "read error.\r\n");
                    return  (PX_ERROR);

                }
            } else {
                iError = __sdhciDataWriteNorm(psdhcihost,
                                              psddat->SDDAT_uiBlkSize,
                                              psddat->SDDAT_uiBlkNum,
                                              psdmsg->SDMSG_pucWrtBuffer);
                if (iError != ERROR_NONE) {
                    _DebugHandle(__ERRORMESSAGE_LEVEL, "write error.\r\n");
                    return  (PX_ERROR);
                }
            }
        }

        i++;
        psdmsg++;

        /*
         * ע��, û�д���ֹͣ����,��Ϊֻ�ж�����ʱ��ʹ��,
         * ����׼����֧��ACMD12(�Զ�ֹͣ�����),��˺���.
         */
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __sdhciTransferSdma
** ��������: SDMA����
** ��    ��: psdhcihost  SD��������
**           psddev      SD�豸
**           psdmsg      SD���������Ϣ��
**           iNum        ��Ϣ����
** ��    ��: NONE
** ��    ��: ERROR CODE
*********************************************************************************************************/
static INT __sdhciTransferSdma (__PSDHCI_HOST       psdhcihost,
                                PLW_SD_DEVICE       psddev,
                                PLW_SD_MESSAGE      psdmsg,
                                INT                 iNum)
{
    PLW_SD_COMMAND psdcmd  = LW_NULL;
    PLW_SD_DATA    psddat  = LW_NULL;
    INT            iError  = ERROR_NONE;
    INT            i       = 0;
    /*
     * ������Ϣ
     */
    while ((i < iNum) && (psdmsg != LW_NULL)) {
        psdcmd = psdmsg->SDMSG_psdcmdCmd;
        psddat = psdmsg->SDMSG_psdData;
        __sdhciDataPrepareSdma(psdhcihost, psdmsg);                     /*  ����׼��                    */
        iError = __sdhciSendCmd(psdhcihost, psdcmd, psddat);            /*  ��������                    */
        if (iError != ERROR_NONE) {
            _DebugHandle(__ERRORMESSAGE_LEVEL, "send cmd error.\r\n");
            return  (PX_ERROR);
        }

        /*
         * ���ݴ���
         */
        if (psddat) {
            iError = __sdhciDataFinishSdma(psdhcihost, psdmsg);
            if (iError != ERROR_NONE) {
                return (PX_ERROR);
            }
        }

        i++;
        psdmsg++;

        /*
         * ע��, û�д���ֹͣ����,��Ϊֻ�ж�����ʱ��ʹ��,
         * ����׼����֧��ACMD12(�Զ�ֹͣ�����),��˺���.
         */
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __sdhciTransferAdma
** ��������: ADMA����
** ��    ��: psdadapter  SD��������
**           psddev      SD�豸
**           psdmsg      SD���������Ϣ��
**           iNum        ��Ϣ����
** ��    ��: NONE
** ��    ��: ERROR CODE
*********************************************************************************************************/
static INT __sdhciTransferAdma (__PSDHCI_HOST       psdhcihost,
                                PLW_SD_DEVICE       psddev,
                                PLW_SD_MESSAGE      psdmsg,
                                INT                 iNum)
{
    /*
     *  TODO: adma mode support.
     */
    __sdhciDataPrepareAdma(LW_NULL, LW_NULL);                           /*  ������δʹ�� warning        */
    
    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: __sdhciTransfer
** ��������: ���ߴ��亯��
** ��    ��: psdadapter  SD����������
**           iCmd        ��������
**           lArg        ����
** ��    ��: NONE
** ��    ��: ERROR CODE
*********************************************************************************************************/
static INT __sdhciTransfer (PLW_SD_ADAPTER      psdadapter,
                            PLW_SD_DEVICE       psddev,
                            PLW_SD_MESSAGE      psdmsg,
                            INT                 iNum)
{
    __PSDHCI_HOST  psdhcihost = LW_NULL;
    INT            iError     = ERROR_NONE;

    if (!psdadapter) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    psdhcihost = (__PSDHCI_HOST)psdadapter->SDADAPTER_psdfunc;
    if (!psdhcihost) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "no sdhci host.\r\n");
        return  (PX_ERROR);
    }

    /*
     * ִ�������϶�Ӧ�����ߴ��亯��
     */
    iError = psdhcihost->SDHCIHS_pfuncMasterXfer(psdhcihost, psddev, psdmsg, iNum);

    return (iError);

}
/*********************************************************************************************************
** ��������: __sdhciIoCtl
** ��������: SD ������IO���ƺ���
** ��    ��: psdadapter  SD����������
**           iCmd        ��������
**           lArg        ����
** ��    ��: NONE
** ��    ��: ERROR CODE
*********************************************************************************************************/
static INT __sdhciIoCtl (PLW_SD_ADAPTER  psdadapter,
                         INT             iCmd,
                         LONG            lArg)
{
    __PSDHCI_HOST  psdhcihost = LW_NULL;
    INT            iError     = ERROR_NONE;

    if (!psdadapter) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    psdhcihost = (__PSDHCI_HOST)psdadapter->SDADAPTER_psdfunc;
    if (!psdhcihost) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "no sdhci host.\r\n");
        return  (PX_ERROR);
    }

    switch (iCmd) {
    
    case SDBUS_CTRL_POWEROFF:
        iError = __sdhciPowerOff(psdhcihost);
        break;

    case SDBUS_CTRL_POWERUP:
    case SDBUS_CTRL_POWERON:
        iError = __sdhciPowerOn(psdhcihost);
        break;

    case SDBUS_CTRL_SETBUSWIDTH:
        iError = __sdhciBusWidthSet(psdhcihost, (UINT32)lArg);
        break;

    case SDBUS_CTRL_SETCLK:
        if (lArg == SDARG_SETCLK_NORMAL) {
            iError =  __sdhciClockSet(psdhcihost, __SDHCI_NOR_CLK);
        } else if (lArg == SDARG_SETCLK_MAX) {
            iError =  __sdhciClockSet(psdhcihost, __SDHCI_MAX_CLK);
        } else {
            _DebugHandle(__ERRORMESSAGE_LEVEL, "setting clock is not supported.\r\n ");
            iError = PX_ERROR;
        }
        break;

    case SDBUS_CTRL_DELAYCLK:
        break;

    case SDBUS_CTRL_GETOCR:
        *(UINT32 *)lArg = psdhcihost->SDHCIHS_sdhcicap.SDHCICAP_uiVoltage;
        iError = ERROR_NONE;
        break;

    default:
        _DebugHandle(__ERRORMESSAGE_LEVEL, "invalidate command.\r\n");
        iError = PX_ERROR;
        break;
    }

    return  (iError);
}
/*********************************************************************************************************
** ��������: __sdhciHostCapDecode
** ��������: �������صĹ��ܼĴ���
** ��    ��: psdhcihostattr  ��������
** ��    ��: psdhcicap       ���ܽṹָ��
** ��    ��: ERROR CODE
*********************************************************************************************************/
static VOID __sdhciHostCapDecode (PLW_SDHCI_HOST_ATTR psdhcihostattr, __PSDHCI_CAPAB psdhcicap)

{
    UINT32  uiCapReg = 0;
    UINT32  uiTmp    = 0;

    if (!psdhcicap) {
        _ErrorHandle(EINVAL);
        return;
    }

    uiCapReg = SDHCI_READL(psdhcihostattr, SDHCI_CAPABILITIES);

    psdhcicap->SDHCICAP_uiBaseClkFreq  = (uiCapReg & SDHCI_CAP_BASECLK_MASK) >> SDHCI_CAP_BASECLK_SHIFT;
    psdhcicap->SDHCICAP_uiMaxBlkSize   = (uiCapReg & SDHCI_CAP_MAXBLK_MASK) >> SDHCI_CAP_MAXBLK_SHIFT;
    psdhcicap->SDHCICAP_bCanSdma       = (uiCapReg & SDHCI_CAP_CAN_DO_SDMA) ? LW_TRUE : LW_FALSE;
    psdhcicap->SDHCICAP_bCanAdma       = (uiCapReg & SDHCI_CAP_CAN_DO_ADMA) ? LW_TRUE : LW_FALSE;
    psdhcicap->SDHCICAP_bCanHighSpeed  = (uiCapReg & SDHCI_CAP_CAN_DO_HISPD) ? LW_TRUE : LW_FALSE;
    psdhcicap->SDHCICAP_bCanSusRes     = (uiCapReg & SDHCI_CAP_CAN_DO_SUSRES) ? LW_TRUE : LW_FALSE;

    psdhcicap->SDHCICAP_uiBaseClkFreq *= 1000000;                       /*  תΪʵ��Ƶ��                */
    psdhcicap->SDHCICAP_uiMaxBlkSize   = 512 << psdhcicap->SDHCICAP_uiMaxBlkSize;
                                                                        /*  ת��Ϊʵ�������С        */
    /*
     * ����ڴ˼Ĵ������ʱ��Ϊ0,��˵��������û���Լ��ڲ�ʱ��,������Դ������ʱ��Դ.
     * ��˲��������ʱ��Դ��Ϊ��ʱ��.
     */
    if (psdhcicap->SDHCICAP_uiBaseClkFreq == 0) {
        uiTmp = psdhcihostattr->SDHCIHOST_uiMaxClock;
        if (uiTmp == 0) {
            _DebugHandle(__ERRORMESSAGE_LEVEL, "no clock source .\r\n");
            return;
        }
        psdhcicap->SDHCICAP_uiBaseClkFreq = uiTmp;
    }

    uiTmp = 0;
    if ((uiCapReg & SDHCI_CAP_CAN_VDD_330) != 0) {
        uiTmp |= SD_VDD_32_33 | SD_VDD_33_34;
    }
    if ((uiCapReg & SDHCI_CAP_CAN_VDD_300) != 0) {
        uiTmp |= SD_VDD_29_30 | SD_VDD_30_31;
    }
    if ((uiCapReg & SDHCI_CAP_CAN_VDD_180) != 0) {
        uiTmp |= SD_VDD_165_195;
    }

    if (uiTmp == 0) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "no voltage information.\r\n");
        return;
    }
    psdhcicap->SDHCICAP_uiVoltage = uiTmp;

#ifdef  __SYLIXOS_DEBUG
#ifndef printk
#define printk
#endif                                                                  /*  printk                      */
    printk("\nhost capablity >>\n");
    printk("uiCapReg       : %08x\n", uiCapReg);
    printk("base clock     : %u\n", psdhcicap->SDHCICAP_uiBaseClkFreq);
    printk("max block size : %u\n", psdhcicap->SDHCICAP_uiMaxBlkSize);
    printk("can sdma       : %s\n", psdhcicap->SDHCICAP_bCanSdma ? "yes" : "no");
    printk("can adma       : %s\n", psdhcicap->SDHCICAP_bCanAdma ? "yes" : "no");
    printk("can high speed : %s\n", psdhcicap->SDHCICAP_bCanHighSpeed ? "yes" : "no");
    printk("voltage support: %08x\n", psdhcicap->SDHCICAP_uiVoltage);
#endif                                                                  /*  __SYLIXOS_DEBUG             */
}
/*********************************************************************************************************
** ��������: __sdhciClockSet
** ��������: ʱ��Ƶ������
** ��    ��: psdhcihost      SDHCI HOST�ṹָ��
**           uiSetClk        Ҫ���õ�ʱ��Ƶ��
** ��    ��: NONE
** ��    ��: ERROR CODE
*********************************************************************************************************/
static INT __sdhciClockSet (__PSDHCI_HOST  psdhcihost, UINT32 uiSetClk)
{
    PLW_SDHCI_HOST_ATTR psdhcihostattr = &psdhcihost->SDHCIHS_hostattr;
    UINT32              uiBaseClk      = psdhcihost->SDHCIHS_sdhcicap.SDHCICAP_uiBaseClkFreq;

    UINT16              usDivClk       = 0;
    UINT                uiTimeOut      = 30;                            /*  �ȴ�Ϊ30ms                  */

    if (uiSetClk > uiBaseClk) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "the clock to set is larger than base clock.\r\n");
        return  (PX_ERROR);
    }

    SDHCI_WRITEW(psdhcihostattr, SDHCI_CLOCK_CONTROL, 0);               /*  ��ֹʱ��ģ�������ڲ�����    */

    /*
     * �����Ƶ����
     */
    if (uiSetClk < uiBaseClk) {
        usDivClk = (UINT16)((uiBaseClk + uiSetClk - 1) / uiSetClk);
        if (usDivClk <= 2) {
            usDivClk = 1 << 0;
        } else if (usDivClk <= 4) {
            usDivClk = 1 << 1;
        } else if (usDivClk <= 8) {
            usDivClk = 1 << 2;
        } else if (usDivClk <= 16) {
            usDivClk = 1 << 3;
        } else if (usDivClk <= 32) {
            usDivClk = 1 << 4;
        } else if (usDivClk <= 64) {
            usDivClk = 1 << 5;
        } else if (usDivClk <= 128) {
            usDivClk = 1 << 6;
        } else {
            usDivClk = 1 << 7;
        }
    }

#ifdef __SYLIXOS_DEBUG
#ifndef printk
#define printk
#endif                                                                  /*  printk                      */
        printk("current clock: %uHz\n", uiBaseClk / (usDivClk << 1));
#endif                                                                  /*  __SYLIXOS_DEBUG             */

    usDivClk <<= SDHCI_CLKCTL_DIVIDER_SHIFT;
    usDivClk  |= SDHCI_CLKCTL_INTER_EN;                                 /*  �ڲ�ʱ��ʹ��                */

    SDHCI_WRITEW(psdhcihostattr, SDHCI_CLOCK_CONTROL, usDivClk);

    /*
     * �ȴ�ʱ���ȶ�
     */
    while (1) {
        usDivClk = SDHCI_READW(psdhcihostattr, SDHCI_CLOCK_CONTROL);
        if (usDivClk & SDHCI_CLKCTL_INTER_STABLE) {
            break;
        }

        uiTimeOut--;
        if (uiTimeOut == 0) {
            _DebugHandle(__ERRORMESSAGE_LEVEL, "wait internal clock to be stable timeout.\r\n");
            return  (PX_ERROR);
        }
        SDHCI_DELAYMS(1);
    }

    usDivClk |= SDHCI_CLKCTL_CLOCK_EN;                                  /*  SDCLK �豸ʱ��ʹ��          */
    SDHCI_WRITEW(psdhcihostattr, SDHCI_CLOCK_CONTROL, usDivClk);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __sdhciBusWidthSet
** ��������: ��������λ������
** ��    ��: psdhcihost      SDHCI HOST�ṹָ��
**           uiBusWidth      Ҫ���õ�����λ��
** ��    ��: NONE
** ��    ��: ERROR CODE
*********************************************************************************************************/
static INT __sdhciBusWidthSet (__PSDHCI_HOST  psdhcihost, UINT32 uiBusWidth)
{
    PLW_SDHCI_HOST_ATTR psdhcihostattr = &psdhcihost->SDHCIHS_hostattr;
    UINT8               ucCtl;
    UINT16              usIntStaEn;

    usIntStaEn  = SDHCI_READW(psdhcihostattr, SDHCI_INTSTA_ENABLE);
    usIntStaEn &= ~SDHCI_INTSTAEN_CARD_INT;
    SDHCI_WRITEW(psdhcihostattr, SDHCI_INTSTA_ENABLE, usIntStaEn);      /*  ��ֹ���ж�                  */

    ucCtl  = SDHCI_READB(psdhcihostattr, SDHCI_HOST_CONTROL);

    switch (uiBusWidth) {
    
    case SDARG_SETBUSWIDTH_1:
        ucCtl &= ~SDHCI_HCTRL_4BITBUS;
        break;

    case SDARG_SETBUSWIDTH_4:
        ucCtl |= SDHCI_HCTRL_4BITBUS;
        break;

    default:
        _DebugHandle(__ERRORMESSAGE_LEVEL, "parameter of bus width error.\r\n");
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    SDHCI_WRITEB(psdhcihostattr, SDHCI_HOST_CONTROL, ucCtl);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __sdhciPowerOn
** ��������: �򿪵�Դ
** ��    ��: psdhcihost     SDHCI HOST�ṹָ��
** ��    ��: NONE
** ��    ��: ERROR CODE
*********************************************************************************************************/
static INT __sdhciPowerOn (__PSDHCI_HOST  psdhcihost)
{
    PLW_SDHCI_HOST_ATTR psdhcihostattr = &psdhcihost->SDHCIHS_hostattr;
    UINT8               ucPow;

    ucPow  = SDHCI_READB(psdhcihostattr, SDHCI_POWER_CONTROL);
    ucPow |= SDHCI_POWCTL_ON;
    SDHCI_WRITEB(psdhcihostattr, SDHCI_POWER_CONTROL, ucPow);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __sdhciPowerff
** ��������: �رյ�Դ
** ��    ��: psdhcihost       SDHCI HOST�ṹָ��
** ��    ��: NONE
** ��    ��: ERROR CODE
*********************************************************************************************************/
static INT __sdhciPowerOff (__PSDHCI_HOST  psdhcihost)
{
    PLW_SDHCI_HOST_ATTR psdhcihostattr = &psdhcihost->SDHCIHS_hostattr;
    UINT8               ucPow;

    ucPow  = SDHCI_READB(psdhcihostattr, SDHCI_POWER_CONTROL);
    ucPow &= ~SDHCI_POWCTL_ON;
    SDHCI_WRITEB(psdhcihostattr, SDHCI_POWER_CONTROL, ucPow);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __sdhciPowerSetVol
** ��������: ��Դ���õ�ѹ
** ��    ��: psdhcihost      SDHCI HOST�ṹָ��
**           ucVol           ��ѹ
**           bIsOn           ����֮���Դ�Ƿ���
** ��    ��: NONE
** ��    ��: ERROR CODE
*********************************************************************************************************/
static INT __sdhciPowerSetVol (__PSDHCI_HOST  psdhcihost,
                               UINT8          ucVol,
                               BOOL           bIsOn)
{
    if (bIsOn) {
        ucVol |= SDHCI_POWCTL_ON;
    } else {
        ucVol &= ~SDHCI_POWCTL_ON;
    }

    __sdhciPowerOff(psdhcihost);

    SDHCI_WRITEB(&psdhcihost->SDHCIHS_hostattr, SDHCI_POWER_CONTROL, ucVol);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __sdhciSendCmd
** ��������: �����
** ��    ��: psdhcihost            SDHCI HOST�ṹָ��
**           psdcmd                ����ṹָ��
**           psddat                ���ݴ�����ƽṹ
** ��    ��: psdcmd->SDCMD_uiResp  �����Ӧ��,�������Ӧ����
** ��    ��: ERROR CODE
*********************************************************************************************************/
static INT  __sdhciSendCmd (__PSDHCI_HOST   psdhcihost,
                            PLW_SD_COMMAND  psdcmd,
                            PLW_SD_DATA     psddat)
{
    PLW_SDHCI_HOST_ATTR psdhcihostattr = &psdhcihost->SDHCIHS_hostattr;
    UINT32              uiRespFlag;
    UINT32              uiMask;
    UINT                uiTimeOut;
    INT                 iCmdFlg;

    struct timespec     tvOld;
    struct timespec     tvNow;

    uiMask = SDHCI_PSTA_CMD_INHIBIT;

    if (psddat || SD_CMD_TEST_RSP(psdcmd, SD_RSP_BUSY)) {
        uiMask |= SDHCI_PSTA_DATA_INHIBIT;
    }

    /*
     * �ȴ�����(����)�߿���
     */
    uiTimeOut = 0;
    lib_clock_gettime(CLOCK_MONOTONIC, &tvOld);
    while (SDHCI_READL(psdhcihostattr, SDHCI_PRESENT_STATE) & uiMask) {
        uiTimeOut++;
        if (uiTimeOut > __SDHCI_CMD_RETRY) {
            goto __cmd_free_timeout;
        }

        lib_clock_gettime(CLOCK_MONOTONIC, &tvNow);
        if ((tvNow.tv_sec - tvOld.tv_sec) >= __SDHCI_CMD_TOUT_SEC) {
            goto __cmd_free_timeout;
        }
    }

    /*
     * д�����Ĵ���
     */
    SDHCI_WRITEL(psdhcihostattr, SDHCI_ARGUMENT, psdcmd->SDCMD_uiArg);

    /*
     * ���ô���ģʽ
     */
    __sdhciTransferModSet(psdhcihost, psddat);

    /*
     * �����
     */
    iCmdFlg = __sdhciCmdRespType(psdcmd);                               /*  Ӧ������                    */
    if (SD_CMD_TEST_RSP(psdcmd, SD_RSP_CRC)) {
        iCmdFlg |= SDHCI_CMD_CRC_CHK;
    }
    if (SD_CMD_TEST_RSP(psdcmd, SD_RSP_OPCODE)) {
        iCmdFlg |= SDHCI_CMD_INDEX_CHK;
    }
    if (psddat) {
        iCmdFlg |= SDHCI_CMD_DATA;                                      /*  ��������                    */
    }

    SDHCI_WRITEW(psdhcihostattr, SDHCI_COMMAND, SDHCI_MAKE_CMD(psdcmd->SDCMD_uiOpcode, iCmdFlg));
                                                                        /*  д������Ĵ���              */
    /*
     * �ȴ��������
     */
    uiTimeOut = 0;
    lib_clock_gettime(CLOCK_MONOTONIC, &tvOld);
    while (1) {
        uiMask = SDHCI_READL(psdhcihostattr, SDHCI_INT_STATUS);         /*  ���ж�״̬                  */
        if (uiMask & SDHCI_INT_CMD_MASK) {
            SDHCI_WRITEL(psdhcihostattr,
                                SDHCI_INT_STATUS,
                                uiMask);                                /*  д1 �����Ӧ�жϱ�־λ      */
            break;
        }

        uiTimeOut++;
        if (uiTimeOut > __SDHCI_CMD_RETRY) {
            goto __cmd_end_timeout;
        }

        lib_clock_gettime(CLOCK_MONOTONIC, &tvNow);
        if ((tvNow.tv_sec - tvOld.tv_sec) >= __SDHCI_CMD_TOUT_SEC) {
            goto __cmd_end_timeout;
        }
    }

    if (uiMask & SDHCI_INT_TIMEOUT) {
        goto __cmd_end_timeout;
    }

    if (uiMask & SDHCI_INT_CRC) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "crc error.\r\n");
        return  (PX_ERROR);
    }

    if (uiMask & SDHCI_INT_END_BIT) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "end bit error.\r\n");
        return  (PX_ERROR);
    }

    if (uiMask & SDHCI_INT_INDEX) {                                     /*  ���������                */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "command check error.\r\n");
        return  (PX_ERROR);
    }

    if (!(uiMask & SDHCI_INT_RESPONSE)) {                               /*  û�еõ�Ӧ��                */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "response error.\r\n");
        return  (PX_ERROR);
    }

    /*
     * ���������ȷ,Ӧ����
     */
    uiRespFlag = SD_RESP_TYPE(psdcmd);
    if (uiRespFlag & SD_RSP_PRESENT) {
        UINT32  *puiResp = psdcmd->SDCMD_uiResp;

        /*
         * ��Ӧ��,��ΪӦ��Ĵ�����ȥ����CRC��һ���ֽ�,�������λ����.
         * ������Ӧ���ֽ������ϲ�涨���෴,����Ӧ��ת��.
         */
        if (uiRespFlag & SD_RSP_136) {

            UINT32   uiRsp0;
            UINT32   uiRsp1;
            UINT32   uiRsp2;
            UINT32   uiRsp3;

            uiRsp3     = SDHCI_READL(psdhcihostattr, SDHCI_RESPONSE3);
            uiRsp2     = SDHCI_READL(psdhcihostattr, SDHCI_RESPONSE2);
            uiRsp1     = SDHCI_READL(psdhcihostattr, SDHCI_RESPONSE1);
            uiRsp0     = SDHCI_READL(psdhcihostattr, SDHCI_RESPONSE0);

            puiResp[0] = (uiRsp3 << 8) | ( uiRsp2 >> 24);
            puiResp[1] = (uiRsp2 << 8) | ( uiRsp1 >> 24);
            puiResp[2] = (uiRsp1 << 8) | ( uiRsp0 >> 24);
            puiResp[3] = (uiRsp0 << 8);
        } else {
            puiResp[0] = SDHCI_READL(psdhcihostattr, SDHCI_RESPONSE0);
        }
    }

    return  (ERROR_NONE);

__cmd_free_timeout:
    _DebugHandle(__ERRORMESSAGE_LEVEL, "wait command\\data line timeout.\r\n");
    return  (PX_ERROR);

__cmd_end_timeout:
    _DebugHandle(__ERRORMESSAGE_LEVEL, "wait command complete timeout.\r\n");

    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: __sdhciTransferModSet
** ��������: ���ô���ģʽ
** ��    ��: psdhcihost      SDHCI HOST�ṹָ��
**           psddat          ���ݴ�����ƻ���
** ��    ��: NONE
** ��    ��: NONE
*********************************************************************************************************/
static VOID __sdhciTransferModSet (__PSDHCI_HOST  psdhcihost, PLW_SD_DATA psddat)
{
    PLW_SDHCI_HOST_ATTR psdhcihostattr = &psdhcihost->SDHCIHS_hostattr;
    UINT16              usTmod;

    if (!psddat) {
        return;
    }

    usTmod = SDHCI_TRNS_BLK_CNT_EN;                                     /*  ʹ�ܿ����                  */

    if (psddat->SDDAT_uiBlkNum > 1) {
        usTmod |= SDHCI_TRNS_MULTI | SDHCI_TRNS_ACMD12;                 /*  ʼ��ʹ��ACMD12����          */
    }

    if (SD_DAT_IS_READ(psddat)) {
        usTmod |= SDHCI_TRNS_READ;
    }

    if (psdhcihost->SDHCIHS_ucTransferMod != SDHCIHOST_TMOD_SET_NORMAL) {
        usTmod |= SDHCI_TRNS_DMA;
    }

    SDHCI_WRITEW(psdhcihostattr, SDHCI_TRANSFER_MODE, usTmod);
}
/*********************************************************************************************************
** ��������: __sdhciDataPrepareNorm
** ��������: һ�����ݴ���׼��
** ��    ��: psdhcihost      SDHCI HOST�ṹָ��
**           psddat          ���ݴ�����ƻ���
** ��    ��: NONE
** ��    ��: NONE
*********************************************************************************************************/
static VOID __sdhciDataPrepareNorm (__PSDHCI_HOST  psdhcihost, PLW_SD_DATA psddat)
{
    PLW_SDHCI_HOST_ATTR psdhcihostattr = &psdhcihost->SDHCIHS_hostattr;

    if (!psddat) {
        return;
    }

    __sdhciTransferIntSet(psdhcihost);                                  /*  �����ж�ʹ��                */

    SDHCI_WRITEW(psdhcihostattr,
                 SDHCI_BLOCK_SIZE,
                 SDHCI_MAKE_BLKSZ(__SDHCI_DMA_BOUND_NBITS,
                                  psddat->SDDAT_uiBlkSize));            /*  ���С�Ĵ���                */
    SDHCI_WRITEW(psdhcihostattr,
                 SDHCI_BLOCK_COUNT,
                 psddat->SDDAT_uiBlkNum);                               /*  ������Ĵ���                */
}
/*********************************************************************************************************
** ��������: __sdhciDataPrepareSdma
** ��������: DMA���ݴ���׼��
** ��    ��: psdhcihost      SDHCI HOST�ṹָ��
**           psdmsg          ���������Ϣ�ṹָ��
** ��    ��: NONE
** ��    ��: NONE
*********************************************************************************************************/
static VOID __sdhciDataPrepareSdma (__PSDHCI_HOST  psdhcihost, PLW_SD_MESSAGE psdmsg)
{
    PLW_SD_DATA  psddata = psdmsg->SDMSG_psdData;
    UINT8       *pucSdma;

    if (!psddata) {
        return;
    }

#if LW_CFG_VMM_EN
    pucSdma = psdhcihost->SDHCIHS_pucDmaBuf;
    if (SD_DAT_IS_WRITE(psddata)) {                                     /*  �����DMAд,�Ȱ��û����ݿ���*/
        lib_memcpy(pucSdma,
                   psdmsg->SDMSG_pucWrtBuffer,
                   psddata->SDDAT_uiBlkNum * psddata->SDDAT_uiBlkSize);
    }
#else
    if (SD_DAT_IS_READ(psddata)) {
        pucSdma = psdmsg->SDMSG_pucRdBuffer;
    } else {
        pucSdma = psdmsg->SDMSG_pucWrtBuffer;
    }
#endif
    
    /*
     * TODO: STREAM MODE
     */
    __sdhciSdmaAddrUpdate(psdhcihost, (LONG)pucSdma);                   /*  д��DMAϵͳ��ַ�Ĵ���       */
                                                                        /*  TODO: ��ַ����  ?           */
    __sdhciDataPrepareNorm(psdhcihost, psddata);
}
/*********************************************************************************************************
** ��������: __sdhciDataPrepareNorm
** ��������: ��ЧDMA���ݴ���׼��
** ��    ��: psdhcihost      SDHCI HOST�ṹָ��
**           psddat          ���ݴ�����ƻ���
** ��    ��: NONE
** ��    ��: NONE
*********************************************************************************************************/
static VOID __sdhciDataPrepareAdma (__PSDHCI_HOST  psdhcihost, PLW_SD_DATA psddat)
{
}
/*********************************************************************************************************
** ��������: __sdhciDataReadNorm
** ��������: һ�����ݴ���(������)
** ��    ��: psdhcihost      SDHCI HOST�ṹָ��
**           uiBlkSize       ���С
**           uiBlkNum        ������
** ��    ��: pucRdBuff       �������
** ��    ��: ERROR CODE
*********************************************************************************************************/
static INT __sdhciDataReadNorm (__PSDHCI_HOST psdhcihost,
                                UINT32        uiBlkSize,
                                UINT32        uiBlkNum,
                                PUCHAR        pucRdBuff)
{
    PLW_SDHCI_HOST_ATTR psdhcihostattr = &psdhcihost->SDHCIHS_hostattr;
    UINT32              uiRdTmp;
    UINT32              uiBlkSizeTmp = uiBlkSize;
    UINT32              uiIntSta;
    INT                 iRetry = 0;

    struct timespec   tvOld;
    struct timespec   tvNow;

    while (uiBlkNum > 0) {
        iRetry = 0;
        lib_clock_gettime(CLOCK_MONOTONIC, &tvOld);
        while (!(SDHCI_READL(psdhcihostattr, SDHCI_PRESENT_STATE) & SDHCI_PSTA_DATA_AVAILABLE)) {
            iRetry++;
            if (iRetry > __SDHCI_READ_RETRY) {
                goto __read_ready_timeout;
            }

            lib_clock_gettime(CLOCK_MONOTONIC, &tvNow);
            if ((tvNow.tv_sec - tvOld.tv_sec) > __SDHCI_READ_TOUT_SEC) {
                goto __read_ready_timeout;
            }
        }

        /*
         * ���ݶ��ж����Կ�Ϊ��λ��,���,һ�ζ�ȡһ����.
         * ע��,����Ŀ��СӦ������4�ֽڶԳ�.ע�����ϲ㴦��.
         */
        uiBlkSize = uiBlkSizeTmp;
        while (uiBlkSize > 0) {
            uiRdTmp      = SDHCI_READL(psdhcihostattr, SDHCI_BUFFER);
            *pucRdBuff++ =  uiRdTmp & 0xff;
            *pucRdBuff++ = (uiRdTmp >> 8) & 0xff;
            *pucRdBuff++ = (uiRdTmp >> 16) & 0xff;
            *pucRdBuff++ = (uiRdTmp >> 24) & 0xff;
            uiBlkSize   -= 4;
        }
        uiBlkNum--;
    }

    /*
     * ���ڶ��򵥿鴫��,��������Ҫ�鿴��������жϱ�־.
     * ���������ƵĴ���,�����ݴ�����ɺ�,��Ҫ������ֹ����.
     * ���ڵ�ǰ�汾,û��ʹ�������ƴ���.
     */
    iRetry = 0;
    lib_clock_gettime(CLOCK_MONOTONIC, &tvOld);
    while (!((uiIntSta = SDHCI_READL(psdhcihostattr, SDHCI_INT_STATUS)) &
              SDHCI_INTSTA_DATA_END)) {
        iRetry++;
        if (iRetry >= __SDHCI_READ_RETRY) {
            goto __read_end_timeout;
        }

        lib_clock_gettime(CLOCK_MONOTONIC, &tvNow);
        if ((tvNow.tv_sec - tvOld.tv_sec) > __SDHCI_READ_TOUT_SEC) {
            goto __read_end_timeout;
        }
    }
    SDHCI_WRITEL(psdhcihostattr, SDHCI_INT_STATUS, uiIntSta);           /*  ���״̬                    */

    return  (ERROR_NONE);

__read_ready_timeout:
    _DebugHandle(__ERRORMESSAGE_LEVEL, "wait block ready timeout.\r\n");
    return  (PX_ERROR);

__read_end_timeout:
    _DebugHandle(__ERRORMESSAGE_LEVEL, "wait complete timeout.\r\n");
    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: __sdhciDataWriteNorm
** ��������: һ�����ݴ���(д����)
** ��    ��: psdhcihost      SDHCI HOST�ṹָ��
**           uiBlkSize       ���С
**           uiBlkNum        ������
**           pucWrtBuf       д����
** ��    ��: NONE
** ��    ��: ERROR CODE
*********************************************************************************************************/
static INT __sdhciDataWriteNorm (__PSDHCI_HOST psdhcihost,
                                 UINT32        uiBlkSize,
                                 UINT32        uiBlkNum,
                                 PUCHAR        pucWrtBuff)
{
    PLW_SDHCI_HOST_ATTR psdhcihostattr = &psdhcihost->SDHCIHS_hostattr;
    UINT32              uiBlkSizeTmp   = uiBlkSize;
    UINT32              uiWrtTmp;
    INT                 iRetry;
    UINT32              uiIntSta;

    struct timespec     tvOld;
    struct timespec     tvNow;

    while (uiBlkNum > 0) {
        /*
         * �ȴ�д������Ч
         */
        iRetry = 0;
        lib_clock_gettime(CLOCK_MONOTONIC, &tvOld);
        while (!((uiIntSta = SDHCI_READL(psdhcihostattr, SDHCI_PRESENT_STATE)) &
                SDHCI_PSTA_SPACE_AVAILABLE)) {
            iRetry++;
            if (iRetry >= __SDHCI_WRITE_RETRY) {
                goto __write_ready_timeout;
            }

            lib_clock_gettime(CLOCK_MONOTONIC, &tvNow);
            if ((tvNow.tv_sec - tvOld.tv_sec) > __SDHCI_WRITE_TOUT_SEC) {
                goto __write_ready_timeout;
            }
        }
        SDHCI_WRITEL(psdhcihostattr, SDHCI_INT_STATUS, uiIntSta);       /*  ���״̬                    */

        /*
         * ����д�ж����Կ�Ϊ��λ��,���,һ��д��һ����.
         * ע��,����Ŀ��СӦ������4�ֽڶԳ�.ע�����ϲ㴦��.
         */
        uiBlkSize = uiBlkSizeTmp;
        while (uiBlkSize > 0) {
            uiWrtTmp   =  *pucWrtBuff++;
            uiWrtTmp  |= (*pucWrtBuff++) << 8;
            uiWrtTmp  |= (*pucWrtBuff++) << 16;
            uiWrtTmp  |= (*pucWrtBuff++) << 24;
            uiBlkSize -= 4;
            SDHCI_WRITEL(psdhcihostattr, SDHCI_BUFFER, uiWrtTmp);
        }
        uiBlkNum--;
    }

    /*
     * ���ڶ��򵥿鴫��,��������Ҫ�鿴��������жϱ�־.
     * ���������ƵĴ���,�����ݴ�����ɺ�,��Ҫ������ֹ����.
     * ���ڵ�ǰ�汾,û��ʹ�������ƴ���.
     */
    iRetry = 0;
    lib_clock_gettime(CLOCK_MONOTONIC, &tvOld);
    while (!((uiIntSta = SDHCI_READL(psdhcihostattr, SDHCI_INT_STATUS)) &
              SDHCI_INTSTA_DATA_END)) {
        iRetry++;
        if (iRetry >= __SDHCI_WRITE_RETRY) {
            goto __write_end_timeout;
        }

        lib_clock_gettime(CLOCK_MONOTONIC, &tvNow);
        if ((tvNow.tv_sec - tvOld.tv_sec) > __SDHCI_WRITE_TOUT_SEC) {
            goto __write_end_timeout;
        }
    }

    SDHCI_WRITEL(psdhcihostattr, SDHCI_INT_STATUS, uiIntSta);           /*  ���״̬                    */

    return  (ERROR_NONE);

__write_ready_timeout:
    _DebugHandle(__ERRORMESSAGE_LEVEL, "wait block ready timeout.\r\n");
    return  (PX_ERROR);

__write_end_timeout:
    _DebugHandle(__ERRORMESSAGE_LEVEL, "wait complete timeout.\r\n");
    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: __sdhciDataFinishSdma
** ��������: SDMA���ݴ�����ɴ���
** ��    ��: psdhcihost       SDHCI HOST�ṹָ��
**           psdmsg
** ��    ��: NONE
** ��    ��: ERROR CODE
*********************************************************************************************************/
static INT __sdhciDataFinishSdma (__PSDHCI_HOST   psdhcihost,  PLW_SD_MESSAGE psdmsg)
{
    PLW_SDHCI_HOST_ATTR  psdhcihostattr = &psdhcihost->SDHCIHS_hostattr;
    PLW_SD_DATA          psddat         = psdmsg->SDMSG_psdData;
    UINT8               *pucSdma;

#if LW_CFG_VMM_EN
    UINT8               *pucDst;
#endif

    INT                  iRetry;
    UINT32               uiIntSta;

    struct timespec      tvOld;
    struct timespec      tvNow;

#if LW_CFG_VMM_EN
    if (SD_DAT_IS_READ(psddat)) {
        pucDst = psdmsg->SDMSG_pucRdBuffer;
    }
    pucSdma = psdhcihost->SDHCIHS_pucDmaBuf;
#else
    if (SD_DAT_IS_READ(psdmsg->SDMSG_psdData)) {
        pucSdma = psdmsg->SDMSG_pucRdBuffer;
    } else {
        pucSdma = psdmsg->SDMSG_pucWrtBuffer;
    }
#endif

    /*
     * ʹ��SDAMʱ, ��׼����������һ���԰��������ڴ��ϵ�512k����,
     * ��������߽�, ���ػ����һ��DMA�ж�,֪ͨ��Ҫ����DMA�����ַ.
     * Ŀǰ����汾�Ѿ�����Ϊ512k�߽��ַ.
     * ��������(�ڿ��С�Ϳ������Ĵ����б�ʾ��)�������,����һ����������ж�.
     * ��������жϵ����ȼ�����DMA�ж�.
     */
    iRetry = 0;
    lib_clock_gettime(CLOCK_MONOTONIC, &tvOld);
    while (1) {
        uiIntSta = SDHCI_READL(psdhcihostattr, SDHCI_INT_STATUS);
        if (uiIntSta & SDHCI_INTSTA_DATA_END) {                         /*  ���ݴ������                */
            SDHCI_WRITEL(psdhcihostattr, SDHCI_INT_STATUS, uiIntSta);   /*  ���״̬                    */
            return  (ERROR_NONE);
        }

        if (uiIntSta & SDHCI_INTSTA_DMA) {                              /*  DMA�߽��ж�                 */
            pucSdma += __SDHCI_DMA_BOUND_LEN;                           /*  ����DMA��ַ                 */
            __sdhciSdmaAddrUpdate(psdhcihost, (LONG)pucSdma);
        }

        iRetry++;
        if (iRetry > __SDHCI_SDMARW_RETRY) {
            goto __sdma_timeout;
        }

        lib_clock_gettime(CLOCK_MONOTONIC, &tvNow);
        if ((tvNow.tv_sec - tvOld.tv_sec) > __SDHCI_WRITE_TOUT_SEC) {
            goto __sdma_timeout;
        }
    }

    /*
     * ���������VMM,����Ҫ��DMA���������ݿ������û�����cache��������
     */
#if LW_CFG_VMM_EN
     if (SD_DAT_IS_READ(psddat)) {
         lib_memcpy(pucDst, pucSdma, psddat->SDDAT_uiBlkNum * psddat->SDDAT_uiBlkSize);
     }
#endif

__sdma_timeout:
    _DebugHandle(__ERRORMESSAGE_LEVEL, "timeout.\r\n");
    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: __sdhciTransferIntSet
** ��������: ���ݴ����ж�����
** ��    ��: psdhcihost       SDHCI HOST�ṹָ��
** ��    ��: NONE
** ��    ��: NONE
*********************************************************************************************************/
static VOID __sdhciTransferIntSet (__PSDHCI_HOST  psdhcihost)
{
    PLW_SDHCI_HOST_ATTR psdhcihostattr = &psdhcihost->SDHCIHS_hostattr;

    UINT32              uiIntIoMsk;                                     /*  ʹ��һ�㴫��ʱ���ж�����    */
    UINT32              uiIntDmaMsk;                                    /*  ʹ��DMA����ʱ���ж�����     */
    UINT32              uiEnMask;                                       /*  ����ʹ������                */

    uiIntIoMsk  = SDHCI_INT_SPACE_AVAIL | SDHCI_INT_DATA_AVAIL;
    uiIntDmaMsk = SDHCI_INT_DMA_END     | SDHCI_INT_ADMA_ERROR;
    uiEnMask    = SDHCI_READL(psdhcihostattr, SDHCI_INTSTA_ENABLE);     /*  ��ȡ32λ(���������ж�)      */

    if (psdhcihost->SDHCIHS_ucTransferMod != SDHCIHOST_TMOD_SET_NORMAL) {
        uiEnMask &= ~uiIntIoMsk;
        uiEnMask |=  uiIntDmaMsk;
    } else {
        uiEnMask &= ~uiIntDmaMsk;
        uiEnMask |=  uiIntIoMsk;
    }

    uiEnMask |= SDHCI_INT_RESPONSE | SDHCI_INT_DATA_END;

    /*
     * ��Ϊ�������Ĵ�����λ��λ�ö���ȫ��ͬ,
     * ���Կ���ͬʱ��һ������.
     */
    SDHCI_WRITEL(psdhcihostattr, SDHCI_INTSTA_ENABLE, uiEnMask);
    SDHCI_WRITEL(psdhcihostattr, SDHCI_SIGNAL_ENABLE, uiEnMask);
}
/*********************************************************************************************************
** ��������: __sdhciIntDisAndEn
** ��������: �ж�����(ʹ��\����).���������ж�״̬(����\һ��״̬)���ж��ź�(����\һ���ź�)�Ĵ���.
** ��    ��: psdhcihost      SDHCI HOST�ṹָ��
**           uiDisMask       ��������
**           uiEnMask        ʹ������
** ��    ��: NONE
** ��    ��: NONE
*********************************************************************************************************/
static VOID __sdhciIntDisAndEn (__PSDHCI_HOST  psdhcihost,
                                UINT32         uiDisMask,
                                UINT32         uiEnMask)
{
    PLW_SDHCI_HOST_ATTR psdhcihostattr = &psdhcihost->SDHCIHS_hostattr;
    UINT32              uiMask;

    uiMask  =  SDHCI_READW(psdhcihostattr, SDHCI_INTSTA_ENABLE);
    uiMask &= ~uiDisMask;
    uiMask |=  uiEnMask;

    /*
     * ��Ϊ�������Ĵ�����λ��λ�ö���ȫ��ͬ,
     * ���Կ���ͬʱ��һ������.
     */
    SDHCI_WRITEW(psdhcihostattr, SDHCI_INTSTA_ENABLE, (UINT16)uiMask);
    SDHCI_WRITEW(psdhcihostattr, SDHCI_SIGNAL_ENABLE, (UINT16)uiMask);
}
/*********************************************************************************************************
** ��������: __sdhciPreStaShow
** ��������: ��ʾ��ǰ����״̬
** ��    ��: lBasePoint
** ��    ��: NONE
** ��    ��: NONE
*********************************************************************************************************/
#ifdef  __SYLIXOS_DEBUG

static VOID __sdhciPreStaShow(PLW_SDHCI_HOST_ATTR psdhcihostattr)
{
#ifndef printk
#define printk
#endif                                                                  /*  printk                      */

    UINT32  uiSta;

    uiSta = SDHCI_READL(psdhcihostattr, SDHCI_PRESENT_STATE);
    printk("\nhost present status >>\n");
    printk("cmd line(cmd) : %s\n", uiSta & SDHCI_PSTA_CMD_INHIBIT ? "busy" : "free");
    printk("cmd line(dat) : %s\n", uiSta & SDHCI_PSTA_DATA_INHIBIT ? "busy" : "free");
    printk("dat line      : %s\n", uiSta & SDHCI_PSTA_DATA_ACTIVE ? "busy" : "free");
    printk("write active  : %s\n", uiSta & SDHCI_PSTA_DOING_WRITE ? "active" : "inactive");
    printk("read active   : %s\n", uiSta & SDHCI_PSTA_DOING_READ ? "active" : "inactive");
    printk("write buffer  : %s\n", uiSta & SDHCI_PSTA_SPACE_AVAILABLE ? "ready" : "not ready");
    printk("read  buffer  : %s\n", uiSta & SDHCI_PSTA_DATA_AVAILABLE ? "ready" : "not ready");
    printk("card insert   : %s\n", uiSta & SDHCI_PSTA_CARD_PRESENT ? "insert" : "not insert");
}
/*********************************************************************************************************
** ��������: __sdhciIntStaShow
** ��������: ��ʾ�ж�״̬
** ��    ��: lBasePoint
** ��    ��: NONE
** ��    ��: NONE
*********************************************************************************************************/
static VOID __sdhciIntStaShow(PLW_SDHCI_HOST_ATTR psdhcihostattr)
{
#ifndef printk
#define printk
#endif                                                                  /*  printk                      */

    UINT32  uiSta;

    uiSta = SDHCI_READL(psdhcihostattr, SDHCI_INT_STATUS);
    printk("\nhost int status >>\n");
    printk("cmd finish  : %s\n", uiSta & SDHCI_INT_RESPONSE ? "yes" : "no");
    printk("dat finish  : %s\n", uiSta & SDHCI_INT_DATA_END ? "yes" : "no");
    printk("dma finish  : %s\n", uiSta & SDHCI_INT_DMA_END ? "yes" : "no");
    printk("space avail : %s\n", uiSta & SDHCI_INT_SPACE_AVAIL ? "yes" : "no");
    printk("data avail  : %s\n", uiSta & SDHCI_INT_DATA_AVAIL ? "yes" : "no");
    printk("card insert : %s\n", uiSta & SDHCI_INT_CARD_INSERT ? "insert" : "not insert");
    printk("card remove : %s\n", uiSta & SDHCI_INT_CARD_REMOVE ? "remove" : "not remove");
}

#endif                                                                  /*  __SYLIXOS_DEBUG             */
#endif                                                                  /*  (LW_CFG_DEVICE_EN > 0)      */
                                                                        /*  (LW_CFG_SDCARD_EN > 0)      */
/*********************************************************************************************************
  END
*********************************************************************************************************/
