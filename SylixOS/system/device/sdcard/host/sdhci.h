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
** �ļ���������: 2011 �� 01 �� 14 ��
**
** ��        ��: sd��׼������������ͷ�ļ�(SD Host Controller Simplified Specification Version 2.00).
**               ע�����мĴ������嶼Ϊ���ƫ�Ƶ�ַ.
**               ����ʹ�ñ�׼��������ƽ̨����,ʹ�ø�ģ����ȫ������sd bus��Ĳ���,��ֲ��������.

** BUG:
** 2011.03.02  �������ش���ģʽ�鿴\���ú���.����̬�ı䴫��ģʽ(�����ϲ������豸�������).
** 2011.04.12  �����������������ʱ��Ƶ����һ��.
** 2011.05.10  ���ǵ�SD�������ڲ�ͬƽ̨����Ĵ���������IO�ռ�,Ҳ�������ڴ�ռ�,���Զ�д�Ĵ�����6����������
**             Ϊ�ⲿ����,�ɾ���ƽ̨������ʵ��.
** 2011.06.01  ���ʼĴ�������Ӳ��ƽ̨��صĺ������ûص���ʽ,������ʵ��.
**             ��������һ��ͯ��,�������۵�ͯ��,��,һȥ������.ף�����µ�С��������׳�ɳ�,��������.
*********************************************************************************************************/

#ifndef SDHCI_H
#define SDHCI_H

/*********************************************************************************************************
  ����ü�֧��
*********************************************************************************************************/
#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_SDCARD_EN > 0)

/*********************************************************************************************************
   ϵͳSDMA��ַ�Ĵ���.
*********************************************************************************************************/

#define SDHCI_SYS_SDMA                  0x00                            /*  DMA����ʱָ��һ���ڴ��ַ   */

/*********************************************************************************************************
    ���С�Ĵ���.�üĴ�����������DMA����(bit 12 - 14)�еĵ�ַ�߽��һ�㴫���еĿ��С(bit 0 - 11)
*********************************************************************************************************/

#define SDHCI_BLOCK_SIZE                0x04
#define SDHCI_MAKE_BLKSZ(dma, blksz)    (((dma & 0x7) << 12) | (blksz & 0xfff))

/*********************************************************************************************************
    ������Ĵ���.
*********************************************************************************************************/

#define SDHCI_BLOCK_COUNT               0x06

/*********************************************************************************************************
    �����Ĵ���.
*********************************************************************************************************/

#define SDHCI_ARGUMENT                  0x08

/*********************************************************************************************************
    ����ģʽ�Ĵ���.
*********************************************************************************************************/

#define SDHCI_TRANSFER_MODE             0x0c
#define SDHCI_TRNS_DMA                  0x01
#define SDHCI_TRNS_BLK_CNT_EN           0x02
#define SDHCI_TRNS_ACMD12               0x04
#define SDHCI_TRNS_READ                 0x10
#define SDHCI_TRNS_MULTI                0x20

/*********************************************************************************************************
    ����Ĵ���.
*********************************************************************************************************/

#define SDHCI_COMMAND                   0x0e
#define SDHCI_CMD_CRC_CHK               0x08
#define SDHCI_CMD_INDEX_CHK             0x10
#define SDHCI_CMD_DATA                  0x20
#define SDHCI_CMD_RESP_TYPE_MASK        0x03
#define SDHCI_CMD_RESP_TYPE_NONE        0x00
#define SDHCI_CMD_RESP_TYPE_LONG        0x01
#define SDHCI_CMD_RESP_TYPE_SHORT       0x02
#define SDHCI_CMD_RESP_TYPE_SHORT_BUSY  0x03

#define SDHCI_MAKE_CMD(cmd, flg)        (((cmd & 0xff) << 8) | (flg & 0xff))

/*********************************************************************************************************
    ��Ӧ�Ĵ���.
*********************************************************************************************************/

#define SDHCI_RESPONSE0                 0x10
#define SDHCI_RESPONSE1                 0x14
#define SDHCI_RESPONSE2                 0x18
#define SDHCI_RESPONSE3                 0x1c

/*********************************************************************************************************
    ���ݻ���Ĵ���.
*********************************************************************************************************/

#define SDHCI_BUFFER                    0x20

