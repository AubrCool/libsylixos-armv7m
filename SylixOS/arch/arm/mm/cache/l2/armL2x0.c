/**********************************************************************************************************
**
**                                    �й������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                       SylixOS(TM)
**
**                               Copyright  All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: armL2x0.c
**
** ��   ��   ��: Jiao.Jinxing (������)
**
** �ļ���������: 2013 �� 12 �� 09 ��
**
** ��        ��: ARM ��ϵ���� L2 CACHE ������ PL210 / PL220 / PL310 ����.
** ע        ��: ���۴������Ǵ�˻���С��, L2 ��������ΪС��.
*********************************************************************************************************/
#define  __SYLIXOS_IO
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  �ü�����
*********************************************************************************************************/
#if LW_CFG_CACHE_EN > 0 && LW_CFG_ARM_CACHE_L2 > 0
#include "armL2.h"
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
static VOID armL2x0Sync(L2C_DRVIER  *pl2cdrv);
static VOID armL2x0InvalidateAll(L2C_DRVIER  *pl2cdrv);
static VOID armL2x0ClearAll(L2C_DRVIER  *pl2cdrv);
/*********************************************************************************************************
** ��������: armL2x0Enable
** ��������: ʹ�� L2 CACHE ������
** �䡡��  : pl2cdrv            �����ṹ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : ����� lockdown �������� unlock & invalidate �������� L2.
*********************************************************************************************************/
static VOID armL2x0Enable (L2C_DRVIER  *pl2cdrv)
{
    while (!(read32_le(L2C_BASE(pl2cdrv) + L2C_CTRL) & 0x01)) {
        write32_le(L2C_AUX(pl2cdrv), L2C_BASE(pl2cdrv) + L2C_AUX_CTRL); /*  ��Щ��������Ҫ�˲���        */
        armL2x0InvalidateAll(pl2cdrv);
        write32_le(0x01, L2C_BASE(pl2cdrv) + L2C_CTRL);
        armL2x0Sync(pl2cdrv);
    }
}
/*********************************************************************************************************
** ��������: armL2x0Disable
** ��������: ���� L2 CACHE ������
** �䡡��  : pl2cdrv            �����ṹ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID armL2x0Disable (L2C_DRVIER  *pl2cdrv)
{
    while (read32_le(L2C_BASE(pl2cdrv) + L2C_CTRL) & 0x01) {
        armL2x0ClearAll(pl2cdrv);
        write32_le(0x00, L2C_BASE(pl2cdrv) + L2C_CTRL);
    }
}
/*********************************************************************************************************
** ��������: armL2x0IsEnable
** ��������: ��� L2 CACHE �������Ƿ�ʹ��
** �䡡��  : pl2cdrv            �����ṹ
** �䡡��  : �Ƿ�ʹ��
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static BOOL armL2x0IsEnable (L2C_DRVIER  *pl2cdrv)
{
    return  (read32_le(L2C_BASE(pl2cdrv) + L2C_CTRL) & 0x01);
}
/*********************************************************************************************************
** ��������: armL2x0Sync
** ��������: L2 CACHE ������ͬ�� (������ PL310 ����Ҫ�ȴ�)
** �䡡��  : pl2cdrv            �����ṹ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID armL2x0Sync (L2C_DRVIER  *pl2cdrv)
{
    while (read32_le(L2C_BASE(pl2cdrv) + L2C_CACHE_SYNC) & 0x01);
}
/*********************************************************************************************************
** ��������: armPl310FlushAll
** ��������: L2 CACHE ��������д��������
** �䡡��  : pl2cdrv            �����ṹ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID armL2x0FlushAll (L2C_DRVIER  *pl2cdrv)
{
    write32_le(L2C_WAYMASK(pl2cdrv), L2C_BASE(pl2cdrv) + L2C_CLEAN_WAY);
    while (read32_le(L2C_BASE(pl2cdrv) + L2C_CLEAN_WAY));
    armL2x0Sync(pl2cdrv);
}
/*********************************************************************************************************
** ��������: armPl310InvalidateAll
** ��������: L2 CACHE ��������Ч��������
** �䡡��  : pl2cdrv            �����ṹ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID armL2x0InvalidateAll (L2C_DRVIER  *pl2cdrv)
{
    write32_le(L2C_WAYMASK(pl2cdrv), L2C_BASE(pl2cdrv) + L2C_INV_WAY);
    while (read32_le(L2C_BASE(pl2cdrv) + L2C_INV_WAY));
    armL2x0Sync(pl2cdrv);
}
/*********************************************************************************************************
** ��������: armPl310ClearAll
** ��������: L2 CACHE ��������д����Ч��������
** �䡡��  : pl2cdrv            �����ṹ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID armL2x0ClearAll (L2C_DRVIER  *pl2cdrv)
{
    write32_le(L2C_WAYMASK(pl2cdrv), L2C_BASE(pl2cdrv) + L2C_CLEAN_INV_WAY);
    while (read32_le(L2C_BASE(pl2cdrv) + L2C_CLEAN_INV_WAY));
    armL2x0Sync(pl2cdrv);
}
/*********************************************************************************************************
** ��������: armL2x0Init
** ��������: ��ʼ�� L2 CACHE ������
** �䡡��  : pl2cdrv            �����ṹ
**           uiInstruction      ָ�� CACHE ����
**           uiData             ���� CACHE ����
**           pcMachineName      ��������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID armL2x0Init (L2C_DRVIER  *pl2cdrv,
                  CACHE_MODE   uiInstruction,
                  CACHE_MODE   uiData,
                  CPCHAR       pcMachineName)
{
    pl2cdrv->L2CD_pfuncEnable        = armL2x0Enable;
    pl2cdrv->L2CD_pfuncDisable       = armL2x0Disable;
    pl2cdrv->L2CD_pfuncIsEnable      = armL2x0IsEnable;
    pl2cdrv->L2CD_pfuncSync          = armL2x0Sync;
    pl2cdrv->L2CD_pfuncFlush         = LW_NULL;
    pl2cdrv->L2CD_pfuncFlushAll      = armL2x0FlushAll;
    pl2cdrv->L2CD_pfuncInvalidate    = LW_NULL;
    pl2cdrv->L2CD_pfuncInvalidateAll = armL2x0InvalidateAll;
    pl2cdrv->L2CD_pfuncClear         = LW_NULL;
    pl2cdrv->L2CD_pfuncClearAll      = armL2x0ClearAll;
}

#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
                                                                        /*  LW_CFG_ARM_CACHE_L2 > 0     */
/*********************************************************************************************************
  END
*********************************************************************************************************/
