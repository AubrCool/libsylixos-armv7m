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
** ��   ��   ��: vmmAbort.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2012 �� 09 �� 05 ��
**
** ��        ��: ƽ̨�޹��쳣����.
**
** ע        ��: ������ȱҳ�ж�ʱ, ���е����I/O����(��������mmap���)������ vmm lock �½��е�!
                 SylixOS ����ȱҳ�쳣����������������, ͨ������������, ����, ��Ӱ���޹�����ĵ���������.
                 ����Ҳ�������������. ������������ͬһ��ַ�����쳣�ȵ�, ��������õ�һЩ�������������
                 �������.
                 
** BUG:
2012.09.15  δ����ָ����� SIGILL �ź�.
2012.12.22  �������͵Ĵ�ӡʹ���ַ���.
2012.12.22  __vmmAbortKillSignal() ��Ӧ�÷���, ���ǵȴ���ɾ��.
2013.03.29  �����쳣��, Ӧ�ûָ�֮ǰ�� errno.
2013.05.09  API_VmmAbortIsr() ��������ж�������, ֱ�Ӵ�����, ����Ҫ�������������Ĺ���.
2013.05.10  ȱ������ҳ��ʱ, Ӧ��ʹ�� SIGKILL �ź�ɱ������.
2013.09.13  �쳣�˳���, �ָ� kernel space ״̬.
2013.09.14  ֧���ļ�֮��Ĺ���ӳ��.
2013.11.21  ���� _doSigEvent() ���ò���.
2013.12.23  ֧�ֶ� LW_VMM_ABORT_TYPE_EXEC �Ĵ�����.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
/*********************************************************************************************************
  ����ü�֧��
*********************************************************************************************************/
#include "vmmSwap.h"
#include "dtrace.h"                                                     /*  ���Ը�����                  */
#if LW_CFG_VMM_EN > 0
#include "phyPage.h"
/*********************************************************************************************************
  �ں���Ϣ��ӡ�������ں˺���
*********************************************************************************************************/
#ifndef printk
#define printk
#endif                                                                  /*  printk                      */
#if LW_CFG_MODULELOADER_EN > 0
extern pid_t  API_ModulePid(PVOID pvVProc);
#else
#define API_ModulePid(x)     0
#endif                                                                  /*  LW_CFG_MODULELOADER_EN > 0  */
/*********************************************************************************************************
  �����ҳ�ռ����� (ȱҳ�жϲ��ұ�)
*********************************************************************************************************/
LW_LIST_LINE_HEADER       _K_plineVmmVAddrSpaceHeader = LW_NULL;
/*********************************************************************************************************
  ͳ�Ʊ���
*********************************************************************************************************/
INT64                     _K_i64VmmAbortCounter    = 0;                 /*  �쳣��ֹ����                */
INT64                     _K_i64VmmPageFailCounter = 0;                 /*  ȱҳ�ж������������        */
INT64                     _K_i64VmmPageLackCounter = 0;                 /*  ϵͳȱ������ҳ�����        */
INT64                     _K_i64VmmMapErrCounter   = 0;                 /*  ӳ��������                */
/*********************************************************************************************************
  �ڲ���������
*********************************************************************************************************/
static VOID   __vmmAbortKillSignal(PLW_VMM_PAGE_FAIL_CTX  pvmpagefailctx);
static VOID   __vmmAbortAccess(PLW_VMM_PAGE_FAIL_CTX  pvmpagefailctx);
/*********************************************************************************************************
  ������
*********************************************************************************************************/
extern  LW_OBJECT_HANDLE    _G_ulVmmLock;
#define __VMM_LOCK()        API_SemaphoreMPend(_G_ulVmmLock, LW_OPTION_WAIT_INFINITE)
#define __VMM_UNLOCK()      API_SemaphoreMPost(_G_ulVmmLock)
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
#define __PAGEFAIL_ALLOC_PAGE_NUM   1                                   /*  ȱҳ�жϷ���ҳ����, ����Ϊ1 */
#define __PAGEFAIL_CUR_PID          API_ModulePid(ptcbCur->TCB_pvVProcessContext)
/*********************************************************************************************************
** ��������: __vmmAbortCacheRefresh
** ��������: �� MMU ȱҳ������ɺ�, ���ﴦ�� cache ��ˢ�²���.
** �䡡��  : bSwapNeedLoad     �Ƿ��Ǵ� swap �ж�������
**           
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID __vmmAbortCacheRefresh (BOOL    bSwapNeedLoad,
                                    addr_t  ulVirtualPageAlign,
                                    ULONG   ulAllocPageNum)
{
#if LW_CFG_CACHE_EN > 0
    size_t  stSize = 0;
    
    if (API_CacheLocation(DATA_CACHE) == CACHE_LOCATION_VIVT) {         /*  ����������ַ cache        */
        stSize = (size_t)(ulAllocPageNum * LW_CFG_VMM_PAGE_SIZE);
        API_CacheClear(DATA_CACHE, (PVOID)LW_CFG_VMM_VIRTUAL_SWITCH,
                       stSize);                                         /*  ������д���ڴ沢��������    */
        API_CacheInvalidate(DATA_CACHE, (PVOID)ulVirtualPageAlign, 
                            stSize);                                    /*  ��Ч�������ڴ�ռ�          */
    }
    
    if (bSwapNeedLoad) {                                                /*  swap load �п����Ǵ���      */
        if (stSize == 0) {
            stSize = (size_t)(ulAllocPageNum * LW_CFG_VMM_PAGE_SIZE);
        }
        API_CacheTextUpdate((PVOID)ulVirtualPageAlign, stSize);         /*  ��Ч I-CACHE (�ڲ��Ѿ���д) */
    }
