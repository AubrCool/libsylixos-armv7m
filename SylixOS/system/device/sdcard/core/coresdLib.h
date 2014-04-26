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
** ��   ��   ��: coresdLib.h
**
** ��   ��   ��: Zeng.Bo (����)
**
** �ļ���������: 2010 �� 11 �� 23 ��
**
** ��        ��: sd����������ӿ�ͷ�ļ�

** BUG:
2010.11.27 �����˼���API.
2010.03.30 ����__sdCoreDevMmcSetRelativeAddr(),��֧��MMC��.
*********************************************************************************************************/

#ifndef __CORESD_LIB_H
#define __CORESD_LIB_H

INT __sdCoreDecodeCID(LW_SDDEV_CID  *psdcidDec, UINT32 *pRawCID, UINT8 ucType);
INT __sdCoreDecodeCSD(LW_SDDEV_CSD  *psdcsdDec, UINT32 *pRawCSD, UINT8 ucType);
INT __sdCoreDevReset(PLW_SDCORE_DEVICE psdcoredevice);
INT __sdCoreDevSendIfCond(PLW_SDCORE_DEVICE psdcoredevice);
INT __sdCoreDevSendAppOpCond(PLW_SDCORE_DEVICE  psdcoredevice,
                             UINT32             uiOCR,
                             LW_SDDEV_OCR      *puiOutOCR,
                             UINT8             *pucType);
INT __sdCoreDevSendRelativeAddr(PLW_SDCORE_DEVICE psdcoredevice, UINT32 *puiRCA);
INT __sdCoreDevSendAllCID(PLW_SDCORE_DEVICE psdcoredevice, LW_SDDEV_CID *psdcid);
INT __sdCoreDevSendAllCSD(PLW_SDCORE_DEVICE psdcoredevice, LW_SDDEV_CSD *psdcsd);
INT __sdCoreDevSelect(PLW_SDCORE_DEVICE psdcoredevice);
INT __sdCoreDevDeSelect(PLW_SDCORE_DEVICE psdcoredevice);
INT __sdCoreDevSetBusWidth(PLW_SDCORE_DEVICE psdcoredevice, INT iBusWidth);
INT __sdCoreDevSetBlkLen(PLW_SDCORE_DEVICE psdcoredevice, INT iBlkLen);
INT __sdCoreDevGetStatus(PLW_SDCORE_DEVICE psdcoredevice, UINT32 *puiStatus);
INT __sdCoreDevSetPreBlkLen(PLW_SDCORE_DEVICE psdcoredevice, INT iPreBlkLen);
INT __sdCoreDevIsBlockAddr(PLW_SDCORE_DEVICE psdcoredevice, BOOL *pbResult);

INT __sdCoreDevSpiClkDely(PLW_SDCORE_DEVICE psdcoredevice, INT iClkConts);
INT __sdCoreDevSpiCrcEn(PLW_SDCORE_DEVICE psdcoredevice, BOOL bEnable);

INT __sdCoreDevMmcSetRelativeAddr(PLW_SDCORE_DEVICE psdcoredevice, UINT32 uiRCA);

#endif                                                                  /*  __CORESD_LIB_H              */
/*********************************************************************************************************
  END
*********************************************************************************************************/
