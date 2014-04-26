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
** ��   ��   ��: lwip_pppfd.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2014 �� 01 �� 08 ��
**
** ��        ��: lwip ppp ���ӹ�����.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
#include "../SylixOS/net/include/net_net.h"
/*********************************************************************************************************
  �ü�����
*********************************************************************************************************/
#if (LW_CFG_NET_EN > 0) && (LW_CFG_LWIP_PPP > 0)
#include "lwip/netif.h"
#include "lwip/pppapi.h"
#include "lwip_pppfd.h"
/*********************************************************************************************************
  PPP ��������
*********************************************************************************************************/
#define LW_PPPFD_BMSG_BUFSIZE   512
/*********************************************************************************************************
  PPP �豸�ṹ
*********************************************************************************************************/
typedef struct {
    LW_DEV_HDR          PD_devhdrHdr;                                   /*  �豸ͷ                      */
    LW_LIST_LINE        PD_lineManage;                                  /*  ���� PPP ��������           */
    ppp_pcb            *PD_pcb;                                         /*  PPP ���ƿ�                  */
    PLW_BMSG            PD_pbmsg;                                       /*  PPP �������׶���Ϣ        */
    INT                 PD_iFlag;
    INT                 PD_iSerial;                                     /*  PPPoS �����ļ�������        */
    LW_OBJECT_HANDLE    PD_ulInput;                                     /*  PPPoS �����߳�              */
    LW_OBJECT_HANDLE    PD_ulSignal;                                    /*  PPP �¼�ͬ��                */
} LW_PPPFD_DEV;
typedef LW_PPPFD_DEV   *PLW_PPPFD_DEV;
/*********************************************************************************************************
  ��������ȫ�ֱ���
*********************************************************************************************************/
static INT                  _G_iPppfdDrvNum = PX_ERROR;
static LW_LIST_LINE_HEADER  _G_plinePppfd;
static LW_OBJECT_HANDLE     _G_hPppfdSelMutex;
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
static LONG     _pppfdOpen(PLW_PPPFD_DEV   p_pppfddev, PCHAR  pcName, INT  iFlags, INT  iMode);
static INT      _pppfdClose(PLW_PPPFD_DEV  p_pppfddev);
static ssize_t  _pppfdRead(PLW_PPPFD_DEV   p_pppfddev, PCHAR  pcBuffer, size_t  stMaxBytes);
static INT      _pppfdIoctl(PLW_PPPFD_DEV  p_pppfddev, INT    iRequest, LONG  lArg);
/*********************************************************************************************************
  ������
*********************************************************************************************************/
#define LW_PPPFD_LOCK()             API_SemaphoreMPend(_G_hPppfdSelMutex, LW_OPTION_WAIT_INFINITE)
#define LW_PPPFD_UNLOCK()           API_SemaphoreMPost(_G_hPppfdSelMutex)
/*********************************************************************************************************
** ��������: API_PppfdDrvInstall
** ��������: ��װ pppfd �豸��������
** �䡡��  : NONE
** �䡡��  : �����Ƿ�װ�ɹ�
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
INT  API_PppfdDrvInstall (VOID)
{
    if (_G_iPppfdDrvNum == PX_ERROR) {
        _G_iPppfdDrvNum =  iosDrvInstall(LW_NULL,
                                         LW_NULL,
                                         _pppfdOpen,
                                         _pppfdClose,
                                         _pppfdRead,
                                         LW_NULL,
                                         _pppfdIoctl);
        DRIVER_LICENSE(_G_iPppfdDrvNum,     "Dual BSD/GPL->Ver 1.0");
        DRIVER_AUTHOR(_G_iPppfdDrvNum,      "Han.hui");
        DRIVER_DESCRIPTION(_G_iPppfdDrvNum, "pppfd driver.");
    }

    if (_G_hPppfdSelMutex == LW_OBJECT_HANDLE_INVALID) {
        _G_hPppfdSelMutex =  API_SemaphoreMCreate("pppfdsel_lock", LW_PRIO_DEF_CEILING,
                                                  LW_OPTION_WAIT_PRIORITY | LW_OPTION_DELETE_SAFE |
                                                  LW_OPTION_INHERIT_PRIORITY | LW_OPTION_OBJECT_GLOBAL,
                                                  LW_NULL);
    }

    return  ((_G_iPppfdDrvNum == (PX_ERROR)) ? (PX_ERROR) : (ERROR_NONE));
}
/*********************************************************************************************************
** ��������: __pppfd_status_cb
** ��������: PPP ����״̬�仯�ص�
** �䡡��  : pcb           ���ӿ��ƿ�
**           errcode       �������
**           p_pppfddev    PPP �����豸
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static void  __pppfd_status_cb (ppp_pcb *pcb, u8_t errcode, PLW_PPPFD_DEV  p_pppfddev)
{

}
/*********************************************************************************************************
** ��������: __pppfd_phase_cb
** ��������: PPP ���ӹ��̱仯�ص�
** �䡡��  : pcb           ���ӿ��ƿ�
**           phase         ���̱���
**           p_pppfddev    PPP �����豸
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static void  __pppfd_phase_cb (ppp_pcb *pcb, u8_t phase, PLW_PPPFD_DEV  p_pppfddev)
{

}
/*********************************************************************************************************
** ��������: pppfd_os_create
** ��������: ����һ�� PPPoS ����������
** �䡡��  : serial        ʹ�õĴ��нӿ��豸��
**           pppif         ��������ɹ�, �򷵻������豸����
**           bufsize       ifname ��������С
** �䡡��  : PPP ���������Ƿ�װ�ɹ�
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
int  pppfd_os_create (const char *serial, char *pppif, size_t bufsize)
{
    char            cDevName[32];
    PLW_PPPFD_DEV   p_pppfddev;
    INT             iFd;
    INT             iErrLevel = 0;

    if (!serial || !pppif || (bufsize < 4)) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    iFd = open(serial, O_RDWR);
    if (iFd < 0) {
        return  (PX_ERROR);
    }

    p_pppfddev = (PLW_PPPFD_DEV)__SHEAP_ALLOC(sizeof(LW_PPPFD_DEV));
    if (p_pppfddev == LW_NULL) {
        _ErrorHandle(ENOMEM);
        iErrLevel = 1;
        goto    __error_handle;
    }
    lib_bzero(p_pppfddev, sizeof(LW_PPPFD_DEV));

    p_pppfddev->PD_iSerial  = iFd;
    p_pppfddev->PD_ulSignal = API_SemaphoreBCreate("ppp_signal", LW_FALSE,
                                                   LW_OPTION_OBJECT_GLOBAL, LW_NULL);
    if (p_pppfddev->PD_ulSignal == LW_OBJECT_HANDLE_INVALID) {
        iErrLevel = 2;
        goto    __error_handle;
    }

    p_pppfddev->PD_pcb = pppapi_new();
    if (p_pppfddev->PD_pcb == LW_NULL) {
        _ErrorHandle(EMFILE);
        iErrLevel = 3;
        goto    __error_handle;
    }

    if (pppapi_over_serial_create(p_pppfddev->PD_pcb, (sio_fd_t)iFd,
                                  __pppfd_status_cb, (void *)p_pppfddev)) {
        _ErrorHandle(ENODEV);
        iErrLevel = 4;
        goto    __error_handle;
    }

    pppapi_set_notify_phase_callback(p_pppfddev->PD_pcb, __pppfd_phase_cb);

    snprintf(pppif, bufsize, "%c%c%d", p_pppfddev->PD_pcb->netif.name[0],
                                       p_pppfddev->PD_pcb->netif.name[1],
                                       p_pppfddev->PD_pcb->netif.num);
    snprintf(cDevName, sizeof(cDevName), "/dev/ppp/%s", pppif);

    if (iosDevAddEx(&p_pppfddev->PD_devhdrHdr, cDevName, _G_iPppfdDrvNum, DT_CHR) < ERROR_NONE) {
        iErrLevel = 5;
        goto    __error_handle;
    }

    ioctl(iFd, FIOSETOPTIONS, OPT_RAW);

    LW_PPPFD_LOCK();
    _List_Line_Add_Tail(&p_pppfddev->PD_lineManage, &_G_plinePppfd);
    LW_PPPFD_UNLOCK();

    return  (ERROR_NONE);

__error_handle:
    if (iErrLevel > 3) {
        pppapi_delete(p_pppfddev->PD_pcb);
    }
    if (iErrLevel > 2) {
        API_SemaphoreBDelete(&p_pppfddev->PD_ulSignal);
    }
    if (iErrLevel > 1) {
        __SHEAP_FREE(p_pppfddev);
    }
    if (iErrLevel > 0) {
        close(iFd);
    }

    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: pppfd_oe_create
** ��������: ����һ�� PPPoE ����������
** �䡡��  : ethif              ʹ�õ���̫����ӿ��� ����: en1
**           service_name       ��ʱδʹ��
**           concentrator_name  ��ʱδʹ��
**           pppif              ��������ɹ�, �򷵻������豸����
**           bufsize            ifname ��������С
** �䡡��  : PPP ���������Ƿ�װ�ɹ�
** ȫ�ֱ���:
** ����ģ��:
** ע  ��  : ���� LWIP PPP �������, ����Ӳ�����̫����ӿڵ����� PPPoE ���Ӽ�, ���������������ɾ������.

                                           API ����
*********************************************************************************************************/
LW_API
int  pppfd_oe_create (const char *ethif, const char *service_name, const char *concentrator_name,
                      char       *pppif, size_t      bufsize)
{
    char            cDevName[32];
    PLW_PPPFD_DEV   p_pppfddev;
    struct netif   *netif;
    INT             iErrLevel = 0;

    if (!ethif || !pppif || (bufsize < 4)) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    netif = netif_find((char *)ethif);
    if (netif == LW_NULL) {
        _ErrorHandle(ENODEV);
        return  (PX_ERROR);
    }

    p_pppfddev = (PLW_PPPFD_DEV)__SHEAP_ALLOC(sizeof(LW_PPPFD_DEV));
    if (p_pppfddev == LW_NULL) {
        _ErrorHandle(ENOMEM);
        iErrLevel = 1;
        goto    __error_handle;
    }
    lib_bzero(p_pppfddev, sizeof(LW_PPPFD_DEV));

    p_pppfddev->PD_iSerial  = PX_ERROR;
    p_pppfddev->PD_ulSignal = API_SemaphoreBCreate("ppp_signal", LW_FALSE,
                                                   LW_OPTION_OBJECT_GLOBAL, LW_NULL);
    if (p_pppfddev->PD_ulSignal == LW_OBJECT_HANDLE_INVALID) {
        iErrLevel = 2;
        goto    __error_handle;
    }

    p_pppfddev->PD_pcb = pppapi_new();
    if (p_pppfddev->PD_pcb == LW_NULL) {
        _ErrorHandle(EMFILE);
        iErrLevel = 3;
        goto    __error_handle;
    }

    if (pppapi_over_ethernet_create(p_pppfddev->PD_pcb, ethif, service_name,
                                    concentrator_name, __pppfd_status_cb, (void *)p_pppfddev)) {
        _ErrorHandle(ENODEV);
        iErrLevel = 4;
        goto    __error_handle;
    }

    pppapi_set_notify_phase_callback(p_pppfddev->PD_pcb, __pppfd_phase_cb);

    snprintf(pppif, bufsize, "%c%c%d", p_pppfddev->PD_pcb->netif.name[0],
                                           p_pppfddev->PD_pcb->netif.name[1],
                                           p_pppfddev->PD_pcb->netif.num);
    snprintf(cDevName, sizeof(cDevName), "/dev/ppp/%s", pppif);

    if (iosDevAddEx(&p_pppfddev->PD_devhdrHdr, cDevName, _G_iPppfdDrvNum, DT_CHR) < ERROR_NONE) {
        iErrLevel = 5;
        goto    __error_handle;
    }

    LW_PPPFD_LOCK();
    _List_Line_Add_Tail(&p_pppfddev->PD_lineManage, &_G_plinePppfd);
    LW_PPPFD_UNLOCK();

    return  (ERROR_NONE);

__error_handle:
    if (iErrLevel > 3) {
        pppapi_delete(p_pppfddev->PD_pcb);
    }
    if (iErrLevel > 2) {
        API_SemaphoreBDelete(&p_pppfddev->PD_ulSignal);
    }
    if (iErrLevel > 1) {
        __SHEAP_FREE(p_pppfddev);
    }

    return  (PX_ERROR);

}
/*********************************************************************************************************
** ��������: pppfd_ol2tp_create
** ��������: ����һ�� PPPoL2TP ����������
** �䡡��  : serial        ʹ�õĴ��нӿ��豸��
**           ipaddr        ������ IP ��ַ (*.*.*.* ��ʽ)
**           port          �������˿ں� (�����ֽ���)
**           secret        L2TP ����
**           secret_len    L2TP ���볤��
**           pppif         ��������ɹ�, �򷵻������豸����
**           bufsize       ifname ��������С
** �䡡��  : PPP ���������Ƿ�װ�ɹ�
** ȫ�ֱ���:
** ����ģ��:
** ע  ��  : ���� LWIP PPP �������, ����Ӳ�����̫����ӿڵ����� PPPoE ���Ӽ�, ���������������ɾ������.

                                           API ����
*********************************************************************************************************/
LW_API
int  pppfd_ol2tp_create (const char *ethif,
                         const char *ipaddr, short   port,
                         const char *secret, size_t  secret_len,
                         char       *pppif,  size_t  bufsize)
{
    char            cDevName[32];
    PLW_PPPFD_DEV   p_pppfddev;
    struct netif   *netif;
    ip_addr_t       ipaddr_l2tp;
    INT             iErrLevel = 0;

    if (!ethif || !pppif || (bufsize < 4)) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    if (ipaddr_aton(ipaddr, &ipaddr_l2tp) == 0) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    netif = netif_find((char *)ethif);
    if (netif == LW_NULL) {
        _ErrorHandle(ENODEV);
        return  (PX_ERROR);
    }

    p_pppfddev = (PLW_PPPFD_DEV)__SHEAP_ALLOC(sizeof(LW_PPPFD_DEV));
    if (p_pppfddev == LW_NULL) {
        _ErrorHandle(ENOMEM);
        iErrLevel = 1;
        goto    __error_handle;
    }
    lib_bzero(p_pppfddev, sizeof(LW_PPPFD_DEV));

    p_pppfddev->PD_iSerial  = PX_ERROR;
    p_pppfddev->PD_ulSignal = API_SemaphoreBCreate("ppp_signal", LW_FALSE,
                                                   LW_OPTION_OBJECT_GLOBAL, LW_NULL);
    if (p_pppfddev->PD_ulSignal == LW_OBJECT_HANDLE_INVALID) {
        iErrLevel = 2;
        goto    __error_handle;
    }

    p_pppfddev->PD_pcb = pppapi_new();
    if (p_pppfddev->PD_pcb == LW_NULL) {
        _ErrorHandle(EMFILE);
        iErrLevel = 3;
        goto    __error_handle;
    }

    if (pppapi_over_l2tp_create(p_pppfddev->PD_pcb, ethif, &ipaddr_l2tp, port,
                                secret, secret_len,
                                __pppfd_status_cb, (void *)p_pppfddev)) {
        _ErrorHandle(ENODEV);
        iErrLevel = 4;
        goto    __error_handle;
    }

    pppapi_set_notify_phase_callback(p_pppfddev->PD_pcb, __pppfd_phase_cb);

    snprintf(pppif, bufsize, "%c%c%d", p_pppfddev->PD_pcb->netif.name[0],
                                       p_pppfddev->PD_pcb->netif.name[1],
                                       p_pppfddev->PD_pcb->netif.num);
    snprintf(cDevName, sizeof(cDevName), "/dev/ppp/%s", pppif);

    if (iosDevAddEx(&p_pppfddev->PD_devhdrHdr, cDevName, _G_iPppfdDrvNum, DT_CHR) < ERROR_NONE) {
        iErrLevel = 5;
        goto    __error_handle;
    }

    LW_PPPFD_LOCK();
    _List_Line_Add_Tail(&p_pppfddev->PD_lineManage, &_G_plinePppfd);
    LW_PPPFD_UNLOCK();

    return  (ERROR_NONE);

__error_handle:
    if (iErrLevel > 3) {
        pppapi_delete(p_pppfddev->PD_pcb);
    }
    if (iErrLevel > 2) {
        API_SemaphoreBDelete(&p_pppfddev->PD_ulSignal);
    }
    if (iErrLevel > 1) {
        __SHEAP_FREE(p_pppfddev);
    }

    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: _pppfdOpen
** ��������: �� pppfd �豸
** �䡡��  : p_pppfddev    pppfd �豸
**           pcName        �豸����
**           iFlags        O_...
**           iMode         0666 0444 ...
** �䡡��  : pppfd �豸
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static LONG  _pppfdOpen (PLW_PPPFD_DEV   p_pppfddev, PCHAR  pcName, INT  iFlags, INT  iMode)
{
    if (LW_DEV_INC_USE_COUNT(&p_pppfddev->PD_devhdrHdr) == 1) {
        p_pppfddev->PD_pbmsg = _bmsgCreate(LW_PPPFD_BMSG_BUFSIZE);
        if (p_pppfddev->PD_pbmsg == LW_NULL) {
            LW_DEV_DEC_USE_COUNT(&p_pppfddev->PD_devhdrHdr);
            return  (PX_ERROR);
        }
        p_pppfddev->PD_iFlag = iFlags;
        return  ((LONG)p_pppfddev);

    } else {
        LW_DEV_DEC_USE_COUNT(&p_pppfddev->PD_devhdrHdr);
        _ErrorHandle(EBUSY);                                            /*  ֻ�����һ��              */
        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** ��������: _pppfdClose
** ��������: �ر�һ���Ѿ��򿪵� pppfd �豸
** �䡡��  : p_pppfddev    pppfd �豸
** �䡡��  : ERROR_NONE
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT  _pppfdClose (PLW_PPPFD_DEV  p_pppfddev)
{
    if (LW_DEV_GET_USE_COUNT(&p_pppfddev->PD_devhdrHdr)) {
        _bmsgDelete(p_pppfddev->PD_pbmsg);
        p_pppfddev->PD_pbmsg = LW_NULL;
        LW_DEV_DEC_USE_COUNT(&p_pppfddev->PD_devhdrHdr);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: _pppfdRead
** ��������: ��ȡ pppfd �豸��Ϣ
** �䡡��  : p_pppfddev    pppfd �豸
**           pcBuffer      ���ջ�����
**           stMaxBytes    ���ջ�������С
** �䡡��  : ERROR
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static ssize_t  _pppfdRead (PLW_PPPFD_DEV   p_pppfddev, PCHAR  pcBuffer, size_t  stMaxBytes)
{
    ULONG      ulErrCode;
    ULONG      ulTimeout;
    size_t     stMsgLen;
    ssize_t    sstRet;

    if (!pcBuffer || !stMaxBytes) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    if (p_pppfddev->PD_iFlag & O_NONBLOCK) {                            /*  ������ IO                   */
        ulTimeout = LW_OPTION_NOT_WAIT;
    } else {
        ulTimeout = LW_OPTION_WAIT_INFINITE;
    }

    for (;;) {
        LW_PPPFD_LOCK();
        stMsgLen = (size_t)_bmsgNBytesNext(p_pppfddev->PD_pbmsg);
        if (stMsgLen > stMaxBytes) {
            LW_PPPFD_UNLOCK();
            _ErrorHandle(EMSGSIZE);                                     /*  ������̫С                  */
            return  (PX_ERROR);

        } else if (stMsgLen) {
            break;                                                      /*  ���ݿɶ�                    */
        }
        LW_PPPFD_UNLOCK();

        ulErrCode = API_SemaphoreBPend(p_pppfddev->PD_ulSignal,         /*  �ȴ�������Ч                */
                                       ulTimeout);
        if (ulErrCode != ERROR_NONE) {                                  /*  ��ʱ                        */
            _ErrorHandle(EAGAIN);
            return  (0);
        }
    }

    sstRet = (ssize_t)_bmsgGet(p_pppfddev->PD_pbmsg, pcBuffer, stMaxBytes);

    LW_PPPFD_UNLOCK();

    return  (sstRet);
}
/*********************************************************************************************************
** ��������: _pppfdInput
** ��������: pppfd �豸����
** �䡡��  : p_pppfddev    pppfd �豸
**           iRequest      ��������
**           lArg          �������
** �䡡��  : ERROR
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static PVOID _pppfdInput (PLW_PPPFD_DEV  p_pppfddev)
{
    UINT8   ucBuffer[256];
    ssize_t sstNum;

    for (;;) {
        sstNum = read(p_pppfddev->PD_iSerial, ucBuffer, sizeof(ucBuffer));
        if (sstNum > 0) {
            pppos_input(p_pppfddev->PD_pcb, ucBuffer, (int)sstNum);

        } else {
            break;
        }
    }
}
/*********************************************************************************************************
** ��������: _pppfdDail
** ��������: pppfd �豸����
** �䡡��  : p_pppfddev    pppfd �豸
**           iRequest      ��������
**           lArg          �������
** �䡡��  : ERROR
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT  _pppfdDail (PLW_PPPFD_DEV  p_pppfddev)
{

}
/*********************************************************************************************************
** ��������: _pppfdIoctl
** ��������: ���� pppfd �豸
** �䡡��  : p_pppfddev    pppfd �豸
**           iRequest      ��������
**           lArg          �������
** �䡡��  : ERROR
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT  _pppfdIoctl (PLW_PPPFD_DEV  p_pppfddev, INT  iRequest, LONG  lArg)
{

}
#endif                                                                  /*  LW_CFG_NET_EN > 0           */
                                                                        /*  LW_CFG_LWIP_PPP > 0         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