#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
}
/*********************************************************************************************************
** ��������: __vmmAbortPageGet
** ��������: �����쳣��ַ, ��ȡ����ҳ����ƿ�
** �䡡��  : ulAbortAddr       �쳣��ַ
** �䡡��  : ����ҳ����ƿ�
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static PLW_VMM_PAGE  __vmmAbortPageGet (addr_t  ulAbortAddr)
{
             PLW_LIST_LINE          plineTemp;
    REGISTER PLW_VMM_PAGE           pvmpageVirtual;
    REGISTER PLW_VMM_PAGE_PRIVATE   pvmpagep;
    
    for (plineTemp  = _K_plineVmmVAddrSpaceHeader;
         plineTemp != LW_NULL;
         plineTemp  = _list_line_get_next(plineTemp)) {                 /*  ��ѯ���ʵĵ�ַ�Ƿ�Ϊ�Ϸ���ַ*/
         
        pvmpagep       = _LIST_ENTRY(plineTemp, LW_VMM_PAGE_PRIVATE, PAGEP_lineManage);
        pvmpageVirtual = pvmpagep->PAGEP_pvmpageVirtual;
        
        if ((ulAbortAddr >= pvmpageVirtual->PAGE_ulPageAddr) &&
            (ulAbortAddr <  (pvmpageVirtual->PAGE_ulPageAddr + 
                             (pvmpageVirtual->PAGE_ulCount * LW_CFG_VMM_PAGE_SIZE)))) {
            break;
        }
    }
    
    if (plineTemp) {
        return  (pvmpageVirtual);
    } else {
        return  (LW_NULL);
    }
}
/*********************************************************************************************************
** ��������: __vmmAbortShellDumpMessage
** ��������: �� __vmmAbortShell() �޷�ִ��ʱ, ��ӡ������Ϣ
** �䡡��  : pvmpagefailctx    page fail ������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __vmmAbortShellDumpMessage (PLW_VMM_PAGE_FAIL_CTX  pvmpagefailctx)
{
#if LW_CFG_POSIX_EN > 0
LW_API  int  mmapfd(void  *pvAddr);
#endif                                                                  /*  LW_CFG_POSIX_EN > 0         */

             addr_t                 ulAbortAddr = pvmpagefailctx->PAGEFCTX_ulAbortAddr;
             LW_OBJECT_HANDLE       ulOwner;
             CHAR                   cBuffer[PATH_MAX + 1];
             PCHAR                  pcTail = LW_NULL;
             PCHAR                  pcType = LW_NULL;
             CPCHAR                 pcKernelFunc;
             CHAR                   cMmapMsg[PATH_MAX + 1] = "<unknown>.";
             
    REGISTER PLW_VMM_PAGE           pvmpageVirtual;
    REGISTER PLW_VMM_PAGE_PRIVATE   pvmpagep;
    
    ulOwner      = __KERNEL_OWNER();
    pcKernelFunc = __KERNEL_ENTERFUNC();
    
    pvmpageVirtual = __vmmAbortPageGet(ulAbortAddr);                    /*  ��ö�Ӧ�����ڴ���ƿ�      */
    
    if (pvmpageVirtual) {
        pvmpagep = (PLW_VMM_PAGE_PRIVATE)pvmpageVirtual->PAGE_pvAreaCb;
        if (pvmpagep->PAGEP_pfuncFiller) {
#if LW_CFG_POSIX_EN > 0
            INT     iFd = mmapfd((void *)ulAbortAddr);                  /*  ��õ�ַ��Ӧ���ļ�������    */
            snprintf(cMmapMsg, PATH_MAX + 1, "address in mmap, fd %d\n", iFd);
#endif                                                                  /*  LW_CFG_POSIX_EN > 0         */
            pcTail = cMmapMsg;
        
        } else {
            pcTail = "address in vmm.";
        }
    } else {
        pcTail = "address invalidate.";
    }
    
    switch (pvmpagefailctx->PAGEFCTX_ulAbortType) {
    
    case LW_VMM_ABORT_TYPE_TERMINAL:
        pcType = "cpu extremity error";
        break;
        
    case LW_VMM_ABORT_TYPE_MAP:
        pcType = "memory map";
        break;
    
    case LW_VMM_ABORT_TYPE_WRITE:
        pcType = "can not write";
        break;
    
    case LW_VMM_ABORT_TYPE_FPE:
        pcType = "float points";
        break;
    
    case LW_VMM_ABORT_TYPE_BUS:
        pcType = "bus";
        break;
    
    case LW_VMM_ABORT_TYPE_BREAK:
        pcType = "break points";
        break;
    
    case LW_VMM_ABORT_TYPE_SYS:
        pcType = "syscall";
        break;
    
    case LW_VMM_ABORT_TYPE_UNDEF:
        pcType = "undefined instruction";
        break;
    
    default:
        pcType = "unknown";
        break;
    }
    
    snprintf(cBuffer, PATH_MAX + 1, "abort in kernel status, "
                                    "owner : 0x%08lx, func : %s, addr: 0x%08lx, type : %s, %s.\r\n", 
                                    ulOwner, 
                                    pcKernelFunc,
                                    pvmpagefailctx->PAGEFCTX_ulAbortAddr,
                                    pcType,
                                    pcTail);
                                    
    _DebugHandle(__ERRORMESSAGE_LEVEL, cBuffer);
}
/*********************************************************************************************************
** ��������: __vmmAbortCopyOnWrite
** ��������: ���������ϸı�ʱ, ��Ҫ���ı��ҳ���Ϊ private, ���ٱ�����.
** �䡡��  : pvmpageVirtual        ����ռ�
**           ulAbortAddrAlign      �쳣�ĵ�ַ
** �䡡��  : �Ƿ�ɹ�
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : 1: ����ǹ���仯�Ķ�, ������Ϊ�����ַ cache ����, �� mmap �϶��Ѿ�����Ӧ����� cache ����
                ����, ���ﲻ��Ҫ���� cache
             2: __vmmPhysicalPageClone() ����һ��Ϊ pvmpagePhysical, ��Ϊ pvmpagePhysical ��Ӧ�������ַ
                ����������Ҫ copy ������Դ, ��Ϊ pvmpageReal ��Ӧ�������ַ�����Ѿ������ڻ��߱���д��.
*********************************************************************************************************/
static INT  __vmmAbortCopyOnWrite (PLW_VMM_PAGE  pvmpageVirtual, addr_t  ulAbortAddrAlign)
{
    PLW_VMM_PAGE            pvmpagePhysical = __pageFindLink(pvmpageVirtual, ulAbortAddrAlign);
    PLW_VMM_PAGE            pvmpageReal;
    PLW_VMM_PAGE            pvmpageNew;
    PLW_VMM_PAGE_PRIVATE    pvmpagep;

    ULONG                   ulError;
    
    if (pvmpagePhysical == LW_NULL) {
        return  (PX_ERROR);
    }
    
    pvmpagep = (PLW_VMM_PAGE_PRIVATE)pvmpageVirtual->PAGE_pvAreaCb;

    if (LW_VMM_PAGE_IS_FAKE(pvmpagePhysical)) {
        pvmpageReal = LW_VMM_PAGE_GET_REAL(pvmpagePhysical);
    
    } else {
        pvmpageReal = pvmpagePhysical;
    }
    
    if ((pvmpageReal->PAGE_ulRef == 1) ||                               /*  ��ǰ����ҳ����Ψһ����      */
        (pvmpagep->PAGEP_iFlags & LW_VMM_SHARED_CHANGE)) {              /*  ����仯                    */
        
        pvmpageReal->PAGE_iChange     = 1;
        pvmpagePhysical->PAGE_iChange = 1;
        pvmpageReal->PAGE_ulFlags     = pvmpageVirtual->PAGE_ulFlags;
        pvmpagePhysical->PAGE_ulFlags = pvmpageVirtual->PAGE_ulFlags;
        
        __vmmLibSetFlag(ulAbortAddrAlign, pvmpageVirtual->PAGE_ulFlags);/*  ��д                        */
        return  (ERROR_NONE);                                           /*  �������Լ���                */
    
    } else {                                                            /*  �������Ĺ������            */
                                                                        /*  ִ�� copy-on-write ����     */
        pvmpageNew = __vmmPhysicalPageClone(pvmpagePhysical);
        if (pvmpageNew == LW_NULL) {
            if (API_GetLastError() == ERROR_VMM_LOW_PHYSICAL_PAGE) {
                printk(KERN_CRIT "kernel no more physical page.\n");    /*  ϵͳ�޷���������ҳ��        */
            }
            return  (PX_ERROR);
        }
        
        __pageUnlink(pvmpageVirtual, pvmpagePhysical);                  /*  ������ӹ�ϵ                */
        
        __vmmPhysicalPageFree(pvmpagePhysical);                         /*  ����ʹ��������ҳ��          */
        
        ulError = __vmmLibPageMap(pvmpageNew->PAGE_ulPageAddr,          
                                  ulAbortAddrAlign, 1,
                                  pvmpageVirtual->PAGE_ulFlags);        /*  ��д                        */
        if (ulError) {
            __vmmPhysicalPageFree(pvmpageNew);
            return  (PX_ERROR);
        }
        
        pvmpageNew->PAGE_ulMapPageAddr = ulAbortAddrAlign;
        pvmpageNew->PAGE_ulFlags       = pvmpageVirtual->PAGE_ulFlags;
        pvmpageNew->PAGE_iChange       = 1;
                                                                        
        __pageLink(pvmpageVirtual, pvmpageNew);                         /*  ��������                    */
    }

    return  (ERROR_NONE);
}