/*********************************************************************************************************
    ��ǰ״̬�Ĵ���.
*********************************************************************************************************/

#define SDHCI_PRESENT_STATE             0x24
#define SDHCI_PSTA_CMD_INHIBIT          0x00000001
#define SDHCI_PSTA_DATA_INHIBIT         0x00000002
#define SDHCI_PSTA_DATA_ACTIVE          0x00000004
#define SDHCI_PSTA_DOING_WRITE          0x00000100
#define SDHCI_PSTA_DOING_READ           0x00000200
#define SDHCI_PSTA_SPACE_AVAILABLE      0x00000400
#define SDHCI_PSTA_DATA_AVAILABLE       0x00000800
#define SDHCI_PSTA_CARD_PRESENT         0x00010000
#define SDHCI_PSTA_WRITE_PROTECT        0x00080000

/*********************************************************************************************************
    ���������ƼĴ���.
*********************************************************************************************************/

#define SDHCI_HOST_CONTROL              0x28
#define SDHCI_HCTRL_LED                 0x01
#define SDHCI_HCTRL_4BITBUS             0x02
#define SDHCI_HCTRL_HISPD               0x04
#define SDHCI_HCTRL_DMA_MASK            0x18
#define SDHCI_HCTRL_SDMA                0x00
#define SDHCI_HCTRL_ADMA1               0x08
#define SDHCI_HCTRL_ADMA32              0x10
#define SDHCI_HCTRL_ADMA64              0x18
#define SDHCI_HCTRL_MMC_BITS8           0x20

/*********************************************************************************************************
    ��Դ���ƼĴ���.
*********************************************************************************************************/

#define SDHCI_POWER_CONTROL             0x29
#define SDHCI_POWCTL_ON                 0x01
#define SDHCI_POWCTL_180                0x0a
#define SDHCI_POWCTL_300                0x0c
#define SDHCI_POWCTL_330                0x0e

/*********************************************************************************************************
    �����Ĵ���.
*********************************************************************************************************/

#define SDHCI_BLOCK_GAP_CONTROL         0x2a
#define SDHCI_BLKGAP_STOP               0x01
#define SDHCI_BLKGAP_RESTART            0x02
#define SDHCI_BLKGAP_RWCTL_EN           0x04
#define SDHCI_BLKGAP_INT_EN             0x08

/*********************************************************************************************************
    ���ѿ��ƼĴ���.
*********************************************************************************************************/

#define SDHCI_WAKE_UP_CONTROL           0x2b
#define SDHCI_WAKUP_INT_EN              0x01
#define SDHCI_WAKUP_INSERT_EN           0x02
#define SDHCI_WAKUP_REMOVE_EN           0x04

/*********************************************************************************************************
    ʱ�ӿ��ƼĴ���.
*********************************************************************************************************/

#define SDHCI_CLOCK_CONTROL             0x2c
#define SDHCI_CLKCTL_DIVIDER_SHIFT      8
#define SDHCI_CLKCTL_CLOCK_EN           0x0004
#define SDHCI_CLKCTL_INTER_STABLE       0x0002
#define SDHCI_CLKCTL_INTER_EN           0x0001

/*********************************************************************************************************
   ��ʱ�������ƼĴ���.
*********************************************************************************************************/

#define SDHCI_TIMEOUT_CONTROL           0x2e

/*********************************************************************************************************
    �����λ�Ĵ���.
*********************************************************************************************************/

#define SDHCI_SOFTWARE_RESET            0x2f
#define SDHCI_SFRST_ALL                 0x01
#define SDHCI_SFRST_CMD                 0x02
#define SDHCI_SFRST_DATA                0x04

/*********************************************************************************************************
    һ���ж�״̬�Ĵ���.
*********************************************************************************************************/

#define SDHCI_INT_STATUS                0x30
#define SDHCI_INTSTA_CMD_END            0x0001
#define SDHCI_INTSTA_DATA_END           0x0002
#define SDHCI_INTSTA_BLKGAP_END         0x0004
#define SDHCI_INTSTA_DMA                0x0008
#define SDHCI_INTSTA_WRTBUF_RDY         0x0010
#define SDHCI_INTSTA_RDBUF_RDY          0x0020
#define SDHCI_INTSTA_CARD_INSERT        0x0040
#define SDHCI_INTSTA_CARD_REMOVE        0x0080
#define SDHCI_INTSTA_CARD_INT           0x0100
#define SDHCI_INTSTA_CCS_INT            0x0200
#define SDHCI_INTSTA_RDWAIT_INT         0x0400
#define SDHCI_INTSTA_FIFA0_INT          0x0800
#define SDHCI_INTSTA_FIFA1_INT          0x1000
#define SDHCI_INTSTA_FIFA2_INT          0x2000
#define SDHCI_INTSTA_FIFA3_INT          0x4000
#define SDHCI_INTSTA_INT_ERROR          0x8000

