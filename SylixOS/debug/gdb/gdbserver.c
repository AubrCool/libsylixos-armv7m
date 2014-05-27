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
** ��   ��   ��: gdbserver.c
**
** ��   ��   ��: Jiang.Taijin (��̫��)
**
** �ļ���������: 2014 �� 05 �� 06 ��
**
** ��        ��: GDBServer ���س���
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
#include "spawn.h"
#include "dtrace.h"
#include "socket.h"
#include "sys/signalfd.h"
/*********************************************************************************************************
  �ü�֧��
*********************************************************************************************************/
#if LW_CFG_GDB_EN > 0
#include "arch/arch_gdb.h"
#include "../SylixOS/loader/elf/elf_type.h"
#include "../SylixOS/loader/include/loader.h"
#include "../SylixOS/loader/include/loader_vppatch.h"
/*********************************************************************************************************
  �����붨��
*********************************************************************************************************/
#define ERROR_GDB_INIT_SOCK         200000
#define ERROR_GDB_PARAM             200001
#define ERROR_GDB_ATTACH_PROG       200002
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
#define GDB_RSP_MAX_LEN             0x1000                              /* rsp ��������С               */
#define GDB_MAX_THREAD_NUM          LW_CFG_MAX_THREADS                  /* ����߳���                   */
/*********************************************************************************************************
  �ڴ����
*********************************************************************************************************/
#define LW_GDB_SAFEMALLOC(size)     __SHEAP_ALLOC((size_t)size)
#define LW_GDB_SAFEFREE(a)          { if (a) { __SHEAP_FREE((PVOID)a); a = 0; } }
/*********************************************************************************************************
  ��ӡ����
*********************************************************************************************************/
#define LW_GDB_MSG                   printf
/*********************************************************************************************************
  ͨ������
*********************************************************************************************************/
typedef enum {
    COMM_TYPE_TCP = 1,                                                  /* TCP                          */
    COMM_TYPE_TTY                                                       /* ����                         */
}LW_GDB_COMM_TYPE;
/*********************************************************************************************************
  �ϵ����ݽṹ
*********************************************************************************************************/
typedef struct {
    LW_LIST_LINE            BP_plistBpLine;                             /* �ϵ�����                     */
    addr_t                  BP_addr;                                    /* ��������                     */
    ULONG                   BP_ulInstOrg;                               /* �ϵ�����ԭָ��               */
} LW_GDB_BP;
/*********************************************************************************************************
  ȫ�ֲ����ṹ
*********************************************************************************************************/
typedef struct {
    BYTE                    GDB_byCommType;                             /* ��������                     */
    INT                     GDB_iCommFd;                                /* ���ڴ���Э��֡���ļ�������   */
    INT                     GDB_iSigFd;                                 /* �ж�signalfd�ļ����         */
    CHAR                    GDB_cProgPath[MAX_FILENAME_LENGTH];         /* ��ִ���ļ�·��               */
    INT                     GDB_iPid;                                   /* ���ٵĽ��̺�                 */
    PVOID                   GDB_pvDtrace;                               /* dtrace������               */
    LW_LIST_LINE_HEADER     GDB_plistBpHead;                            /* �ϵ�����                     */
    LW_GDB_BP               GDB_bpStep;                                 /* �����ϵ�                     */
    LONG                    GDB_lOpCThreadId;                           /* c,s�����߳�                  */
    LONG                    GDB_lOpGThreadId;                           /* ���������߳�                 */
    BOOL                    GDB_bNonStop;                               /* Non-stopģʽ                 */
} LW_GDB_PARAM;
/*********************************************************************************************************
  ȫ�ֱ�������
*********************************************************************************************************/
static const CHAR GcHexChars[] = "0123456789abcdef";                    /* hex->ascת����               */
/*********************************************************************************************************
** ��������: gdbTcpSockInit
** ��������: ��ʼ��socket
** �䡡��  : ui32Ip       ����ip
**           usPort       �����˿�
** �䡡��  : �ɹ�--fd��������ʧ��-- PX_ERROR.
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbTcpSockInit (UINT32 ui32Ip, UINT16 usPort)
{
    struct sockaddr_in  addrServer;
    struct sockaddr_in  addrClient;
    INT                 iSockListen;
    INT                 iSockNew;
    INT                 iOpt        = 1;
    socklen_t           iAddrLen    = sizeof(addrClient);
    CHAR                cIpBuff[32] = {0};

    bzero(&addrServer, sizeof(addrServer));
    if (0 == ui32Ip) {
        ui32Ip = INADDR_ANY;
    }
    
    addrServer.sin_family       = AF_INET;
    addrServer.sin_addr.s_addr  = htonl(ui32Ip);
    addrServer.sin_port         = htons(usPort);
    
    iSockListen = socket(PF_INET, SOCK_STREAM, 0);
    if (iSockListen < 0) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "Create listen sock failed.\r\n");
        return  (PX_ERROR);
    }

    setsockopt(iSockListen, SOL_SOCKET, SO_REUSEADDR, &iOpt, sizeof(iOpt));
    
    if (bind(iSockListen, (struct sockaddr*)&addrServer, sizeof(addrServer))) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "Bind sock failed.\r\n");
        close(iSockListen);
        return  (PX_ERROR);
    }

    if (listen(iSockListen, 1)) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "Bind sock failed.\r\n");
        close(iSockListen);
        return  (PX_ERROR);
    }
    
    LW_GDB_MSG("[GDB]Waiting for connect...\n");
    
    iSockNew = accept(iSockListen, (struct sockaddr *)&addrClient, &iAddrLen);
    if (iSockNew < 0) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "Accept failed.\r\n");
        close(iSockListen);
        return  (PX_ERROR);
    }

    setsockopt(iSockNew, IPPROTO_TCP, TCP_NODELAY, &iOpt, sizeof(iOpt));

    LW_GDB_MSG("[GDB]Connected. host : %s\n",
               inet_ntoa_r(addrClient.sin_addr, cIpBuff, sizeof(cIpBuff)));

    close(iSockListen);

    return  (iSockNew);
}
/*********************************************************************************************************
** ��������: gdbSerialInit
** ��������: ��ʼ�� tty
** �䡡��  : pcSerial     tty �豸��
** �䡡��  : �ɹ�--fd��������ʧ��-- PX_ERROR.
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbSerialInit (CPCHAR pcSerial)
{
#define SERIAL_PARAM    "115200,n,8,1"
#define SERIAL_BAUD     SIO_BAUD_115200
#define SERIAL_BSIZE    1024

    INT iFd;
    
    iFd = open(pcSerial, O_RDWR);
    if (iFd < 0) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "Open serial failed.\r\n");
        return  (PX_ERROR);
    }
    
    if (!isatty(iFd)) {
        close(iFd);
        _DebugHandle(__ERRORMESSAGE_LEVEL, "Serial is not a tty device.\r\n");
        return  (PX_ERROR);
    }
    
    ioctl(iFd, FIOSETOPTIONS,   OPT_RAW);
    ioctl(iFd, SIO_BAUD_SET,    SERIAL_BAUD);
    ioctl(iFd, SIO_HW_OPTS_SET, CLOCAL | CREAD | CS8 | HUPCL);
    ioctl(iFd, FIORBUFSET,      SERIAL_BSIZE);
    ioctl(iFd, FIOWBUFSET,      SERIAL_BSIZE);
    
    LW_GDB_MSG("[GDB]Serial device:%s %s\n", pcSerial, SERIAL_PARAM);
    
    return  (iFd);
}
/*********************************************************************************************************
** ��������: gdbAsc2Hex
** ��������: ASCת����hexֵ
** �䡡��  : ch       ASC�ַ�
** �䡡��  : hexֵ
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbAsc2Hex (CHAR ch)
{
    if ((ch >= 'a') && (ch <= 'f')) {
        return  (ch - 'a' + 10);
    }
    if ((ch >= '0') && (ch <= '9')) {
        return  (ch - '0');
    }
    if ((ch >= 'A') && (ch <= 'F')) {
        return  (ch - 'A' + 10);
    }

    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: gdbByte2Asc
** ��������: byteת����ASC�ַ���
** �䡡��  : iByte       byteֵ
** �䡡��  : pcAsc       ת�����ASC�ַ���
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static VOID gdbByte2Asc (PCHAR pcAsc, BYTE iByte)
{
    pcAsc[0] = GcHexChars[(iByte >> 4) & 0xf];
    pcAsc[1] = GcHexChars[iByte & 0xf];
}
/*********************************************************************************************************
** ��������: gdbAscToByte
** ��������: ASC�ַ���ת����byte�ֽ�
** �䡡��  : pcAsc       ASC�ַ���ֵ
** �䡡��  : ת�������ֵ
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbAscToByte (PCHAR pcAsc)
{
    return  (gdbAsc2Hex(pcAsc[0]) << 4) + gdbAsc2Hex(pcAsc[1]);
}
/*********************************************************************************************************
** ��������: gdbWord2Asc
** ��������: byteת����ASC�ַ���
** �䡡��  : iByte       byteֵ
** �䡡��  : pcAsc       ת�����ASC�ַ���
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static VOID gdbWord2Asc (PCHAR pcAsc, UINT32 ui32Word)
{
    INT i;

    for (i = 0; i < 4; i++) {
        gdbByte2Asc(pcAsc + i * 2, (ui32Word >> (i * 8)) & 0xFF);
    }
}
/*********************************************************************************************************
** ��������: gdbAscToWord
** ��������: ASC�ַ���ת����word�ֽ�
** �䡡��  : pcAsc       ASC�ַ���ֵ
** �䡡��  : ת�������ֵ
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static UINT32 gdbAscToWord (char *pcAsc)
{
    INT         i;
    UINT32      ui32Ret = 0;

    for (i = 3; i >= 0; i--) {
        ui32Ret = (ui32Ret << 8) + gdbAscToByte(pcAsc + i * 2);
    }
    
    return  (ui32Ret);
}
/*********************************************************************************************************
** ��������: gdbReplyError
** ��������: ��װrsp�����
** �䡡��  : pcReply       ���������
**           iErrNum       ������
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static VOID gdbReplyError (PCHAR pcReply, INT iErrNum)
{
    pcReply[0] = 'E';
    gdbByte2Asc(pcReply + 1, iErrNum);
    pcReply[3] = 0;
}
/*********************************************************************************************************
** ��������: gdbReplyOk
** ��������: ��װrsp ok��
** �䡡��  : pcReply       ���������
** �䡡��  :
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static VOID gdbReplyOk (PCHAR pcReply)
{
    lib_strcpy(pcReply, "OK");
}
/*********************************************************************************************************
** ��������: gdbRspPkgPut
** ��������: ����rsp���ݰ�
** �䡡��  : pparam       ϵͳ����
**           pcOutBuff    ���ͻ�����
** �䡡��  : ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbRspPkgPut (LW_GDB_PARAM *pparam, PCHAR pcOutBuff)
{
    CHAR    cHeader    = '$';
    UCHAR   ucCheckSum = 0;
    INT     iPkgLen    = 0;
    INT     iSendLen   = 0;
    CHAR    cAck;

    while (pcOutBuff[iPkgLen] != '\0') {
        ucCheckSum += pcOutBuff[iPkgLen];
        iPkgLen++;
    }

    if (iPkgLen > GDB_RSP_MAX_LEN - 10) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "Package Too long.\r\n");
        return  (PX_ERROR);
    }

    pcOutBuff[iPkgLen++] = '#';
    pcOutBuff[iPkgLen++] = GcHexChars[ucCheckSum >> 4];
    pcOutBuff[iPkgLen++] = GcHexChars[ucCheckSum % 16];

    do {
        if (write(pparam->GDB_iCommFd, &cHeader, 1) < 0) {
            return  (PX_ERROR);
        }

        iSendLen = write(pparam->GDB_iCommFd, pcOutBuff, iPkgLen);
        while (iSendLen > 0) {
            if (iSendLen <= 0) {
                return  (PX_ERROR);
            }
            iPkgLen -= iSendLen;
            if (iPkgLen <= 0) {
                break;
            }
            iSendLen = write(pparam->GDB_iCommFd, pcOutBuff, iPkgLen);
        }

        if (read(pparam->GDB_iCommFd, &cAck, 1) <= 0) {
            return  (PX_ERROR);
        }
    } while(cAck != '+');

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: gdbRspPkgGet
** ��������: ����rsp���ݰ�
** �䡡��  : pparam       ϵͳ����
**           pcInBuff     ���ܻ�����
** �䡡��  : ���ܵ������ݳ���
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbRspPkgGet (LW_GDB_PARAM *pparam, PCHAR pcInBuff)
{
    CHAR    cHeader;
    CHAR    cSend;
    INT     iReadLen    = 0;
    INT     iReadChkSum = 0;
    INT     iDataLen    = 0;
    UCHAR   ucCheckSum  = 0;
    UCHAR   ucXmitcSum  = 0;

    while (1) {
        iReadLen = read(pparam->GDB_iCommFd, &cHeader, 1);
        while (iReadLen > 0 ) {
            if ((cHeader & 0x7F) == '$') {
                break;
            }

            iReadLen = read(pparam->GDB_iCommFd, &cHeader, 1);
        }
        if (iReadLen < 0) {
            return  (PX_ERROR);
        }

        cSend = '+';
        iReadLen = read(pparam->GDB_iCommFd,
                        pcInBuff + iDataLen,
                        GDB_RSP_MAX_LEN - iDataLen);

        while (iReadLen > 0) {
            while (iReadLen > 0) {
                if ((pcInBuff[iDataLen] & 0x7F) == '#') {
                    while (iReadLen < 3 && (GDB_RSP_MAX_LEN - iDataLen) > 3) {
                        iReadChkSum = read(pparam->GDB_iCommFd,
                                           pcInBuff + iDataLen + iReadLen,
                                           2 - iReadLen);
                        if (iReadChkSum <= 0) {
                            return  (PX_ERROR);
                        }
                        iReadLen += iReadChkSum;
                    }
                    ucXmitcSum  = gdbAsc2Hex(pcInBuff[iDataLen + 1] & 0x7f) << 4;
                    ucXmitcSum += gdbAsc2Hex(pcInBuff[iDataLen + 2] & 0x7f);

                    if (ucXmitcSum == ucCheckSum) {
                        cSend = '+';
                        write(pparam->GDB_iCommFd, &cSend, sizeof(cSend));
                        return iDataLen;
                    } else {
                        cSend = '-';
                        write(pparam->GDB_iCommFd, &cSend, sizeof(cSend));
                        break;
                    }

                } else {
                    ucCheckSum += (pcInBuff[iDataLen] & 0x7F);
                }
                iDataLen++;
                iReadLen--;
            }

            if (cSend == '-') {
                break;
            }

            iReadLen = read(pparam->GDB_iCommFd,
                            pcInBuff + iDataLen,
                            GDB_RSP_MAX_LEN - iDataLen);
        }

        if (iReadLen <= 0) {
            return  (PX_ERROR);
        }
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: gdbRemoveBP
** ��������: �Ƴ��ϵ�
** �䡡��  : cInBuff       rsp��������ַ�ͳ���
** �䡡��  : cOutBuff      rsp�������ڴ�����
**           ����ֵ        ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbRemoveBP (LW_GDB_PARAM *pparam, PCHAR pcInBuff, PCHAR pcOutBuff)
{
    INT                 iType;
    addr_t              addrBp = 0;
    UINT                uiLen  = 0;
    PLW_LIST_LINE       plineTemp;
    LW_GDB_BP          *pbpItem;

    if (sscanf(pcInBuff, "%d,%lx,%x",
               &iType, (ULONG *)&addrBp, &uiLen) != 3) {
        gdbReplyError(pcOutBuff, 0);
        return  (ERROR_NONE);
    }

    plineTemp = pparam->GDB_plistBpHead;                                /* �ͷŶϵ�����                 */
    while (plineTemp) {
        pbpItem  = _LIST_ENTRY(plineTemp, LW_GDB_BP, BP_plistBpLine);
        plineTemp = _list_line_get_next(plineTemp);
        if (addrBp == pbpItem->BP_addr) {                               /* �ϵ��Ѵ���                   */
            if (API_DtraceBreakpointRemove(pparam->GDB_pvDtrace,
                                           addrBp,
                                           pbpItem->BP_ulInstOrg) != ERROR_NONE) {
                gdbReplyError(pcOutBuff, 0);
                return  (ERROR_NONE);
            }
            _List_Line_Del(&pbpItem->BP_plistBpLine, &pparam->GDB_plistBpHead);
            LW_GDB_SAFEFREE(pbpItem);
            return  (ERROR_NONE);
        }
    }

    gdbReplyOk(pcOutBuff);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: gdbInsertBP