/*********************************************************************************************************
** ��������: __vmmAbortWriteProtect
** ��������: ����д������ֹ
** �䡡��  : pvmpageVirtual        ����ռ�
**           ulAbortAddrAlign      �쳣�ĵ�ַ
**           ulFlag                ҳ������
** �䡡��  : �Ƿ�ɹ�
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : 
*********************************************************************************************************/
static INT  __vmmAbortWriteProtect (PLW_VMM_PAGE  pvmpageVirtual, 
                                    addr_t        ulAbortAddrAlign,
                                    ULONG         ulFlag)
{
    if (ulFlag & LW_VMM_FLAG_WRITABLE) {
        return  (ERROR_NONE);                                           /*  ������������ͬʱ���ʴ˵�ַ  */
    }
    
    if (pvmpageVirtual->PAGE_ulFlags & LW_VMM_FLAG_WRITABLE) {          /*  ����ռ�����д����          */
        return  (__vmmAbortCopyOnWrite(pvmpageVirtual, 
                                       ulAbortAddrAlign));              /*  copy-on-write               */
    }
    
    return  (PX_ERROR);                                                 /*  ɱ������, �ڴ治��д        */
}
/*********************************************************************************************************
** ��������: __vmmAbortNoPage
** ��������: ȱҳ��ֹʱ�����µ�ҳ��
** �䡡��  : pvmpagePhysical       ����ҳ��
**           pvmpagep              ����ռ�����
**           ulVirtualPageAlign    �����ַ
**           ulAllocPageNum        ����ҳ������
** �䡡��  : �Ƿ�ɹ�
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : 
*********************************************************************************************************/
static INT  __vmmAbortNoPage (PLW_VMM_PAGE           pvmpagePhysical, 
                              PLW_VMM_PAGE_PRIVATE   pvmpagep,
                              addr_t                 ulVirtualPageAlign, 
                              ULONG                  ulAllocPageNum)
{
    ULONG   ulError;

    if (pvmpagep->PAGEP_pfuncFiller) {                                  /*  ��Ҫ���                    */
        ulError = __vmmLibPageMap(pvmpagePhysical->PAGE_ulPageAddr,     /*  ʹ�� CACHE ����             */
                                  LW_CFG_VMM_VIRTUAL_SWITCH,            /*  �����������ַ              */
                                  ulAllocPageNum, 
                                  LW_VMM_FLAG_RDWR);                    /*  ӳ��ָ���������ַ          */
        if (ulError) {
            _K_i64VmmMapErrCounter++;
            printk(KERN_CRIT "kernel physical page map error.\n");      /*  ϵͳ�޷�ӳ������ҳ��        */
            return  (PX_ERROR);
        }
        
        if (pvmpagep->PAGEP_pfuncFiller) {                              /*  ��ҳ����Ҫ���              */
            pvmpagep->PAGEP_pfuncFiller(pvmpagep->PAGEP_pvArg,          
                                        LW_CFG_VMM_VIRTUAL_SWITCH,      /*  ��Ҫ�����Ļ���Ŀ�������ַ  */
                                        ulVirtualPageAlign,             /*  ��������л���Ŀ�������ַ  */
                                        ulAllocPageNum);                /*  ������ҳ�����              */
        }
                                                                        /*  cache ˢ��                  */
        __vmmAbortCacheRefresh(LW_FALSE, ulVirtualPageAlign, ulAllocPageNum);
    
        __vmmLibSetFlag(LW_CFG_VMM_VIRTUAL_SWITCH, LW_VMM_FLAG_FAIL);   /*  VIRTUAL_SWITCH ���������   */
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __vmmAbortSwapPage
** ��������: ҳ�潻��
** �䡡��  : pvmpagePhysical       ����ҳ��
**           ulVirtualPageAlign    �����ַ
**           ulAllocPageNum        ҳ�����
** �䡡��  : �Ƿ�ɹ�
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : ��������������, �������ʱ, ������ vmm lock �½���, ����� vmm ���������� vmm �ϵ�����
             �������п��ܻ��������! ����Ҳ����ֱ�ӽ�������䵽ָ���������ַ��, ��Ϊ���״̬��û���κ���
             ������ס�����������ָ������ڴ�! ���ֻ���Ƚ�����ڴ�ת������л�������ҳ��.
             (���е���䶼���� vmm lock �´���ִ�е�)
*********************************************************************************************************/
static INT  __vmmAbortSwapPage (PLW_VMM_PAGE  pvmpagePhysical, 
                                addr_t        ulVirtualPageAlign, 
                                ULONG         ulAllocPageNum)
{
    PLW_CLASS_TCB   ptcbCur;                                            /*  ��ǰ������ƿ�              */
    ULONG           ulError;

    LW_TCB_GET_CUR_SAFE(ptcbCur);

    ulError = __vmmLibPageMap(pvmpagePhysical->PAGE_ulPageAddr,         /*  ʹ�� CACHE ����             */
                              LW_CFG_VMM_VIRTUAL_SWITCH,                /*  �����������ַ              */
                              ulAllocPageNum, 
                              LW_VMM_FLAG_RDWR);                        /*  ӳ��ָ���������ַ          */
    if (ulError) {
        _K_i64VmmMapErrCounter++;
        printk(KERN_CRIT "kernel physical page map error.\n");          /*  ϵͳ�޷�ӳ������ҳ��        */
        return  (PX_ERROR);
    }
    
    ulError = __vmmPageSwapLoad(__PAGEFAIL_CUR_PID,
                                LW_CFG_VMM_VIRTUAL_SWITCH, 
                                ulVirtualPageAlign);                    /*  �ӽ������ж�ȡҳ��ԭʼ����  */
    if (ulError) {
        printk(KERN_CRIT "kernel swap load page error.\n");             /*  ϵͳ�޷����� swap ҳ��      */
        return  (PX_ERROR);                                             /*  ������� swap �����ͳ�Ʊ���*/
    }
    
    __vmmAbortCacheRefresh(LW_TRUE, ulVirtualPageAlign, ulAllocPageNum);/*  cache ˢ��                  */
    
    __vmmLibSetFlag(LW_CFG_VMM_VIRTUAL_SWITCH, LW_VMM_FLAG_FAIL);       /*  VIRTUAL_SWITCH ���������   */
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __vmmAbortNewPage
** ��������: ����һ���µ�ҳ��, ���ȱ������ҳ���򽻻�һ���Ͼɵ�����ҳ��
** �䡡��  : ulAllocPageNum        ��Ҫ���ڴ�ҳ�����
** �䡡��  : ����ҳ��
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : 
*********************************************************************************************************/
static PLW_VMM_PAGE  __vmmAbortNewPage (ULONG  ulAllocPageNum)
{
    PLW_VMM_PAGE    pvmpagePhysical;
    ULONG           ulZoneIndex;
    PLW_CLASS_TCB   ptcbCur;                                            /*  ��ǰ������ƿ�              */
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);
    
    pvmpagePhysical = __vmmPhysicalPageAlloc(ulAllocPageNum, 
                                             LW_ZONE_ATTR_NONE,
                                             &ulZoneIndex);             /*  ���������ڴ�(1��ҳ��)       */
    if (pvmpagePhysical == LW_NULL) {                                   /*  �������ʧ���񽻻���һ��ҳ��*/
        _K_i64VmmPageLackCounter++;
        pvmpagePhysical = __vmmPageSwapSwitch(__PAGEFAIL_CUR_PID,
                                              ulAllocPageNum, 
                                              LW_ZONE_ATTR_NONE);       /*  ����ҳ�潻��                */
        return  (pvmpagePhysical);
    }
    
    return  (pvmpagePhysical);
}
/*********************************************************************************************************
** ��������: __vmmAbortShareCallback
** ��������: __vmmAbortShare �ص�����
** �䡡��  : pvStartAddr       ���ҵ��ɹ���������������ʵ�����ַ
**           stOffset          �ɹ���ҳ��ƫ��
** �䡡��  : ����ҳ��
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : 
*********************************************************************************************************/
static PVOID __vmmAbortShareCallback (PVOID  pvStartAddr, size_t  stOffset)
{
    PLW_VMM_PAGE         pvmpageVirtual;
    PLW_VMM_PAGE         pvmpagePhysical;
    PLW_VMM_PAGE_PRIVATE pvmpagep;
    addr_t               ulVirAddr = (addr_t)pvStartAddr;
    
    pvmpageVirtual = __areaVirtualSearchPage(ulVirAddr);
    if (pvmpageVirtual == LW_NULL) {
        return  (LW_NULL);
    }
    
    pvmpagep = (PLW_VMM_PAGE_PRIVATE)pvmpageVirtual->PAGE_pvAreaCb;
    if (pvmpagep == LW_NULL) {
        return  (LW_NULL);
    }
    
    if (pvmpagep->PAGEP_iFlags & LW_VMM_PRIVATE_CHANGE) {               /*  Ŀ�������������          */
        return  (LW_NULL);
    }
    
    ulVirAddr += stOffset;
    
    pvmpagePhysical = __pageFindLink(pvmpageVirtual, ulVirAddr);
    
    return  ((PVOID)pvmpagePhysical);
}
/*********************************************************************************************************
** ��������: __vmmAbortShare
** ��������: ��ͼ�� mmap �����ҵ����Թ�����ʵ�ҳ��
** �䡡��  : pvmpageVirtual        �����쳣������ҳ��
**           pvmpagep              ����ҳ����Ϣ
**           ulVirtualPageAlign    �����쳣��ҳ������ַ
** �䡡��  : �Ƿ��Ѿ����ù���
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : 
*********************************************************************************************************/
static INT  __vmmAbortShare (PLW_VMM_PAGE         pvmpageVirtual, 
                             PLW_VMM_PAGE_PRIVATE pvmpagep,
                             addr_t               ulVirtualPageAlign)
{
    PLW_VMM_PAGE    pvmpagePhysical;
    PLW_VMM_PAGE    pvmpageRef;
    ULONG           ulError;
    
    if (pvmpagep->PAGEP_pfuncFindShare == LW_NULL) {
        return  (PX_ERROR);
    }
    
    pvmpagePhysical = (PLW_VMM_PAGE)pvmpagep->PAGEP_pfuncFindShare(pvmpagep->PAGEP_pvFindArg, 
                                                                   ulVirtualPageAlign,
                                                                   __vmmAbortShareCallback);
    if (pvmpagePhysical == LW_NULL) {
        return  (PX_ERROR);
    }
    
    pvmpageRef = __vmmPhysicalPageRef(pvmpagePhysical);                 /*  ��������ҳ��                */
    if (pvmpageRef == LW_NULL) {
        return  (PX_ERROR);
    }
    
    ulError = __vmmLibPageMap(pvmpageRef->PAGE_ulPageAddr,
                              ulVirtualPageAlign, 1, 
                              pvmpageRef->PAGE_ulFlags);
    if (ulError) {
        __vmmPhysicalPageFree(pvmpageRef);
        return  (PX_ERROR);
    }
    
    pvmpageRef->PAGE_ulMapPageAddr = ulVirtualPageAlign;
    
    __pageLink(pvmpageVirtual, pvmpageRef);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: __vmmAbortShell
** ��������: �� MMU ��������ʧЧʱ, �߳�ִ�����庯��.
** �䡡��  : pvmpagefailctx    page fail ������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __vmmAbortShell (PLW_VMM_PAGE_FAIL_CTX  pvmpagefailctx)
{
             addr_t                 ulAbortAddr = pvmpagefailctx->PAGEFCTX_ulAbortAddr;
             ULONG                  ulFlag;
             
    REGISTER PLW_VMM_PAGE           pvmpageVirtual;
    REGISTER PLW_VMM_PAGE           pvmpagePhysical;
    REGISTER PLW_VMM_PAGE_PRIVATE   pvmpagep;
             
             ULONG                  ulAllocPageNum;
             BOOL                   bSwapNeedLoad;
             
             addr_t                 ulVirtualPageAlign;
             ULONG                  ulError;
             INT                    iRet;
             PLW_CLASS_TCB          ptcbCur;                            /*  ��ǰ������ƿ�              */

    if (__KERNEL_ISENTER()) {
        __vmmAbortShellDumpMessage(pvmpagefailctx);                     /*  ��ӡ�ؼ���Ϣ                */
        __vmmAbortKillSignal(pvmpagefailctx);
        goto    __abort_return;
    }
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);
    
    __VMM_LOCK();
    _K_i64VmmAbortCounter++;
    
    if ((pvmpagefailctx->PAGEFCTX_ulAbortType != LW_VMM_ABORT_TYPE_MAP) &&
        (pvmpagefailctx->PAGEFCTX_ulAbortType != LW_VMM_ABORT_TYPE_WRITE)) {
        __VMM_UNLOCK();
        __vmmAbortAccess(pvmpagefailctx);                               /*  �����쳣���                */
        goto    __abort_return;                                         /*  �������е�����              */
    }
    
    pvmpageVirtual = __vmmAbortPageGet(ulAbortAddr);                    /*  ��ö�Ӧ�����ڴ���ƿ�      */
    if (pvmpageVirtual) {
        pvmpagep = (PLW_VMM_PAGE_PRIVATE)pvmpageVirtual->PAGE_pvAreaCb;
    } else {
        __VMM_UNLOCK();
        __vmmAbortAccess(pvmpagefailctx);                               /*  �����쳣                    */
        goto    __abort_return;                                         /*  �������е�����              */
    }
    
    _K_i64VmmPageFailCounter++;                                         /*  ȱҳ�жϴ���++              */
    ptcbCur->TCB_i64PageFailCounter++;                                  /*  ȱҳ�жϴ���++              */
    
    ulVirtualPageAlign = ulAbortAddr & LW_CFG_VMM_PAGE_MASK;            /*  ��÷��ʵ�ַҳ�߽�          */
    
    ulError = __vmmLibGetFlag(ulVirtualPageAlign, &ulFlag);             /*  ����쳣��ַ������ҳ������  */
    if (ulError == ERROR_NONE) {                                        /*  ��������ҳ������            */
        if (pvmpagefailctx->PAGEFCTX_ulAbortType == LW_VMM_ABORT_TYPE_WRITE) {
            if (__vmmAbortWriteProtect(pvmpageVirtual,                  /*  д�����쳣                  */
                                       ulVirtualPageAlign,
                                       ulError) == ERROR_NONE) {        /*  ����д��������              */
                __VMM_UNLOCK();
                goto    __abort_return;
            }
        }
        __VMM_UNLOCK();
        __vmmAbortAccess(pvmpagefailctx);                               /*  �Ƿ��ڴ����                */
        goto    __abort_return;                                         /*  �������е�����              */
    
    } else {                                                            /*  ӳ�����, û������ҳ�����  */
        if (pvmpagep->PAGEP_iFlags & LW_VMM_SHARED_CHANGE) {            /*  ��������                    */
            if (__vmmAbortShare(pvmpageVirtual, 
                                pvmpagep,
                                ulVirtualPageAlign) == ERROR_NONE) {    /*  ���Թ���                    */
                __VMM_UNLOCK();
                goto    __abort_return;
            }
        }
        
        ulAllocPageNum  = __PAGEFAIL_ALLOC_PAGE_NUM;                    /*  ȱҳ�жϷ�����ڴ�ҳ�����  */
        
        pvmpagePhysical = __vmmAbortNewPage(ulAllocPageNum);            /*  ��������ҳ��                */
        if (pvmpagePhysical == LW_NULL) {
            __VMM_UNLOCK();
            
            pvmpagefailctx->PAGEFCTX_ulAbortType = 0;                   /*  ȱ������ҳ��                */
            printk(KERN_CRIT "kernel no more physical page.\n");        /*  ϵͳ�޷���������ҳ��        */
            __vmmAbortKillSignal(pvmpagefailctx);
            goto    __abort_return;
        }
        
        bSwapNeedLoad = __vmmPageSwapIsNeedLoad(__PAGEFAIL_CUR_PID,
                                                ulVirtualPageAlign);    /*  ����Ƿ���Ҫ load swap ���� */
        if (bSwapNeedLoad) {
            iRet = __vmmAbortSwapPage(pvmpagePhysical, 
                                      ulVirtualPageAlign,
                                      ulAllocPageNum);                  /*  ����ҳ�潻������            */
        
        } else {
            iRet = __vmmAbortNoPage(pvmpagePhysical, 
                                    pvmpagep,
                                    ulVirtualPageAlign, 
                                    ulAllocPageNum);                    /*  ����ҳ����䴦��            */
        }
        
        if (iRet != ERROR_NONE) {
            __vmmPhysicalPageFree(pvmpagePhysical);
            __VMM_UNLOCK();
            __vmmAbortKillSignal(pvmpagefailctx);
            goto    __abort_return;
        }
    }
    
    ulError = __vmmLibPageMap(pvmpagePhysical->PAGE_ulPageAddr,         /*  �����ʱ��ͬ������          */
                              ulVirtualPageAlign,
                              ulAllocPageNum, 
                              pvmpageVirtual->PAGE_ulFlags);            /*  ӳ��ָ���������ַ          */
    if (ulError) {
        _K_i64VmmMapErrCounter++;
        __vmmPhysicalPageFree(pvmpagePhysical);
        __VMM_UNLOCK();

        printk(KERN_CRIT "kernel physical page map error.\n");          /*  ϵͳ�޷�ӳ������ҳ��        */
        __vmmAbortKillSignal(pvmpagefailctx);
        goto    __abort_return;
    }
    
    pvmpagePhysical->PAGE_ulMapPageAddr = ulVirtualPageAlign;
    pvmpagePhysical->PAGE_ulFlags       = pvmpageVirtual->PAGE_ulFlags;
    
    __pageLink(pvmpageVirtual, pvmpagePhysical);                        /*  �������ӹ�ϵ                */
    __VMM_UNLOCK();
    
__abort_return:
    __KERNEL_SPACE_SET(pvmpagefailctx->PAGEFCTX_iKernelSpace);          /*  �ָ��ɽ���֮ǰ��״̬        */
    errno = pvmpagefailctx->PAGEFCTX_iLastErrno;                        /*  �ָ�֮ǰ�� errno            */
    archSigCtxLoad(pvmpagefailctx->PAGEFCTX_pvStackRet);                /*  �� page fail �������з���   */
}
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
/*********************************************************************************************************
   ���º���û�� MMU ʱҲ��ʹ��, ��Ҫ������������쳣
*********************************************************************************************************/
/*********************************************************************************************************
** ��������: __vmmAbortKillSignal
** ��������: ��ǰ�̲߳���һ���ź�
** �䡡��  : pvmpagefailctx    page fail ������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __vmmAbortKillSignal (PLW_VMM_PAGE_FAIL_CTX  pvmpagefailctx)
{
    ULONG            ulAbortType = pvmpagefailctx->PAGEFCTX_ulAbortType;
    struct sigevent  sigeventAbort;
    INT              iSigCode;
    
    switch (ulAbortType) {
    
    case 0:
        sigeventAbort.sigev_signo = SIGKILL;                            /*  ͨ�� SIGKILL �ź�ɱ������   */
        iSigCode = SI_KILL;
        break;
    
    case LW_VMM_ABORT_TYPE_FPE:
        sigeventAbort.sigev_signo = SIGFPE;
        iSigCode = FPE_INTDIV;                                          /*  Ĭ��Ϊ�� 0 ��Ҫ������ϸ�ж� */
        break;
        
    case LW_VMM_ABORT_TYPE_BUS:
        sigeventAbort.sigev_signo = SIGBUS;
        iSigCode = BUS_ADRALN;
        break;
        
    case LW_VMM_ABORT_TYPE_SYS:
        sigeventAbort.sigev_signo = SIGSYS;
        iSigCode = SI_MESGQ;
        break;
        
    case LW_VMM_ABORT_TYPE_UNDEF:
        sigeventAbort.sigev_signo = SIGILL;
        iSigCode = ILL_ILLOPC;
        break;
        
    case LW_VMM_ABORT_TYPE_MAP:
        sigeventAbort.sigev_signo = SIGSEGV;
        iSigCode = SEGV_MAPERR;
        
    case LW_VMM_ABORT_TYPE_EXEC:                                        /*  �� default �����֧         */
    default:
        sigeventAbort.sigev_signo = SIGSEGV;
        iSigCode = SEGV_ACCERR;
        break;
    }

    sigeventAbort.sigev_value.sival_ptr = (PVOID)pvmpagefailctx->PAGEFCTX_ulAbortAddr;
    sigeventAbort.sigev_notify          = SIGEV_SIGNAL;
    
    _doSigEvent(pvmpagefailctx->PAGEFCTX_ulSelf, &sigeventAbort, iSigCode);
    
    if (__KERNEL_ISENTER()) {                                           /*  ������������                */
        for (;;);
    }
    
    for (;;) {                                                          /*  1.0.0-rc36 ��ʼ���ﲻ�˳�   */
        API_TimeSleep(__ARCH_ULONG_MAX);                                /*  �ȴ���ɾ��                  */
    }
}
/*********************************************************************************************************
** ��������: __vmmAbortAccess
** ��������: �����ʹ����� (�˺������᷵��)
** �䡡��  : pvmpagefailctx    page fail ������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __vmmAbortAccess (PLW_VMM_PAGE_FAIL_CTX  pvmpagefailctx)
{
    INT     iRet;
    ULONG   ulErrorOrig = API_GetLastError();
    
    iRet = dtrace_trap(pvmpagefailctx->PAGEFCTX_pvStackRet,
                       pvmpagefailctx->PAGEFCTX_ulAbortAddr,
                       pvmpagefailctx->PAGEFCTX_ulAbortType,
                       pvmpagefailctx->PAGEFCTX_ulSelf);                /*  ѯ�� dtrace ��Ϣ            */
    if (iRet < 0) {                                                     /*  ���Ƕϵ�                    */
        if (pvmpagefailctx->PAGEFCTX_ulAbortType == LW_VMM_ABORT_TYPE_UNDEF) {
            
            printk(KERN_EMERG "thread 0x%lx undefine-instruction, address : 0x%lx.\n",
                   pvmpagefailctx->PAGEFCTX_ulSelf, 
                   pvmpagefailctx->PAGEFCTX_ulAbortAddr);               /*  �����쳣                    */
                   
        } else if (pvmpagefailctx->PAGEFCTX_ulAbortType == LW_VMM_ABORT_TYPE_FPE) {
#if LW_CFG_CPU_FPU_EN > 0 && LW_CFG_DEVICE_EN > 0
            PLW_CLASS_TCB   ptcbCur;
            LW_TCB_GET_CUR_SAFE(ptcbCur);
            __ARCH_FPU_CTX_SHOW(ioGlobalStdGet(STD_ERR), ptcbCur->TCB_pvStackFP);
#endif                                                                  /*  LW_CFG_CPU_FPU_EN > 0       */
            printk(KERN_EMERG "thread 0x%lx float-point exception, address : 0x%lx.\n",
                   pvmpagefailctx->PAGEFCTX_ulSelf, 
                   pvmpagefailctx->PAGEFCTX_ulAbortAddr);               /*  �����쳣                    */
        
        } else {
#if LW_CFG_DEVICE_EN > 0
            archTaskCtxShow(ioGlobalStdGet(STD_ERR), (PSTACK)pvmpagefailctx->PAGEFCTX_pvStackRet);
#endif
            printk(KERN_EMERG "thread 0x%lx abort, address : 0x%lx.\n",
                   pvmpagefailctx->PAGEFCTX_ulSelf, 
                   pvmpagefailctx->PAGEFCTX_ulAbortAddr);               /*  �����쳣                    */
        }
        
        __vmmAbortKillSignal(pvmpagefailctx);                           /*  �����쳣�ź�                */
    }
    
    _ErrorHandle(ulErrorOrig);                                          /*  �ָ�֮ǰ�� errno            */
    
    /*
     * iRet == 0 or iRet > 0 ����Ҫ��������ִ��ָ��.
     */
    archSigCtxLoad(pvmpagefailctx->PAGEFCTX_pvStackRet);                /*  �� page fail �������з���   */
}
/*********************************************************************************************************
** ��������: API_VmmAbortIsr
** ��������: �� MMU ��������ʧЧʱ, ���ô˺���(�������жϷ�����)
** �䡡��  : ulAbortAddr   ���ַ����쳣���ڴ��ַ.
**           ulAbortType   �쳣����
**           ptcb          �����쳣���߳̿��ƿ� (����Ϊ NULL)
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : �� 1.0.0.rc20 �汾��, �˺����������� VMM ����.

                                           API ����
*********************************************************************************************************/
LW_API 
VOID  API_VmmAbortIsr (addr_t  ulAbortAddr, ULONG  ulAbortType, PLW_CLASS_TCB  ptcb)
{
    PLW_VMM_PAGE_FAIL_CTX    pvmpagefailctx;
    PSTACK                   pstkFailShell;                             /*  ���� fail shell �Ķ�ջ��    */
    
    ULONG                    ulNesting = LW_CPU_GET_CUR_NESTING();
    BYTE                    *pucStkNow;                                 /*  ��¼��ԭ��ջ��              */
    
    if (ulNesting > 1) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "abort occur in ISR mode.\r\n");
        API_KernelReboot(LW_REBOOT_FORCE);                              /*  ֱ��������������ϵͳ        */
        return;
    }
    
    __KERNEL_ENTER();                                                   /*  �����ں�                    */
    /*
     *  1. ����ȱҳ�ж��߳�ִ������, ��������������е�ҳ�����, ���ʧ��, �߳���������.
     *  2. ע��, ����ִ����Ϻ�, ϵͳ�����ܹ��ص��ոշ����ڴ沢�����쳣������ָ��.
     *  3. ���ڲ���ȱҳ�ж�ʱ, ����߳�һ���Ǿ�����, �������ﲻ�ü������������.
     */
    if (ulNesting == 0) {                                               /*  ����״̬�µ���              */
        LW_VMM_PAGE_FAIL_CTX    vmpagefailctx;
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
        vmpagefailctx.PAGEFCTX_ulSelf      = ptcb->TCB_ulId;
        vmpagefailctx.PAGEFCTX_ulAbortAddr = ulAbortAddr;
        vmpagefailctx.PAGEFCTX_ulAbortType = ulAbortType;
        vmpagefailctx.PAGEFCTX_pvStackRet  = ptcb->TCB_pstkStackNow;
#if LW_CFG_VMM_EN > 0
        __vmmAbortShell(&vmpagefailctx);
#else
        __vmmAbortAccess(&vmpagefailctx);
#endif
        return;
    }
    
    pucStkNow = (BYTE *)ptcb->TCB_pstkStackNow;                         /*  ��¼��ԭ��ջ��              */