/*********************************************************************************************************
    �����ж�״̬�Ĵ���.
*********************************************************************************************************/

#define SDHCI_ERRINT_STATUS             0x32
#define SDHCI_EINTSTA_CMD_TIMOUT        0x0001
#define SDHCI_EINTSTA_CMD_CRC           0x0002
#define SDHCI_EINTSTA_CMD_ENDBIT        0x0004
#define SDHCI_EINTSTA_CMD_INDEX         0x0008
#define SDHCI_EINTSTA_DATA_TIMOUT       0x0010
#define SDHCI_EINTSTA_DATA_CRC          0x0020
#define SDHCI_EINTSTA_DATA_ENDBIT       0x0040
#define SDHCI_EINTSTA_CMD_CURRLIMIT     0x0080
#define SDHCI_EINTSTA_CMD_ACMD12        0x0100
#define SDHCI_EINTSTA_CMD_ADMA          0x0200


/*********************************************************************************************************
   һ���ж�״̬ʹ�ܼĴ���.
*********************************************************************************************************/

#define SDHCI_INTSTA_ENABLE             0x34
#define SDHCI_INTSTAEN_CMD              0x0001
#define SDHCI_INTSTAEN_DATA             0x0002
#define SDHCI_INTSTAEN_BLKGAP           0x0004
#define SDHCI_INTSTAEN_DMA              0x0008
#define SDHCI_INTSTAEN_WRTBUF           0x0010
#define SDHCI_INTSTAEN_RDBUF            0x0020
#define SDHCI_INTSTAEN_CARD_INSERT      0x0040
#define SDHCI_INTSTAEN_CARD_REMOVE      0x0080
#define SDHCI_INTSTAEN_CARD_INT         0x0100
#define SDHCI_INTSTAEN_CCS_INT          0x0200
#define SDHCI_INTSTAEN_RDWAIT           0x0400
#define SDHCI_INTSTAEN_FIFA0            0x0800
#define SDHCI_INTSTAEN_FIFA1            0x1000
#define SDHCI_INTSTAEN_FIFA2            0x2000
#define SDHCI_INTSTAEN_FIFA3            0x4000

/*********************************************************************************************************
   �����ж�״̬ʹ�ܼĴ���.
*********************************************************************************************************/

#define SDHCI_EINTSTA_ENABLE            0x36
#define SDHCI_EINTSTAEN_CMD_TIMEOUT     0x0001
#define SDHCI_EINTSTAEN_CMD_CRC         0x0002
#define SDHCI_EINTSTAEN_CMD_ENDBIT      0x0004
#define SDHCI_EINTSTAEN_CMD_INDEX       0x0008
#define SDHCI_EINTSTAEN_DATA_TIMEOUT    0x0010
#define SDHCI_EINTSTAEN_DATA_CRC        0x0020
#define SDHCI_EINTSTAEN_DATA_ENDBIT     0x0040
#define SDHCI_EINTSTAEN_CURR_LIMIT      0x0080
#define SDHCI_EINTSTAEN_ACMD12          0x0100
#define SDHCI_EINTSTAEN_ADMA            0x0200

/*********************************************************************************************************
  һ���ж��ź�ʹ�ܼĴ���.
*********************************************************************************************************/

#define SDHCI_SIGNAL_ENABLE             0x38
#define SDHCI_SIGEN_CMD_END             0x0001
#define SDHCI_SIGEN_DATA_END            0x0002
#define SDHCI_SIGEN_BLKGAP              0x0004
#define SDHCI_SIGEN_DMA                 0x0008
#define SDHCI_SIGEN_WRTBUF              0x0010
#define SDHCI_SIGEN_RDBUF               0x0020
#define SDHCI_SIGEN_CARD_INSERT         0x0040
#define SDHCI_SIGEN_CARD_REMOVE         0x0080
#define SDHCI_SIGEN_CARD_INT            0x0100

