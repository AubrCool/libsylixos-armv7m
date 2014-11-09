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
** ��   ��   ��: sdiocoreLib.h
**
** ��   ��   ��: Zeng.Bo (����)
**
** �ļ���������: 2014 �� 10 �� 28 ��
**
** ��        ��: sdio ����������ӿ�ͷ�ļ�

** BUG:
*********************************************************************************************************/

#ifndef __SDIOCORE_LIB_H
#define __SDIOCORE_LIB_H

INT __sdioCoreDevReset(PLW_SDCORE_DEVICE   psdcoredev);
INT __sdioCoreDevSendIoOpCond(PLW_SDCORE_DEVICE   psdcoredev, UINT32 uiOcr, UINT32 *puiOcrOut);

INT __sdioCoreDevReadFbr(PLW_SDCORE_DEVICE   psdcoredev, SDIO_FUNC *psdiofunc);
INT __sdioCoreDevReadCCCR(PLW_SDCORE_DEVICE   psdcoredev, SDIO_CCCR *psdiocccr);

INT __sdioCoreDevReadCis(PLW_SDCORE_DEVICE   psdcoredev, SDIO_FUNC *psdiofunc);
INT __sdioCoreDevFuncClean(SDIO_FUNC *psdiofunc);

INT __sdioCoreDevHighSpeedEn(PLW_SDCORE_DEVICE   psdcoredev, SDIO_CCCR *psdiocccr);
INT __sdioCoreDevWideBusEn(PLW_SDCORE_DEVICE   psdcoredev, SDIO_CCCR *psdiocccr);

INT __sdioCoreDevReadByte(PLW_SDCORE_DEVICE   psdcoredev,
                          UINT32              uiFn,
                          UINT32              uiAddr,
                          UINT8              *pucByte);
INT __sdioCoreDevWriteByte(PLW_SDCORE_DEVICE   psdcoredev,
                           UINT32              uiFn,
                           UINT32              uiAddr,
                           UINT8               ucByte);
INT __sdioCoreDevWriteThenReadByte(PLW_SDCORE_DEVICE   psdcoredev,
                                   UINT32              uiFn,
                                   UINT32              uiAddr,
                                   UINT8               ucWrByte,
                                   UINT8              *pucRdByte);

INT __sdioCoreDevFuncEn(PLW_SDCORE_DEVICE   psdcoredev,
                        SDIO_FUNC          *psdiofunc);
INT __sdioCoreDevFuncDis(PLW_SDCORE_DEVICE   psdcoredev,
                         SDIO_FUNC          *psdiofunc);

INT __sdioCoreDevFuncBlkSzSet(PLW_SDCORE_DEVICE   psdcoredev,
                              SDIO_FUNC          *psdiofunc,
                              UINT32              uiBlkSz);

INT __sdioCoreDevRwDirect(PLW_SDCORE_DEVICE   psdcoredev,
                          BOOL                bWrite,
                          UINT32              uiFn,
                          UINT32              uiAddr,
                          UINT8               ucWrData,
                          UINT8              *pucRdBack);

INT __sdioCoreDevRwExtend(PLW_SDCORE_DEVICE   psdcoredev,
                          BOOL                bWrite,
                          UINT32              uiFn,
                          UINT32              uiAddr,
                          BOOL                bAddrInc,
                          UINT8              *pucBuf,
                          UINT32              uiBlkCnt,
                          UINT32              uiBlkSz);

INT __sdioCoreDevRwExtendX(PLW_SDCORE_DEVICE   psdcoredev,
                           BOOL                bWrite,
                           UINT32              uiFn,
                           BOOL                bIsBlkMode,
                           UINT32              uiAddr,
                           BOOL                bAddrInc,
                           UINT8              *pucBuf,
                           UINT32              uiBlkCnt,
                           UINT32              uiBlkSz);


#endif                                                              /*  __SDIOCORE_LIB_H                */
/*********************************************************************************************************
  END
*********************************************************************************************************/