#if	CPU_STK_GROWTH == 0
    pucStkNow      += sizeof(STACK);                                    /*  ���ջ�����ƶ�һ����ջ�ռ�  */
    pvmpagefailctx  = (PLW_VMM_PAGE_FAIL_CTX)pucStkNow;                 /*  ��¼ PAGE_FAIL_CTX λ��     */
    pucStkNow      += __PAGEFAILCTX_SIZE_ALIGN;                         /*  �ó� PAGE_FAIL_CTX �ռ�     */
#else
    pucStkNow      -= __PAGEFAILCTX_SIZE_ALIGN;                         /*  �ó� PAGE_FAIL_CTX �ռ�     */
    pvmpagefailctx  = (PLW_VMM_PAGE_FAIL_CTX)pucStkNow;                 /*  ��¼ PAGE_FAIL_CTX λ��     */
    pucStkNow      -= sizeof(STACK);                                    /*  ���ջ�����ƶ�һ����ջ�ռ�  */
#endif

    pvmpagefailctx->PAGEFCTX_ulSelf       = ptcb->TCB_ulId;
    pvmpagefailctx->PAGEFCTX_ulAbortAddr  = ulAbortAddr;
    pvmpagefailctx->PAGEFCTX_ulAbortType  = ulAbortType;
    pvmpagefailctx->PAGEFCTX_pvStackRet   = ptcb->TCB_pstkStackNow;
    pvmpagefailctx->PAGEFCTX_iLastErrno   = (errno_t)ptcb->TCB_ulLastError;
    pvmpagefailctx->PAGEFCTX_iKernelSpace = __KERNEL_SPACE_GET2(ptcb);