/*********************************************************************************************************
  �����ж��ź�ʹ�ܼĴ���.
*********************************************************************************************************/

#define SDHCI_ERRSIGNAL_ENABLE          0x3a
#define SDHCI_ESIGEN_CMD_TIMEOUT        0x0001
#define SDHCI_ESIGEN_CMD_CRC            0x0002
#define SDHCI_ESIGEN_CMD_ENDBIT         0x0004
#define SDHCI_ESIGEN_CMD_INDEX          0x0008
#define SDHCI_ESIGEN_DATA_TIMEOUT       0x0010
#define SDHCI_ESIGEN_DATA_CRC           0x0020
#define SDHCI_ESIGEN_DATA_ENDBIT        0x0040
#define SDHCI_ESIGEN_CURR_LIMIT         0x0080
#define SDHCI_ESIGEN_ACMD12             0x0100
#define SDHCI_ESIGEN_ADMA               0x0200

/*********************************************************************************************************
  following is for the case of 32-bit registers operation.
*********************************************************************************************************/

#define SDHCI_INT_RESPONSE              0x00000001
#define SDHCI_INT_DATA_END              0x00000002
#define SDHCI_INT_DMA_END               0x00000008
#define SDHCI_INT_SPACE_AVAIL           0x00000010
#define SDHCI_INT_DATA_AVAIL            0x00000020
#define SDHCI_INT_CARD_INSERT           0x00000040
#define SDHCI_INT_CARD_REMOVE           0x00000080
#define SDHCI_INT_CARD_INT              0x00000100
#define SDHCI_INT_ERROR                 0x00008000                      /*  normal                      */

#define SDHCI_INT_TIMEOUT               0x00010000
#define SDHCI_INT_CRC                   0x00020000
#define SDHCI_INT_END_BIT               0x00040000
#define SDHCI_INT_INDEX                 0x00080000
#define SDHCI_INT_DATA_TIMEOUT          0x00100000
#define SDHCI_INT_DATA_CRC              0x00200000
#define SDHCI_INT_DATA_END_BIT          0x00400000
#define SDHCI_INT_BUS_POWER             0x00800000
#define SDHCI_INT_ACMD12ERR             0x01000000
#define SDHCI_INT_ADMA_ERROR            0x02000000                      /*  error                       */

#define SDHCI_INT_NORMAL_MASK           0x00007fff
#define SDHCI_INT_ERROR_MASK            0xffff8000

#define SDHCI_INT_CMD_MASK              (SDHCI_INT_RESPONSE     | \
                                         SDHCI_INT_TIMEOUT      | \
                                         SDHCI_INT_CRC          | \
                                         SDHCI_INT_END_BIT      | \
                                         SDHCI_INT_INDEX)
#define SDHCI_INT_DATA_MASK             (SDHCI_INT_DATA_END     | \
                                         SDHCI_INT_DMA_END      | \
                                         SDHCI_INT_DATA_AVAIL   | \
                                         SDHCI_INT_SPACE_AVAIL  | \
                                         SDHCI_INT_DATA_TIMEOUT | \
                                         SDHCI_INT_DATA_CRC     | \
                                         SDHCI_INT_DATA_END_BIT | \
                                         SDHCI_INT_ADMA_ERROR)
#define SDHCI_INT_ALL_MASK              ((unsigned int)-1)

/*********************************************************************************************************
    �Զ�CMD12����״̬�Ĵ���.
*********************************************************************************************************/

#define SDHCI_ACMD12_ERR                0x3c
#define SDHCI_EACMD12_EXE               0x0001
#define SDHCI_EACMD12_TIMEOUT           0x0002
#define SDHCI_EACMD12_CRC               0x0004
#define SDHCI_EACMD12_ENDBIT            0x0008
#define SDHCI_EACMD12_INDEX             0x0010
#define SDHCI_EACMD12_CMDISSUE          0x0080

/*********************************************************************************************************
    ���ع��ܼĴ���.
*********************************************************************************************************/