** ��������: �Ƴ��ϵ�
** �䡡��  : cInBuff       �Ĵ���������rsp����ʽ
** �䡡��  : cOutBuff      �Ĵ���ֵ��rsp����ʽ
**           ����ֵ        ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbInsertBP (LW_GDB_PARAM *pparam, PCHAR pcInBuff, PCHAR pcOutBuff)
{
    INT                 iType;
    addr_t              addrBp = 0;
    UINT                uiLen  = 0;
    PLW_LIST_LINE       plineTemp;
    LW_GDB_BP          *pbpItem;

    if (sscanf(pcInBuff, "%d,%lx,%x", &iType,
               (ULONG *)&addrBp, &uiLen) != 3) {
        gdbReplyError(pcOutBuff, 0);
        return  (ERROR_NONE);
    }

    plineTemp = pparam->GDB_plistBpHead;                                /* �ͷŶϵ�����                 */
    while (plineTemp) {
        pbpItem  = _LIST_ENTRY(plineTemp, LW_GDB_BP, BP_plistBpLine);
        plineTemp = _list_line_get_next(plineTemp);
        if (addrBp == pbpItem->BP_addr) {                               /* �ϵ��Ѵ���                   */
            return  (ERROR_NONE);
        }
    }

    pbpItem = (LW_GDB_BP*)LW_GDB_SAFEMALLOC(sizeof(LW_GDB_BP));
    if (LW_NULL == pbpItem) {
        gdbReplyError(pcOutBuff, 0);
        return  (ERROR_NONE);
    }

    pbpItem->BP_addr = addrBp;
    if (API_DtraceBreakpointInsert(pparam->GDB_pvDtrace,
                                   addrBp,
                                   &pbpItem->BP_ulInstOrg) != ERROR_NONE) {
        LW_GDB_SAFEFREE(pbpItem);
        gdbReplyError(pcOutBuff, 0);
        return  (ERROR_NONE);
    }

    _List_Line_Add_Ahead(&pbpItem->BP_plistBpLine, 
                         &pparam->GDB_plistBpHead);                     /*  ����ϵ�����                */

    gdbReplyOk(pcOutBuff);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: gdbRegGet
** ��������: ��ȡ�Ĵ���ֵ
** �䡡��  : iTid          �߳�id
**           cInBuff       �Ĵ���������rsp����ʽ
** �䡡��  : cOutBuff      �Ĵ���ֵ��rsp����ʽ
**           ����ֵ        ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbRegsGet (LW_GDB_PARAM     *pparam,
                       PLW_DTRACE_MSG    pdmsg,
                       PCHAR             pcInBuff,
                       PCHAR             pcOutBuff)
{
    INT         i;
    PCHAR       pcPos;
    GDB_REG_SET regset;
    ULONG       ulThreadId;

    if ((pparam->GDB_lOpGThreadId == 0) || (pparam->GDB_lOpGThreadId == -1)) {
        ulThreadId = pdmsg->DTM_ulThread;
    } else {
        ulThreadId = pparam->GDB_lOpGThreadId;
    }

    pcPos = pcOutBuff;

    archGdbRegsGet(pparam->GDB_pvDtrace, ulThreadId, &regset);

    for (i = 0; i < regset.GDBR_iRegCnt; i++) {
        gdbWord2Asc(pcPos, regset.regArr[i].GDBRA_ulValue);
        pcPos += 8;
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: gdbRegSet
** ��������: ���üĴ���ֵ
** �䡡��  : iTid          �߳�id
**           cInBuff       �Ĵ���������rsp����ʽ
** �䡡��  : cOutBuff      �Ĵ���ֵ��rsp����ʽ
**           ����ֵ        ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbRegsSet (LW_GDB_PARAM     *pparam,
                       PLW_DTRACE_MSG    pdmsg,
                       PCHAR             pcInBuff,
                       PCHAR             pcOutBuff)
{
    PCHAR       pcPos;
    INT         i;
    INT         iLen;
    GDB_REG_SET regset;
    ULONG       ulThreadId;

    if ((pparam->GDB_lOpGThreadId == 0) || (pparam->GDB_lOpGThreadId == -1)) {
        ulThreadId = pdmsg->DTM_ulThread;
    } else {
        ulThreadId = pparam->GDB_lOpGThreadId;
    }

    iLen = lib_strlen(pcInBuff);

    archGdbRegsGet(pparam->GDB_pvDtrace, ulThreadId, &regset);

    pcPos = pcInBuff;
    for (i = 0; i < regset.GDBR_iRegCnt && iLen >= (i + 1) * 8; i++) {
        regset.regArr[i].GDBRA_ulValue = gdbAscToWord(pcPos);
        pcPos += 8;
    }

    archGdbRegsSet(pparam->GDB_pvDtrace, ulThreadId, &regset);

    gdbReplyOk(pcOutBuff);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
 ** ��������: gdbRegGet
 ** ��������: ��ȡ�Ĵ���ֵ
 ** �䡡��  : iTid          �߳�id
 **           cInBuff       �Ĵ���������rsp����ʽ
 ** �䡡��  : cOutBuff      �Ĵ���ֵ��rsp����ʽ
 **           ����ֵ        ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����
 ** ȫ�ֱ���:
 ** ����ģ��:
 *********************************************************************************************************/
static INT gdbRegGet (LW_GDB_PARAM      *pparam,
                      PLW_DTRACE_MSG     pdmsg,
                      PCHAR              pcInBuff,
                      PCHAR              pcOutBuff)
 {
     GDB_REG_SET regset;
     INT         iRegIdx;
     ULONG       ulThreadId;

     if ((pparam->GDB_lOpGThreadId == 0) || (pparam->GDB_lOpGThreadId == -1)) {
         ulThreadId = pdmsg->DTM_ulThread;
     } else {
         ulThreadId = pparam->GDB_lOpGThreadId;
     }

     if (sscanf(pcInBuff, "%x", &iRegIdx) != 1) {
         gdbReplyError(pcOutBuff, 0);
         return  (PX_ERROR);
     }

     archGdbRegsGet(pparam->GDB_pvDtrace, ulThreadId, &regset);

     gdbWord2Asc(pcOutBuff, regset.regArr[iRegIdx].GDBRA_ulValue);

     return  (ERROR_NONE);
 }
/*********************************************************************************************************
** ��������: gdbRegSet
** ��������: ���üĴ���ֵ
** �䡡��  : iTid          �߳�id
**           cInBuff       �Ĵ���������rsp����ʽ
** �䡡��  : cOutBuff      �Ĵ���ֵ��rsp����ʽ
**           ����ֵ        ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbRegSet (LW_GDB_PARAM      *pparam,
                      PLW_DTRACE_MSG     pdmsg,
                      PCHAR              pcInBuff,
                      PCHAR              pcOutBuff)
 {
     GDB_REG_SET regset;
     INT         iRegIdx;
     INT         iPos;
     ULONG       ulThreadId;

     if ((pparam->GDB_lOpGThreadId == 0) || (pparam->GDB_lOpGThreadId == -1)) {
         ulThreadId = pdmsg->DTM_ulThread;
     } else {
         ulThreadId = pparam->GDB_lOpGThreadId;
     }

     iPos = -1;
     sscanf(pcInBuff, "%x=%n", &iRegIdx, &iPos);
     if (iPos == -1) {
         gdbReplyError(pcOutBuff, 0);
        return  (PX_ERROR);
     }

     archGdbRegsGet(pparam->GDB_pvDtrace, ulThreadId, &regset);

     regset.regArr[iRegIdx].GDBRA_ulValue = gdbAscToWord(pcInBuff + iPos);

     archGdbRegsSet(pparam->GDB_pvDtrace, ulThreadId, &regset);

     return  (ERROR_NONE);
 }
/*********************************************************************************************************
** ��������: gdbMemGet
** ��������: ���üĴ���ֵ
** �䡡��  : cInBuff       rsp��������ַ�ͳ���
** �䡡��  : cOutBuff      rsp�������ڴ�����
**           ����ֵ        ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbMemGet (LW_GDB_PARAM *pparam, PCHAR pcInBuff, PCHAR pcOutBuff)
{
    UINT    uiAddr  = 0;
    UINT    uiLen   = 0;
    INT     i;
    PBYTE   pbyBuff = NULL;

    if (sscanf(pcInBuff, "%x,%x", &uiAddr, &uiLen) != 2) {
        gdbReplyError(pcOutBuff, 0);
        return  (ERROR_NONE);
    }

    if (uiLen > (GDB_RSP_MAX_LEN - 16) / 2) {
        gdbReplyError(pcOutBuff, 1);
        return  (ERROR_NONE);
    }

    pbyBuff  = (PBYTE)LW_GDB_SAFEMALLOC(uiLen);                         /* �����ڴ�                     */
    if (NULL == pbyBuff) {
        gdbReplyError(pcOutBuff, 20);
        return  (PX_ERROR);
    }

    if (API_DtraceGetMems(pparam->GDB_pvDtrace, uiAddr,
                          pbyBuff, uiLen) != (ERROR_NONE)) {            /* ���ڴ�                       */
        LW_GDB_SAFEFREE(pbyBuff);
        gdbReplyError(pcOutBuff, 20);
        return  (ERROR_NONE);
    }

    for (i = 0; i < uiLen; i++) {
        gdbByte2Asc(pcOutBuff + i * 2, pbyBuff[i]);
    }

    LW_GDB_SAFEFREE(pbyBuff);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: gdbRegSet
** ��������: ���üĴ���ֵ
** �䡡��  : cInBuff       rsp��������ַ�ͳ���
** �䡡��  : cOutBuff      rsp�������ڴ�����
**           ����ֵ        ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbMemSet (LW_GDB_PARAM *pparam, PCHAR pcInBuff, PCHAR pcOutBuff)
{
    UINT    uiAddr  = 0;
    UINT    uiLen   = 0;
    INT     iPos;
    INT     i;
    PBYTE   pbyBuff = NULL;

    if ((sscanf(pcInBuff, "%x,%x:%n", &uiAddr, &uiLen, &iPos) < 2) || (iPos == -1)) {
        gdbReplyError(pcOutBuff, 0);
        return  (ERROR_NONE);
    }

    pbyBuff  = (PBYTE)LW_GDB_SAFEMALLOC(uiLen);                         /* �����ڴ�                     */
    if (NULL == pbyBuff) {
        gdbReplyError(pcOutBuff, 20);
        return  (PX_ERROR);
    }

    for (i = 0; i < uiLen; i++) {
        pbyBuff[i] = gdbAscToByte(pcInBuff + iPos + i * 2);
    }

    if (API_DtraceSetMems(pparam->GDB_pvDtrace, uiAddr,
                          pbyBuff, uiLen) != (ERROR_NONE)) {            /* д�ڴ�                       */
        LW_GDB_SAFEFREE(pbyBuff);
        gdbReplyError(pcOutBuff, 20);
        return  (ERROR_NONE);
    }

    gdbReplyOk(pcOutBuff);

    LW_GDB_SAFEFREE(pbyBuff);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: gdbGoTo
** ��������: �ָ������Գ���ִ��
** �䡡��  : pparam        gdbȫ�ֲ���
**           iTid          �߳�id
**           cInBuff       rsp��������ʼִ�еĵ�ַ
** �䡡��  : cOutBuff      rsp������ʾ����ִ�н��
**           ����ֵ        ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbGoTo (LW_GDB_PARAM    *pparam,
                    PLW_DTRACE_MSG   pdmsg,
                    PCHAR            pcInBuff,
                    PCHAR            pcOutBuff,
                    INT              iBeStep)
{
    INT         iSig;
    ULONG       ulAddr = 0;
    GDB_REG_SET regset;

    if (sscanf(pcInBuff, "%x;%lx", &iSig, &ulAddr) != 2) {
        sscanf(pcInBuff, "%lx", &ulAddr);
    }

    if (ulAddr != 0) {
        archGdbRegSetPc(pparam->GDB_pvDtrace, pdmsg->DTM_ulThread, ulAddr);
    }

    if (iBeStep) {
        archGdbRegsGet(pparam->GDB_pvDtrace, pdmsg->DTM_ulThread, &regset);
        pparam->GDB_bpStep.BP_addr = archGdbGetNextPc(&regset);
        API_DtraceBreakpointInsert(pparam->GDB_pvDtrace,
                                   pparam->GDB_bpStep.BP_addr,
                                   &pparam->GDB_bpStep.BP_ulInstOrg);
    }

    gdbReplyOk(pcOutBuff);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: gdbGetElfOffset
** ��������: ��ȡ�����ض�λ��ַ
** �䡡��  : pid           ����id
**           pcModPath     ģ��·��
** �䡡��  : addrText      text�ε�ַ
**           addrData      data�ε�ַ
**           addrBss       bss�ε�ַ
**           ����ֵ        ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbGetElfOffset (pid_t   pid,
                            PCHAR   pcModPath,
                            addr_t *addrText,
                            addr_t *addrData,
                            addr_t *addrBss)
{
    addr_t          addrBase;
    INT             iFd;
    INT             i;
    INT             iReadLen;

    Elf_Ehdr        ehdr;
    Elf_Shdr       *pshdr;
    size_t          stHdSize;
    PCHAR           pcBuf    = NULL;
    PCHAR           pcShName = NULL;

    if (API_ModuleGetBase(pid, pcModPath, &addrBase) != (ERROR_NONE)) {
        return  (PX_ERROR);
    }

    iFd = open(pcModPath, O_RDONLY);
    if (iFd < 0) {                                                      /*  ���ļ�                    */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "Open file error.\r\n");
        return  (PX_ERROR);
    }

    if (read(iFd, &ehdr, sizeof(ehdr)) < sizeof(ehdr)) {                /*  ��ȡELFͷ                   */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "Read elf header error.\r\n");
        return  (PX_ERROR);
    }

    if (ehdr.e_type == ET_REL) {
        stHdSize = ehdr.e_shentsize * ehdr.e_shnum;

        pcBuf = (PCHAR)LW_GDB_SAFEMALLOC(stHdSize);
        if (pcBuf == LW_NULL) {
            close(iFd);
            return  (PX_ERROR);
        }

        if (lseek(iFd, ehdr.e_shoff, SEEK_SET) < 0) {
            close(iFd);
            LW_GDB_SAFEFREE(pcBuf);
            return  (PX_ERROR);
        }

        if (read(iFd, pcBuf, stHdSize) < stHdSize) {
            close(iFd);
            LW_GDB_SAFEFREE(pcBuf);
            return  (PX_ERROR);
        }

        pshdr = (Elf_Shdr *)pcBuf;

        pcShName = (PCHAR)LW_GDB_SAFEMALLOC(pshdr[ehdr.e_shstrndx].sh_size);
        if (pcBuf == LW_NULL) {
            close(iFd);
            LW_GDB_SAFEFREE(pcBuf);
            return  (PX_ERROR);
        }

        if (lseek(iFd, pshdr[ehdr.e_shstrndx].sh_offset, SEEK_SET) < 0) {
            close(iFd);
            LW_GDB_SAFEFREE(pcBuf);
            LW_GDB_SAFEFREE(pcShName);
            return  (PX_ERROR);
        }

        iReadLen = read(iFd, pcShName, pshdr[ehdr.e_shstrndx].sh_size);
        if (iReadLen < pshdr[ehdr.e_shstrndx].sh_size) {
            close(iFd);
            LW_GDB_SAFEFREE(pcBuf);
            LW_GDB_SAFEFREE(pcShName);
            return  (PX_ERROR);
        }

        for (i = 0; i < ehdr.e_shnum; i++) {
            if (lib_strcmp(pcShName + pshdr[i].sh_name, ".text") == 0) {
                (*addrText) = addrBase + pshdr[i].sh_addr;
            }

            if (lib_strcmp(pcShName + pshdr[i].sh_name, ".data") == 0) {
                (*addrData) = addrBase + pshdr[i].sh_addr;
            }

            if (strcmp(pcShName + pshdr[i].sh_name, ".bss") == 0) {
                (*addrBss) = addrBase + pshdr[i].sh_addr;
            }
        }
    } else {                                                            /* so�ļ�ȫ�����ó�ģ�����ַ   */
        (*addrText) = addrBase;
        (*addrData) = addrBase;
        (*addrBss)  = addrBase;
    }

    close(iFd);
    LW_GDB_SAFEFREE(pcBuf);
    LW_GDB_SAFEFREE(pcShName);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: gdbHandleQCmd
** ��������: ��Ӧgdb Q����
** �䡡��  : pparam        gdbȫ�ֲ���
**           cInBuff       rsp��������ʼִ�еĵ�ַ
** �䡡��  : cOutBuff      rsp������ʾ����ִ�н��
**           ����ֵ        ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbHandleQCmd (LW_GDB_PARAM *pparam, PCHAR pcInBuff, PCHAR pcOutBuff)
{
    if (lib_strstr(pcInBuff, "NonStop:1") == pcInBuff) {
        pparam->GDB_bNonStop = 1;
        gdbReplyOk(pcOutBuff);

    } else if (lib_strstr(pcInBuff, "NonStop:0") == pcInBuff) {
        pparam->GDB_bNonStop = 0;
        gdbReplyOk(pcOutBuff);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: gdbCmdQuery
** ��������: ��Ӧgdb q����
** �䡡��  : pparam        gdbȫ�ֲ���
**           cInBuff       rsp��������ʼִ�еĵ�ַ
** �䡡��  : cOutBuff      rsp������ʾ����ִ�н��
**           ����ֵ        ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbCmdQuery (LW_GDB_PARAM *pparam, PCHAR pcInBuff, PCHAR pcOutBuff)
{
    addr_t            addrText = 0;
    addr_t            addrData = 0;
    addr_t            addrBss  = 0;

    LW_OBJECT_HANDLE *phdArray = NULL;
    UINT              uiThreadNum;
    UINT              i;
    UINT              uiStrLen = 0;
    ULONG             ulThreadId;
    CHAR              cThreadInfo[0x100] = {0};

    if (lib_strstr(pcInBuff, "Offsets") == pcInBuff) {                  /* ���س����ض�λ��Ϣ           */
        if (gdbGetElfOffset(pparam->GDB_iPid, pparam->GDB_cProgPath,
                            &addrText, &addrData, &addrBss) != ERROR_NONE) {
            pcOutBuff[0] = 0;
        }
        if (addrText) {
            sprintf(pcOutBuff, "Text=%lx;", (ULONG)addrText);
            if (addrData) {
                sprintf(pcOutBuff + lib_strlen(pcOutBuff), "Data=%lx;", (ULONG)addrData);
                sprintf(pcOutBuff + lib_strlen(pcOutBuff), "Bss=%lx", (ULONG)addrData);
            }
        } else {
            pcOutBuff[0] = 0;
        }
    
    } else if (lib_strstr(pcInBuff, "Supported") == pcInBuff) {
        sprintf(pcOutBuff, "PacketSize=1000;"
                           "qXfer:features:read+;"
                           "qXfer:libraries:read+;"
                           "QNonStop+");
    
    }else if (lib_strstr(pcInBuff, "Xfer:features:read:target.xml") == pcInBuff) {
        sprintf(pcOutBuff, archGdbTargetXml());
    
    }else if (lib_strstr(pcInBuff, "Xfer:features:read:arm-core.xml") == pcInBuff) {
        sprintf(pcOutBuff, archGdbCoreXml());
    
    } else if (lib_strstr(pcInBuff, "Xfer:libraries:read") == pcInBuff) {
        pcOutBuff[0] = 'l';
        if (vprocGetModsInfo(pparam->GDB_iPid, pcOutBuff + 1,
                             GDB_RSP_MAX_LEN - 4) <= 0) {
            gdbReplyError(pcOutBuff, 1);
            return  (PX_ERROR);
        }
    
    } else if (lib_strstr(pcInBuff, "fThreadInfo") == pcInBuff) {
        phdArray = (LW_OBJECT_HANDLE*)LW_GDB_SAFEMALLOC(GDB_MAX_THREAD_NUM *
                                                        sizeof(LW_OBJECT_HANDLE));
        if (phdArray == LW_NULL) {
            gdbReplyError(pcOutBuff, 1);
            return  (PX_ERROR);
        }

        API_DtraceProcessThread(pparam->GDB_pvDtrace, phdArray,
                                GDB_MAX_THREAD_NUM, &uiThreadNum);      /* ��ȡ�߳���Ϣ                 */
        if (uiThreadNum <= 0) {
            gdbReplyError(pcOutBuff, 1);
            LW_GDB_SAFEFREE(phdArray);
            return  (PX_ERROR);
        }

        uiStrLen += sprintf(pcOutBuff, "m");
        for (i = 0; i < uiThreadNum - 1; i++) {
            uiStrLen += sprintf(pcOutBuff + uiStrLen, "%lx,", (ULONG)phdArray[i]);
        }

        uiStrLen += sprintf(pcOutBuff + uiStrLen,
                            "%lx", (ULONG)phdArray[i]);                 /* ���һ���̲߳�׷��","        */

        LW_GDB_SAFEFREE(phdArray);
    
    } else if (lib_strstr(pcInBuff, "sThreadInfo") == pcInBuff) {
        sprintf(pcOutBuff, "l");
    
    } else if (lib_strstr(pcInBuff, "ThreadExtraInfo") == pcInBuff) {
        if (sscanf(pcInBuff + lib_strlen("ThreadExtraInfo,"), "%lx", &ulThreadId) < 1) {
            gdbReplyError(pcOutBuff, 1);
            return  (PX_ERROR);
        }
        API_DtraceThreadExtraInfo(pparam->GDB_pvDtrace, ulThreadId,
                                  cThreadInfo, sizeof(cThreadInfo) - 1);
        for (i = 0; cThreadInfo[i] != 0; i++) {
            gdbByte2Asc(&pcOutBuff[i * 2], cThreadInfo[i]);
        }
    
    } else {
        pcOutBuff[0] = 0;
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: gdbReportStopReason
** ��������: �ϱ�ֹͣԭ��
** �䡡��  : pdmsg         �ж���Ϣ
**           pcOutBuff     ���������
** �䡡��  : ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbReportStopReason (PLW_DTRACE_MSG pdmsg, PCHAR pcOutBuff)
{
    if (pdmsg->DTM_uiType == SIGQUIT) {
        sprintf(pcOutBuff, "W00");
        return  (ERROR_NONE);
    }

    sprintf(pcOutBuff, "T%02xthread:%lx;",
            pdmsg->DTM_uiType, pdmsg->DTM_ulThread);                    /* ����ֹͣԭ��Ĭ��Ϊ�ж�     */

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: gdbHcmdHandle
** ��������: ��Ӧgdb H����
** �䡡��  : pparam        gdbȫ�ֲ���
**           cInBuff       rsp��������ʼִ�еĵ�ַ
** �䡡��  : cOutBuff      rsp������ʾ����ִ�н��
**           ����ֵ        ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbHcmdHandle (LW_GDB_PARAM *pparam, PCHAR pcInBuff, PCHAR pcOutBuff)
{
    CHAR    cOp;
    LONG    lThread;

    cOp = pcInBuff[0];
    if (pcInBuff[1] == '-') {
        if (sscanf(pcInBuff + 2, "%lx", (ULONG*)&lThread) < 1) {
            gdbReplyError(pcOutBuff, 1);
            return  (PX_ERROR);
        }
        lThread = 0 - lThread;
    
    } else {
        if (sscanf(pcInBuff + 1, "%lx", (ULONG*)&lThread) < 1) {
            gdbReplyError(pcOutBuff, 1);
            return  (PX_ERROR);
        }
    }

    if (cOp == 'c') {
        if (lThread == -1) {                                            /* -1��0����ʽһ��            */
            lThread = 0;
        }
        pparam->GDB_lOpCThreadId = lThread;
    
    } else if (cOp == 'g') {
        pparam->GDB_lOpGThreadId = lThread;
    
    } else {
        gdbReplyError(pcOutBuff, 1);
        return  (PX_ERROR);
    }

    gdbReplyOk(pcOutBuff);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: gdbContHandle
** ��������: ��Ӧgdb vCont����
** �䡡��  : pparam        gdbȫ�ֲ���
**           cInBuff       rsp��������ʼִ�еĵ�ַ
** �䡡��  : cOutBuff      rsp������ʾ����ִ�н��
**           ����ֵ        ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����1��ʾ����ѭ��
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbContHandle (LW_GDB_PARAM     *pparam,
                          PLW_DTRACE_MSG    pdmsg,
                          PCHAR             pcInBuff,
                          PCHAR             pcOutBuff)
{
    CHAR        chOp     = '\0';
    ULONG       ulTid    = 0;
    INT         iSigNo;
    GDB_REG_SET regset;

    chOp = pcInBuff[0];
    if ('S' == chOp || 'C' == chOp) {
        sscanf(pcInBuff + 1, "%02x:%lx", &iSigNo, &ulTid);

    } else if ('s' == chOp || 'c' == chOp) {
        sscanf(pcInBuff + 1, ":%lx", &ulTid);

    } else {
        gdbReplyError(pcOutBuff, 1);

        return  (ERROR_NONE);
    }

    pparam->GDB_lOpCThreadId = ulTid;

    if ('S' == chOp || 's' == chOp) {
        if (ulTid == 0 || ulTid == -1) {
            gdbReplyError(pcOutBuff, 2);

            return  (ERROR_NONE);
        }

        archGdbRegsGet(pparam->GDB_pvDtrace, ulTid, &regset);
        pparam->GDB_bpStep.BP_addr = archGdbGetNextPc(&regset);
        API_DtraceBreakpointInsert(pparam->GDB_pvDtrace,
                                   pparam->GDB_bpStep.BP_addr,
                                   &pparam->GDB_bpStep.BP_ulInstOrg);
    }

    return  (1);
}
/*********************************************************************************************************
** ��������: gdbHcmdHandle
** ��������: ��Ӧgdb v����
** �䡡��  : pparam        gdbȫ�ֲ���
**           cInBuff       rsp��������ʼִ�еĵ�ַ
** �䡡��  : cOutBuff      rsp������ʾ����ִ�н��
**           ����ֵ        ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����, 1��ʾ
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbVcmdHandle (LW_GDB_PARAM     *pparam,
                          PLW_DTRACE_MSG    pdmsg,
                          PCHAR             pcInBuff,
                          PCHAR             pcOutBuff)
{

    if (lib_strstr(pcInBuff, "Cont?") == pcInBuff) {
        lib_strncpy(pcOutBuff, "vCont;c;C;s;S", GDB_RSP_MAX_LEN);  //in linux is vCont;c;C;s;S;t;r

    } else if (lib_strstr(pcInBuff, "Cont;") == pcInBuff) {
        return gdbContHandle(pparam,
                             pdmsg,
                             pcInBuff + lib_strlen("Cont;"),
                             pcOutBuff);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: gdbCheckThread
** ��������: ��Ӧgdb H����
** �䡡��  : pparam        gdbȫ�ֲ���
**           cInBuff       rsp��������ʼִ�еĵ�ַ
** �䡡��  : cOutBuff      rsp������ʾ����ִ�н��
**           ����ֵ        ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbCheckThread (LW_GDB_PARAM *pparam, PCHAR pcInBuff, PCHAR pcOutBuff)
{
    ULONG ulThread;
    if (sscanf(pcInBuff, "%lx", (ULONG*)&ulThread) < 1) {
        gdbReplyError(pcOutBuff, 1);
        return  (PX_ERROR);
    }

    if (API_ThreadVerify(ulThread)) {
        gdbReplyOk(pcOutBuff);
        return  (ERROR_NONE);
    
    } else {
        gdbReplyError(pcOutBuff, 10);
        return  (ERROR_NONE);
    }
}
/*********************************************************************************************************
** ��������: gdbRspPkgHandle
** ��������: ����rspЭ����������gdb�Ľ���
** �䡡��  : pparam     ϵͳ����
** �䡡��  : ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbRspPkgHandle (LW_GDB_PARAM    *pparam,
                            PLW_DTRACE_MSG   pdmsg,
                            BOOL             bNeedReport)
{
    INT      iRet;
    CHAR    *cInBuff  = NULL;
    CHAR    *cOutBuff = NULL;

    cInBuff  = (CHAR *)LW_GDB_SAFEMALLOC(GDB_RSP_MAX_LEN);
    cOutBuff = (CHAR *)LW_GDB_SAFEMALLOC(GDB_RSP_MAX_LEN);
    if (!cInBuff || !cOutBuff) {
        LW_GDB_SAFEFREE(cInBuff);
        LW_GDB_SAFEFREE(cOutBuff);
        _DebugHandle(__ERRORMESSAGE_LEVEL, "Memory alloc failed.\r\n");
        return  (PX_ERROR);
    }

    if (bNeedReport) {
        gdbReportStopReason(pdmsg, cOutBuff);
        if (gdbRspPkgPut(pparam, cOutBuff) < 0) {
            LW_GDB_SAFEFREE(cInBuff);
            LW_GDB_SAFEFREE(cOutBuff);
            LW_GDB_MSG("[GDB]Host close.\n");
            return  (PX_ERROR);
        }
    }

    while (1) {
        lib_bzero(cInBuff, GDB_RSP_MAX_LEN);
        lib_bzero(cOutBuff, GDB_RSP_MAX_LEN);

        iRet = gdbRspPkgGet(pparam, cInBuff);
        if (iRet < 0) {
            LW_GDB_SAFEFREE(cInBuff);
            LW_GDB_SAFEFREE(cOutBuff);
            LW_GDB_MSG("[GDB]Host close.\n");
            return  (PX_ERROR);
        }
        switch (cInBuff[0]) {
        
        case 'g':
            gdbRegsGet(pparam, pdmsg, cInBuff + 1, cOutBuff);
            break;
        
        case 'G':
            gdbRegsSet(pparam, pdmsg, cInBuff + 1, cOutBuff);
            break;
        
        case 'p':
            gdbRegGet(pparam, pdmsg, cInBuff + 1, cOutBuff);
            break;
        
        case 'P':
            gdbRegSet(pparam, pdmsg, cInBuff + 1, cOutBuff);
            break;
        
        case 'm':
            gdbMemGet(pparam, cInBuff + 1, cOutBuff);
            break;
        
        case 'M':
            gdbMemSet(pparam, cInBuff + 1, cOutBuff);
            break;
        
        case 'c':
            gdbGoTo(pparam, pdmsg, cInBuff+1, cOutBuff, 0);
            LW_GDB_SAFEFREE(cInBuff);
            LW_GDB_SAFEFREE(cOutBuff);
            return  (ERROR_NONE);
        
        case 'q':
            gdbCmdQuery(pparam, cInBuff + 1, cOutBuff);
            break;
        
        case 'Q':
            gdbHandleQCmd(pparam, cInBuff + 1, cOutBuff);
            break;
        
        case '?':
            gdbReportStopReason(pdmsg, cOutBuff);
            break;
        
        case 'H':
            gdbHcmdHandle(pparam, cInBuff + 1, cOutBuff);
            break;
        
        case 'T':
            gdbCheckThread(pparam, cInBuff + 1, cOutBuff);
            break;
        
        case 's':
            gdbGoTo(pparam, pdmsg, cInBuff + 1, cOutBuff, 1);
            LW_GDB_SAFEFREE(cInBuff);
            LW_GDB_SAFEFREE(cOutBuff);
            return  (ERROR_NONE);
            break;
        
        case 'z':
            gdbRemoveBP(pparam, cInBuff + 1, cOutBuff);
            break;
        
        case 'Z':
            gdbInsertBP(pparam, cInBuff + 1, cOutBuff);
            break;

        case 'v':
            if (gdbVcmdHandle(pparam, pdmsg, cInBuff + 1, cOutBuff) > 0) {
                LW_GDB_SAFEFREE(cInBuff);
                LW_GDB_SAFEFREE(cOutBuff);
                return  (ERROR_NONE);
            }
            break;

        default:
            cOutBuff[0] = 0;
            break;
        }

        if (gdbRspPkgPut(pparam, cOutBuff) < 0) {
            LW_GDB_SAFEFREE(cInBuff);
            LW_GDB_SAFEFREE(cOutBuff);
            LW_GDB_MSG("[GDB]Host close.\n");
            return  (PX_ERROR);
        }
    }

    LW_GDB_SAFEFREE(cInBuff);
    LW_GDB_SAFEFREE(cOutBuff);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: gdbWaitGdbSig
** ��������: �¼�ѭ��������ϵͳ��Ϣ����Ⲣ����ϵ�
** �䡡��  : pparam       ϵͳ����
** �䡡��  : bClientSig   �Ƿ�ͻ����ź�
**           pfdsi        ���ܵ����ź���Ϣ
**           ����         ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbWaitSig (LW_GDB_PARAM            *pparam,
                       BOOL                    *pbClientSig,
                       struct signalfd_siginfo *pfdsi)
{
    fd_set      fdset;
    ssize_t     szLen;
    INT         iMaxFd;
    CHAR        chCmd;

    (*pbClientSig) = FALSE;

    FD_ZERO(&fdset);
    FD_SET(pparam->GDB_iCommFd, &fdset);
    FD_SET(pparam->GDB_iSigFd, &fdset);
    iMaxFd = max(pparam->GDB_iCommFd, pparam->GDB_iSigFd);

    while (select(iMaxFd + 1, &fdset, NULL, NULL, NULL) >= 0) {
        if (FD_ISSET(pparam->GDB_iCommFd, &fdset)) {
            if (read(pparam->GDB_iCommFd, &chCmd, sizeof(chCmd)) <= 0) {
                return  (PX_ERROR);
            }

            if (chCmd == 0x3) {
                (*pbClientSig) = TRUE;
                return  (ERROR_NONE);
            }
        }

        if (FD_ISSET(pparam->GDB_iSigFd, &fdset)) {
            szLen = read(pparam->GDB_iSigFd, pfdsi, sizeof(struct signalfd_siginfo));
            if (szLen != sizeof(struct signalfd_siginfo)) {
                continue;
            }
            return  (ERROR_NONE);
        }
    }

    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: gdbSigInit
** ��������: ��ʼ���źž��
** �䡡��  :
** �䡡��  : signalfd���
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbSigInit (VOID)
{
    sigset_t                sigset;

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGTRAP);
    sigaddset(&sigset, SIGCHLD);

    if (sigprocmask(SIG_BLOCK, &sigset, NULL) < ERROR_NONE) {
        return  (PX_ERROR);
    }

    return  (signalfd(-1, &sigset, 0));
}
/*********************************************************************************************************
** ��������: gdbRelease
** ��������: �ͷ���Դ
** �䡡��  : pparam     ϵͳ����
** �䡡��  : ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbRelease (LW_GDB_PARAM *pparam)
{
    PLW_LIST_LINE       plineTemp;
    LW_GDB_BP          *pbpItem;

    plineTemp = pparam->GDB_plistBpHead;                                /* �ͷŶϵ�����                 */
    while (plineTemp) {
        pbpItem  = _LIST_ENTRY(plineTemp, LW_GDB_BP, BP_plistBpLine);
        plineTemp = _list_line_get_next(plineTemp);
        API_DtraceBreakpointRemove(pparam->GDB_pvDtrace,
                                   pbpItem->BP_addr,
                                   pbpItem->BP_ulInstOrg);
        LW_GDB_SAFEFREE(pbpItem);
    }
    pparam->GDB_plistBpHead = LW_NULL;

    if (pparam->GDB_pvDtrace != LW_NULL) {
        if (pparam->GDB_bpStep.BP_addr != 0) {
            API_DtraceBreakpointRemove(pparam->GDB_pvDtrace,
                                       pparam->GDB_bpStep.BP_addr,
                                       pparam->GDB_bpStep.BP_ulInstOrg);
        }
        API_DtraceDelete(pparam->GDB_pvDtrace);
    }

    if (pparam->GDB_iSigFd >= 0) {
        close(pparam->GDB_iSigFd);
    }

    if (pparam->GDB_iCommFd >= 0) {
        close(pparam->GDB_iCommFd);
    }

    LW_GDB_MSG("[GDB]Server exit.\n");

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: gdbEventLoop
** ��������: �¼�ѭ��������ϵͳ��Ϣ����Ⲣ����ϵ�
** �䡡��  : pparam     ϵͳ����
** �䡡��  : ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbEventLoop (LW_GDB_PARAM *pparam)
{
    LW_DTRACE_MSG           dmsg;
    BOOL                    bClientSig;
    struct signalfd_siginfo fdsi;

    dmsg.DTM_ulThread = vprocMainThread(pparam->GDB_iPid);

    if (gdbRspPkgHandle(pparam, &dmsg, FALSE) == PX_ERROR) {            /* ��ʼ��ʱ���̴�����ͣ״̬     */
        return  (ERROR_NONE);
    }
    
    API_DtraceContinueProcess(pparam->GDB_pvDtrace);                    /* ��������                     */

    while (gdbWaitSig(pparam, &bClientSig, &fdsi) == ERROR_NONE) {
        if (bClientSig) {                                               /* �ͻ����źŶ��⴦��           */
            API_DtraceStopProcess(pparam->GDB_pvDtrace);                /* ֹͣ����                     */
            dmsg.DTM_ulThread = vprocMainThread(pparam->GDB_iPid);
            if (gdbRspPkgHandle(pparam, &dmsg, TRUE) == PX_ERROR) {     /* С��0��ʾ�ͻ��˹ر�����      */
                return  (ERROR_NONE);
            }

            if (pparam->GDB_lOpCThreadId <= 0) {
                API_DtraceContinueProcess(pparam->GDB_pvDtrace);
            } else {
                API_DtraceContinueThread(pparam->GDB_pvDtrace,
                                         pparam->GDB_lOpCThreadId);     /* ��ǰ�߳�ֹͣ������һ         */
            }
        }

        if ((fdsi.ssi_signo == SIGCHLD) &&
            (fdsi.ssi_code == CLD_EXITED)) {                            /* �����˳�                     */
            dmsg.DTM_ulAddr   = 0;
            dmsg.DTM_ulThread = vprocMainThread(pparam->GDB_iPid);;
            dmsg.DTM_uiType   = SIGQUIT;
            gdbRspPkgHandle(pparam, &dmsg, TRUE);
            LW_GDB_MSG("[GDB]Targer process exit.\n");
            return  (ERROR_NONE);
        }

        while (API_DtraceGetBreakInfo(pparam->GDB_pvDtrace, &dmsg,
                                      pparam->GDB_lOpCThreadId) == ERROR_NONE) {
            if (dmsg.DTM_ulAddr == pparam->GDB_bpStep.BP_addr) {        /* �����ϵ��Զ��Ƴ�             */
                pparam->GDB_bpStep.BP_addr = (addr_t)0;
                API_DtraceBreakpointRemove(pparam->GDB_pvDtrace,
                                           dmsg.DTM_ulAddr,
                                           pparam->GDB_bpStep.BP_ulInstOrg);
            }

            if (pparam->GDB_lOpCThreadId <= 0) {
                API_DtraceStopProcess(pparam->GDB_pvDtrace);            /* ֹͣ����                     */
                API_DtraceContinueThread(pparam->GDB_pvDtrace,
                                         dmsg.DTM_ulThread);            /* ��ǰ�߳�ֹͣ������һ         */
            }

            if (gdbRspPkgHandle(pparam, &dmsg, TRUE) == PX_ERROR) {     /* С��0��ʾ�ͻ��˹ر�����      */
                return  (ERROR_NONE);
            }

            if (pparam->GDB_lOpCThreadId <= 0) {
                API_DtraceContinueProcess(pparam->GDB_pvDtrace);
            } else {
                API_DtraceContinueThread(pparam->GDB_pvDtrace,
                                         pparam->GDB_lOpCThreadId);     /* ��ǰ�߳�ֹͣ������һ         */
            }

            pparam->GDB_lOpGThreadId = 0;
        }
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: gdbWaitSpawmSig
** ��������: �ȴ���������ź�
** �䡡��  : iSigNo �ź�ֵ
** �䡡��  : ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbWaitSpawmSig (INT iSigNo)
{
    sigset_t                sigset;
    INT                     iSigFd;
    ssize_t                 szLen;
    struct signalfd_siginfo fdsi;

    sigemptyset(&sigset);
    sigaddset(&sigset, iSigNo);

    if (sigprocmask(SIG_BLOCK, &sigset, NULL) < ERROR_NONE) {
        return  (PX_ERROR);
    }

    iSigFd = signalfd(-1, &sigset, 0);
    if (iSigFd < 0) {
        return  (PX_ERROR);
    }

    szLen = read(iSigFd, &fdsi, sizeof(struct signalfd_siginfo));
    if (szLen != sizeof(struct signalfd_siginfo)) {
        close(iSigFd);
        return  (PX_ERROR);
    }

    close(iSigFd);

    return  (fdsi.ssi_int);
}
/*********************************************************************************************************
** ��������: gdbMain
** ��������: gdb stub ������
** �䡡��  : argc         ��������
**           argv         ������
** �䡡��  : ERROR_NONE ��ʾû�д���, PX_ERROR ��ʾ����
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT gdbMain (INT argc, CHAR **argv)
{
    LW_GDB_PARAM        param     = {0};
    UINT32              ui32Ip    = INADDR_ANY;
    UINT16              usPort    = 0;
    INT                 iArgPos   = 1;
    INT                 i;
    INT                 iBeAttach = 0;
    INT                 iPid;
    posix_spawnattr_t   spawnattr;
    posix_spawnstop_t   spawnstop = {0};
    PCHAR               pcSerial  = LW_NULL;    

    param.GDB_iCommFd = PX_ERROR;
    param.GDB_iSigFd  = PX_ERROR;

    if (iArgPos > argc) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "Parameter error.\r\n");
        _ErrorHandle(ERROR_GDB_PARAM);
        return  (PX_ERROR);
    }

    if (lib_strcmp(argv[iArgPos], "--attach") == 0) {
        iArgPos++;
        iBeAttach = 1;
    }

    if (iArgPos > argc) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "Parameter error.\r\n");
        _ErrorHandle(ERROR_GDB_PARAM);
        return  (PX_ERROR);
    }

    param.GDB_byCommType = COMM_TYPE_TTY;
    for (i = 0; argv[iArgPos][i] != '\0'; i++) {                        /* ����ip�Ͷ˿�                 */
        if (argv[iArgPos][i] == ':') {
            argv[iArgPos][i] = '\0';
            sscanf(argv[iArgPos] + i + 1, "%hu", &usPort);
            if (lib_strcmp(argv[iArgPos], "localhost") != 0) {
                ui32Ip = inet_addr(argv[iArgPos]);
                if (IPADDR_NONE == ui32Ip) {
                    ui32Ip = INADDR_ANY;
                }
            } else {
                ui32Ip = INADDR_ANY;
            }
            param.GDB_byCommType = COMM_TYPE_TCP;
        }
    }
    
    if (param.GDB_byCommType == COMM_TYPE_TTY) {                        /* ���ڵ���                     */
        pcSerial = argv[iArgPos];
    }

    iArgPos++;

    if ((param.GDB_iSigFd = gdbSigInit()) < 0) {                        /* ��ʼ�������ź�               */
        gdbRelease(&param);
        _DebugHandle(__ERRORMESSAGE_LEVEL, "Initialize signal failed.\r\n");
        _ErrorHandle(ERROR_GDB_INIT_SOCK);
        return  (PX_ERROR);
    }

    param.GDB_pvDtrace = API_DtraceCreate(LW_DTRACE_PROCESS, 0,
                                          API_ThreadIdSelf());          /*  ����dtrace����              */
    if (param.GDB_pvDtrace == NULL) {
        gdbRelease(&param);
        _DebugHandle(__ERRORMESSAGE_LEVEL, "Create dtrace object error.\r\n");
        _ErrorHandle(ERROR_GDB_PARAM);
        return  (PX_ERROR);
    }

    if (iArgPos > argc) {
        gdbRelease(&param);
        _DebugHandle(__ERRORMESSAGE_LEVEL, "Parameter error.\r\n");
        _ErrorHandle(ERROR_GDB_PARAM);
        return  (PX_ERROR);
    }

    if (iBeAttach) {                                                    /* attach�����н���             */
        if (sscanf(argv[iArgPos], "%d", &iPid) != 1) {
            gdbRelease(&param);
            _DebugHandle(__ERRORMESSAGE_LEVEL, "Parameter error.\r\n");
            _ErrorHandle(ERROR_GDB_PARAM);
            return  (PX_ERROR);
        }

        param.GDB_iPid = iPid;
        if (API_DtraceSetPid(param.GDB_pvDtrace, iPid) == PX_ERROR) {   /* ��������                     */
            gdbRelease(&param);
            _DebugHandle(__ERRORMESSAGE_LEVEL, "Attach to program failed.\r\n");
            _ErrorHandle(ERROR_GDB_ATTACH_PROG);
            return  (PX_ERROR);
        }

        if (API_DtraceStopProcess(param.GDB_pvDtrace) == PX_ERROR) {    /* ֹͣ����                     */
            _DebugHandle(__ERRORMESSAGE_LEVEL, "Stop program failed.\r\n");
            _ErrorHandle(ERROR_GDB_ATTACH_PROG);
            return  (PX_ERROR);
        }
    } else {                                                            /* ��������attach             */
        posix_spawnattr_init(&spawnattr);
        spawnstop.SPS_iSigNo    = SIGUSR1;
        spawnstop.SPS_ulId      = API_ThreadIdSelf();
        posix_spawnattr_setstop(&spawnattr, &spawnstop);
        
        if (posix_spawn(&param.GDB_iPid, argv[iArgPos], NULL,
                        &spawnattr, &argv[iArgPos], NULL) != ERROR_NONE) {
            posix_spawnattr_destroy(&spawnattr);
            gdbRelease(&param);
            _DebugHandle(__ERRORMESSAGE_LEVEL, "Start program failed.\r\n");
            _ErrorHandle(ERROR_GDB_ATTACH_PROG);
            return  (PX_ERROR);
        
        } else {
            posix_spawnattr_destroy(&spawnattr);
        }

        if (API_DtraceSetPid(param.GDB_pvDtrace, 
                             param.GDB_iPid) == PX_ERROR) {             /* ��������                     */
            gdbRelease(&param);
            _DebugHandle(__ERRORMESSAGE_LEVEL, "Attach to program failed.\r\n");
            _ErrorHandle(ERROR_GDB_ATTACH_PROG);
            return  (PX_ERROR);
        }

        if (gdbWaitSpawmSig(spawnstop.SPS_iSigNo) == PX_ERROR) {        /* �ȴ����ؾ����ź�             */
            gdbRelease(&param);
            _DebugHandle(__ERRORMESSAGE_LEVEL, "Load program failed.\r\n");
            return  (PX_ERROR);
        }
    }
    
    vprocGetPath(param.GDB_iPid, param.GDB_cProgPath, MAX_FILENAME_LENGTH);

    if (param.GDB_byCommType == COMM_TYPE_TCP) {
        param.GDB_iCommFd = gdbTcpSockInit(ui32Ip, usPort);             /* ��ʼ��socket����ȡ���Ӿ��   */
    } else {
        param.GDB_iCommFd = gdbSerialInit(pcSerial);
    }
        
    if (param.GDB_iCommFd < 0) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "Initialize comunication failed.\r\n");
        _ErrorHandle(ERROR_GDB_INIT_SOCK);
        return  (PX_ERROR);
    }

    gdbEventLoop(&param);                                               /* �����¼�ѭ��                 */

    gdbRelease(&param);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: gdbInit
** ��������: ע�� GDBServer ����
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
VOID  API_GdbInit (VOID)
{
    API_TShellKeywordAddEx("debug", gdbMain, LW_OPTION_KEYWORD_SYNCBG);
    API_TShellFormatAdd("debug", "  [program file] [argments...]");
    API_TShellHelpAdd("debug",   "GDB Server\n");
}
#endif                                                                  /*  LW_CFG_GDB_EN > 0           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