#if LW_CFG_VMM_EN > 0
    pstkFailShell = archTaskCtxCreate((PTHREAD_START_ROUTINE)__vmmAbortShell, 
                                      (PVOID)pvmpagefailctx,
                                      (PSTACK)pucStkNow,
                                      0);                               /*  ����ȱҳ����������ǻ���    */
#else
    pstkFailShell = archTaskCtxCreate((PTHREAD_START_ROUTINE)__vmmAbortAccess, 
                                      (PVOID)pvmpagefailctx,
                                      (PSTACK)pucStkNow,
                                      0);                               /*  ���������쳣������ǻ���    */
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
    
    archTaskCtxSetFp(pstkFailShell, ptcb->TCB_pstkStackNow);            /*  ���� fp, ʹ callstack ����  */
    
    ptcb->TCB_pstkStackNow = pstkFailShell;                             /*  ���潨���õ�������Ƕ�ջ    */
    
    _StackCheckGuard(ptcb);                                             /*  ��ջ������                */
    
    __KERNEL_EXIT();                                                    /*  �˳��ں�                    */
}
/*********************************************************************************************************
** ��������: API_VmmAbortStatus
** ��������: ϵͳ������ֹ״̬. 
** �䡡��  : pi64AbortCounter        �ڴ������ֹ����(�쳣��������)
**           pi64PageFailCounter     ϵͳ���������ȱҳ�ж�����
**           pi64PageLackCounter      ϵͳȱ������ҳ�����
**           pi64MapErrCounter       ӳ���������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
#if LW_CFG_VMM_EN > 0

LW_API 
VOID  API_VmmAbortStatus (INT64     *pi64AbortCounter,
                          INT64     *pi64PageFailCounter,
                          INT64     *pi64PageLackCounter,
                          INT64     *pi64MapErrCounter)
{
    __VMM_LOCK();
    if (pi64AbortCounter) {
        *pi64AbortCounter = _K_i64VmmAbortCounter;
    }
    if (pi64PageFailCounter) {
        *pi64PageFailCounter = _K_i64VmmPageFailCounter;
    }
    if (pi64PageLackCounter) {
        *pi64PageLackCounter = _K_i64VmmPageLackCounter;
    }
    if (pi64MapErrCounter) {
        *pi64MapErrCounter = _K_i64VmmMapErrCounter;
    }
    __VMM_UNLOCK();
    
    _ErrorHandle(ERROR_NONE);
}

#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