#define SDHCI_CAPABILITIES              0x40
#define SDHCI_CAP_TIMEOUT_CLK_MASK      0x0000003f
#define SDHCI_CAP_TIMEOUT_CLK_SHIFT     0
#define SDHCI_CAP_TIMEOUT_CLK_UNIT      0x00000080
#define SDHCI_CAP_BASECLK_MASK          0x00003f00
#define SDHCI_CAP_BASECLK_SHIFT         8
#define SDHCI_CAP_MAXBLK_MASK           0x00030000
#define SDHCI_CAP_MAXBLK_SHIFT          16
#define SDHCI_CAP_CAN_DO_ADMA           0x00080000
#define SDHCI_CAP_CAN_DO_HISPD          0x00200000
#define SDHCI_CAP_CAN_DO_SDMA           0x00400000
#define SDHCI_CAP_CAN_DO_SUSRES         0x00800000
#define SDHCI_CAP_CAN_VDD_330           0x01000000
#define SDHCI_CAP_CAN_VDD_300           0x02000000
#define SDHCI_CAP_CAN_VDD_180           0x04000000
#define SDHCI_CAP_CAN_64BIT             0x10000000

/*********************************************************************************************************
    Maximum current capability register.
*********************************************************************************************************/

#define SDHCI_MAX_CURRENT               0x48                            /*  4c-4f reserved for more     */
                                                                        /*  for more max current        */

/*********************************************************************************************************
    ����(force set, �������������ж�)ACMD12����Ĵ���.
*********************************************************************************************************/

#define SDHCI_SET_ACMD12_ERROR          0x50

/*********************************************************************************************************
    ����(force set, �����������ж�)�жϴ���Ĵ���.
*********************************************************************************************************/

#define SDHCI_SET_INT_ERROR             0x52

/*********************************************************************************************************
    ADMA����״̬�Ĵ���.
*********************************************************************************************************/

#define SDHCI_ADMA_ERROR                0x54

/*********************************************************************************************************
   ADMA��ַ�Ĵ���.
*********************************************************************************************************/

#define SDHCI_ADMA_ADDRESS              0x58

/*********************************************************************************************************
    ����ж�״̬�Ĵ���.
*********************************************************************************************************/

#define SDHCI_SLOT_INT_STATUS           0xfc
#define SDHCI_SLOTINT_MASK              0x00ff
#define SDHCI_CHK_SLOTINT(slotsta, n)   ((slotsta & 0x00ff) | (1 << (n)))

/*********************************************************************************************************
    ���������汾�Ĵ���.
*********************************************************************************************************/

#define SDHCI_HOST_VERSION              0xfe
#define SDHCI_HVER_VENDOR_VER_MASK      0xff00
#define SDHCI_HVER_VENDOR_VER_SHIFT     8
#define SDHCI_HVER_SPEC_VER_MASK        0x00ff
#define SDHCI_HVER_SPEC_VER_SHIFT       0
#define SDHCI_HVER_SPEC_100             0x0000
#define SDHCI_HVER_SPEC_200             0x0001

/*********************************************************************************************************
    �����궨��.
*********************************************************************************************************/

#define SDHCI_DELAYMS(ms)                                   \
        do {                                                \
            ULONG   ulTimeout = LW_MSECOND_TO_TICK_1(ms);   \
            API_TimeSleep(ulTimeout);                       \
        } while (0)

/*********************************************************************************************************
    ���ش���ģʽ����ʹ�ò���.
*********************************************************************************************************/

#define SDHCIHOST_TMOD_SET_NORMAL       0
#define SDHCIHOST_TMOD_SET_SDMA         1
#define SDHCIHOST_TMOD_SET_ADMA         2

/*********************************************************************************************************
    ����֧�ֵĴ���ģʽ���λ�궨��.
*********************************************************************************************************/

#define SDHCIHOST_TMOD_CAN_NORMAL       (1 << 0)
#define SDHCIHOST_TMOD_CAN_SDMA         (1 << 1)
#define SDHCIHOST_TMOD_CAN_ADMA         (1 << 2)

/*********************************************************************************************************
  SD��׼�����������Խṹ
*********************************************************************************************************/

struct _sdhci_drv_funcs;                                                /*  ��׼����������������        */
typedef struct _sdhci_drv_funcs SDHCI_DRV_FUNCS;

