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
** ��   ��   ��: diskio.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2008 �� 09 �� 26 ��
**
** ��        ��: FAT �ļ�ϵͳ�� BLOCK �豸�ӿ�
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
#include "diskio.h"
/*********************************************************************************************************
  �ü���
*********************************************************************************************************/
#if LW_CFG_MAX_VOLUMES > 0
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
INT  __blockIoDevRead(INT     iIndex, 
                      VOID   *pvBuffer, 
                      ULONG   ulStartSector, 
                      ULONG   ulSectorCount);
INT  __blockIoDevWrite(INT     iIndex, 
                       VOID   *pvBuffer, 
                       ULONG   ulStartSector, 
                       ULONG   ulSectorCount);
INT  __blockIoDevReset(INT     iIndex);
INT  __blockIoDevStatus(INT     iIndex);
INT  __blockIoDevIoctl(INT  iIndex, INT  iCmd, LONG  lArg);
/*********************************************************************************************************
** ��������: disk_initialize
** ��������: ��ʼ�����豸
** �䡡��  : ucDriver      �����
** �䡡��  : VOID
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
DSTATUS  disk_initialize (BYTE  ucDriver)
{
    REGISTER INT    iError = __blockIoDevIoctl((INT)ucDriver, FIODISKINIT, 0);
    
    if (iError < 0) {
        return  ((BYTE)(STA_NOINIT | STA_NODISK));
    } else {
        return  ((BYTE)ERROR_NONE);
    }
}
/*********************************************************************************************************
** ��������: disk_status
** ��������: ��ÿ��豸״̬
** �䡡��  : ucDriver      �����
** �䡡��  : ERROR_NONE ��ʾ���豸��ǰ����, ������ʾ������.
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
DSTATUS  disk_status (BYTE  ucDriver)
{
    return  ((BYTE)__blockIoDevStatus((INT)ucDriver)); 
}
/*********************************************************************************************************
** ��������: disk_status
** ��������: ��һ�����豸
** �䡡��  : ucDriver          �����
**           ucBuffer          ������
**           dwSectorNumber    ��ʼ������
**           ucSectorCount     ��������
** �䡡��  : DRESULT
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
DRESULT disk_read (BYTE  ucDriver, BYTE  *ucBuffer, DWORD   dwSectorNumber, BYTE  ucSectorCount)
{
    REGISTER INT    iError;
    
    iError = __blockIoDevRead((INT)ucDriver, 
                              (PVOID)ucBuffer, 
                              (ULONG)dwSectorNumber,
                              (ULONG)ucSectorCount);
    if (iError >= ERROR_NONE) {
        return  (RES_OK);
    } else {
        return  (RES_ERROR);
    }
}
/*********************************************************************************************************
** ��������: disk_write
** ��������: дһ�����豸
** �䡡��  : ucDriver          �����
**           ucBuffer          ������
**           dwSectorNumber    ��ʼ������
**           ucSectorCount     ��������
** �䡡��  : DRESULT
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
DRESULT disk_write (BYTE  ucDriver, const BYTE  *ucBuffer, DWORD   dwSectorNumber, BYTE  ucSectorCount)
{
    REGISTER INT    iError;
    
    iError = __blockIoDevWrite((INT)ucDriver, 
                               (PVOID)ucBuffer, 
                               (ULONG)dwSectorNumber,
                               (ULONG)ucSectorCount);
    if (iError >= ERROR_NONE) {
        return  (RES_OK);
    } else {
        return  (RES_ERROR);
    }
}
/*********************************************************************************************************
** ��������: disk_write
** ��������: дһ�����豸
** �䡡��  : ucDriver          �����
**           ucCmd             ����
**           pvArg             ����
** �䡡��  : DRESULT
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
DRESULT  disk_ioctl (BYTE  ucDriver, BYTE ucCmd, void  *pvArg)
{
    REGISTER INT    iError;
    
    switch (ucCmd) {                                                    /*  ת������                    */
    
    case CTRL_SYNC:
        return  (RES_OK);                                               /*  ע��, Ŀǰ�����������      */
    
    case GET_SECTOR_COUNT:
        ucCmd = LW_BLKD_GET_SECNUM;
        break;
        
    case GET_SECTOR_SIZE:
        ucCmd = LW_BLKD_GET_SECSIZE;
        break;
        
    case GET_BLOCK_SIZE:
        ucCmd = LW_BLKD_GET_BLKSIZE;
        break;
        
    case CTRL_POWER:
        ucCmd = LW_BLKD_CTRL_POWER;
        break;
        
    case CTRL_LOCK:
        ucCmd = LW_BLKD_CTRL_LOCK;
        break;
        
    case CTRL_EJECT:
        ucCmd = LW_BLKD_CTRL_EJECT;
        break;
    }
    
    iError = __blockIoDevIoctl((INT)ucDriver, (INT)ucCmd, (LONG)pvArg);
    
    if (iError >= ERROR_NONE) {
        return  (RES_OK);
    } else {
        return  (RES_ERROR);
    }
}
/*********************************************************************************************************
** ��������: disk_timerproc
** ��������: I don't know!!!
** �䡡��  : ucDriver          �����
**           ucCmd             ����
**           pvArg             ����
** �䡡��  : DRESULT
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
void  disk_timerproc (void)
{
}
#endif                                                                  /*  LW_CFG_MAX_VOLUMES          */
/*********************************************************************************************************
  END
*********************************************************************************************************/