typedef struct lw_sdhci_host_attr {
    SDHCI_DRV_FUNCS *SDHCIHOST_pDrvFuncs;                               /*  ��׼�������������ṹָ��    */
    LONG             SDHCIHOST_lBasePoint;                              /*  �ۻ���ַָ��.��������ñ��� */
                                                                        /*  �ر�ע��,��Ϊ�ڲ��޷������ */
                                                                        /*  �Ϸ���.                     */
    UINT32           SDHCIHOST_uiMaxClock;                              /*  ���������û���ڲ�ʱ��,�û� */
                                                                        /*  ��Ҫ�ṩʱ��Դ              */
} LW_SDHCI_HOST_ATTR, *PLW_SDHCI_HOST_ATTR;

/*********************************************************************************************************
  SD��׼���������ṹ��
*********************************************************************************************************/

struct _sdhci_drv_funcs {
    UINT32        (*sdhciReadL)
                  (
                  PLW_SDHCI_HOST_ATTR  phostattr,
                  LONG                 lReg
                  );
    UINT16        (*sdhciReadW)
                  (
                  PLW_SDHCI_HOST_ATTR  phostattr,
                  LONG                 lReg
                  );
    UINT8         (*sdhciReadB)
                  (
                  PLW_SDHCI_HOST_ATTR  phostattr,
                  LONG                 lReg
                  );
    VOID          (*sdhciWriteL)
                  (
                  PLW_SDHCI_HOST_ATTR  phostattr,
                  LONG                 lReg,
                  UINT32               uiLword
                  );
    VOID          (*sdhciWriteW)
                  (
                  PLW_SDHCI_HOST_ATTR  phostattr,
                  LONG                 lReg,
                  UINT16               usWord
                  );
    VOID          (*sdhciWriteB)
                  (
                  PLW_SDHCI_HOST_ATTR  phostattr,
                  LONG                 lReg,
                  UINT8                ucByte
                  );
};

#define SDHCI_READL(phostattr, lReg)           \
        ((phostattr)->SDHCIHOST_pDrvFuncs->sdhciReadL)(phostattr, lReg)
#define SDHCI_READW(phostattr, lReg)           \
        ((phostattr)->SDHCIHOST_pDrvFuncs->sdhciReadW)(phostattr, lReg)
#define SDHCI_READB(phostattr, lReg)           \
        ((phostattr)->SDHCIHOST_pDrvFuncs->sdhciReadB)(phostattr, lReg)

#define SDHCI_WRITEL(phostattr, lReg, uiLword) \
        ((phostattr)->SDHCIHOST_pDrvFuncs->sdhciWriteL)(phostattr, lReg, uiLword)
#define SDHCI_WRITEW(phostattr, lReg, usWord)  \
        ((phostattr)->SDHCIHOST_pDrvFuncs->sdhciWriteW)(phostattr, lReg, usWord)
#define SDHCI_WRITEB(phostattr, lReg, ucByte)  \
        ((phostattr)->SDHCIHOST_pDrvFuncs->sdhciWriteB)(phostattr, lReg, ucByte)
/*********************************************************************************************************
  SD��׼������������
*********************************************************************************************************/

LW_API PVOID      API_SdhciHostCreate(CPCHAR               pcAdapterName,
                                      PLW_SDHCI_HOST_ATTR  psdhcihostattr);
LW_API INT        API_SdhciHostDelete(PVOID            pvHost);

LW_API INT        API_SdhciHostTransferModGet(PVOID    pvHost);
LW_API INT        API_SdhciHostTransferModSet(PVOID    pvHost, INT   iTmod);

/*********************************************************************************************************
  SD��׼���������豸����
*********************************************************************************************************/

LW_API PVOID      API_SdhciDeviceAdd(PVOID            pvHost,
                                     CPCHAR           pcDeviceName);
LW_API INT        API_SdhciDeviceRemove(PVOID         pvDevice);

LW_API INT        API_SdhciDeviceUsageInc(PVOID       pvDevice);
LW_API INT        API_SdhciDeviceUsageDec(PVOID       pvDevice);
LW_API INT        API_SdhciDeviceUsageGet(PVOID       pvDevice);

#endif                                                                  /*  (LW_CFG_DEVICE_EN > 0)      */
                                                                        /*  (LW_CFG_SDCARD_EN > 0)      */
#endif                                                                  /*  SDHCI_H                     */
/*********************************************************************************************************
  END
*********************************************************************************************************/
