/*********************************************************************************************************
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
** ��   ��   ��: mman.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2010 �� 08 �� 12 ��
**
** ��        ��: POSIX IEEE Std 1003.1, 2004 Edition sys/mman.h

** BUG:
2011.03.04  ��� vmmMalloc û�гɹ���ת��ʹ�� __SHEAP_ALLOC().
2011.05.16  mprotect() ʹ�� vmmSetFlag() �ӿڲ���.
            ֧�ַ� REG �ļ������ļ� mmap() ����.
2011.05.17  ֧�� mmap() ��׼, ���ȷ�������ռ�, ����ȱҳ�ж�ʱ, ���������ڴ�.
2011.07.27  �� mmap() ȱҳ�ж�����ļ�����û�дﵽһҳʱ, ��Ҫ��� 0 .
            ֧���豸�ļ��� mmap() ����.
2011.08.03  ����ļ� stat ʹ�� fstat api.
2011.12.09  mmap() ��亯��ʹ�� iosRead() ���.
            ���� mmapShow() ����, �鿴 mmap() �ڴ����.
2012.01.10  ����� MAP_ANONYMOUS ��֧��.
2012.08.16  ֱ��ʹ�� pread/pwrite �����ļ�.
2012.10.19  mmap() �� munmap() ������ļ����������õĴ���.
2012.12.07  �� mmap ������Դ������.
2012.12.22  ӳ��ʱ, ������ǽ���ӳ��� pid ���ж�Ϊ������Դ, ���ﲻ���ٶ��ļ�������, 
            ��Ϊ��ǰ���ļ����������Ѳ����Ǵ������̵��ļ���������. �������������ص��ļ�������.
2012.12.27  mmap ����� reg �ļ�, ҲҪ������һ�¶�Ӧ�ļ�ϵͳ�� mmap ����, �����������ȱҳ���.
            ����� shm_open shm_unlink ��֧��.
2012.12.30  mmapShow() ������ʾ�ļ���, ������� pid ����ʾ.
2013.01.02  ����ӳ��ʱ��Ҫ���������� unmap ����.
2013.01.12  munmap �������ͬһ����, �򲻲����ļ��������Ͷ�Ӧ����.
2013.03.12  ���� mmap64.
2013.03.16  mmap ����豸������֧����Ҳ����ӳ��.
2013.03.17  mmap ���� MAP_SHARED ��־�����Ƿ�ʹ�� CACHE.
2013.09.13  ֧�� MAP_SHARED.
            msync() ��������ҳ���ҿ�дʱ�Ż�д.
2013.12.21  ֧�� PROT_EXEC.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "unistd.h"
#include "../include/px_mman.h"                                         /*  �Ѱ�������ϵͳͷ�ļ�        */
#include "../include/posixLib.h"                                        /*  posix �ڲ�������            */
/*********************************************************************************************************
  ����ü�֧��
*********************************************************************************************************/
#if (LW_CFG_POSIX_EN > 0) && (LW_CFG_DEVICE_EN > 0)
/*********************************************************************************************************
** ��������: mlock
** ��������: ����ָ���ڴ��ַ�ռ䲻���л�ҳ����.
** �䡡��  : pvAddr        ��ʼ��ַ
**           stLen         �ڴ�ռ䳤��
** �䡡��  : ERROR_NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
int  mlock (const void  *pvAddr, size_t  stLen)
{
    (VOID)pvAddr;
    (VOID)stLen;
    
    if (geteuid() != 0) {
        errno = EACCES;
        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: munlock
** ��������: ����ָ���ڴ��ַ�ռ�, ������л�ҳ����.
** �䡡��  : pvAddr        ��ʼ��ַ
**           stLen         �ڴ�ռ䳤��
** �䡡��  : ERROR_NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
int  munlock (const void  *pvAddr, size_t  stLen)
{
    (VOID)pvAddr;
    (VOID)stLen;
    
    if (geteuid() != 0) {
        errno = EACCES;
        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: mlockall
** ��������: �������̿ռ䲻���л�ҳ����.
** �䡡��  : iFlag         ����ѡ�� MCL_CURRENT/MCL_FUTURE
** �䡡��  : ERROR_NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
int  mlockall (int  iFlag)
{
    (VOID)iFlag;
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: munlockall
** ��������: �������̿ռ�, ������л�ҳ����.
** �䡡��  : NONE
** �䡡��  : ERROR_NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
int  munlockall (void)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: mprotect
** ��������: ���ý�����ָ���ĵ�ַ�� page flag
** �䡡��  : pvAddr        ��ʼ��ַ
**           stLen         ����
**           iProt         �µ�����
** �䡡��  : ERROR_NONE or PX_ERROR
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
int  mprotect (void  *pvAddr, size_t  stLen, int  iProt)
{
#if LW_CFG_VMM_EN > 0
    ULONG   ulFlag = LW_VMM_FLAG_READ;
    
    
    if (!ALIGNED(pvAddr, LW_CFG_VMM_PAGE_SIZE) || stLen == 0) {
        errno = EINVAL;
        return  (PX_ERROR);
    }
    
    if (iProt & (~(PROT_READ | PROT_WRITE | PROT_EXEC))) {
        errno = ENOTSUP;
        return  (PX_ERROR);
    }
    
    if (iProt) {
        if (iProt & PROT_WRITE) {
            ulFlag |= LW_VMM_FLAG_RDWR;                                 /*  �ɶ�д                      */
        }
        if (iProt & PROT_EXEC) {
            ulFlag |= LW_VMM_FLAG_EXEC;                                 /*  ��ִ��                      */
        }
    } else {
        ulFlag |= LW_VMM_FLAG_FAIL;                                     /*  ���������                  */
    }
    
    if (API_VmmSetFlag(pvAddr, ulFlag) == ERROR_NONE) {                 /*  ���������µ� flag           */
        return  (ERROR_NONE);
    } else {
        return  (PX_ERROR);
    }
    
#else
    return  (ERROR_NONE);
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
}
/*********************************************************************************************************
  mmap ���ƿ�ṹ
*********************************************************************************************************/
typedef struct {
    LW_LIST_LINE            PMAPN_lineManage;                           /*  ˫������                    */
    PVOID                   PMAPN_pvAddr;                               /*  ��ʼ��ַ                    */
    size_t                  PMAPN_stLen;                                /*  �ڴ泤��                    */
    ULONG                   PMAPN_ulFlag;                               /*  �ڴ�����                    */
    INT                     PMAPN_iFd;                                  /*  �����ļ�                    */
    mode_t                  PMAPN_mode;                                 /*  �ļ�mode                    */
    off_t                   PMAPN_off;                                  /*  �ļ�ӳ��ƫ����              */
    off_t                   PMAPN_offFSize;                             /*  �ļ���С                    */
    INT                     PMAPN_iFlag;                                /*  mmap iFlag ����             */
    BOOL                    PMAPN_bBusy;                                /*  æ��־                      */
    BOOL                    PMAPN_bIsHeapMem;                           /*  �Ƿ�Ϊ heap ��������ڴ�    */
    
    dev_t                   PMAPN_dev;                                  /*  �ļ���������Ӧ dev          */
    ino64_t                 PMAPN_ino64;                                /*  �ļ���������Ӧ inode        */
    
    LW_LIST_LINE_HEADER     PMAPN_plineUnmap;                           /*  �Ѿ� unmap ������           */
    size_t                  PMAPN_stTotalUnmap;                         /*  �Ѿ� unmap ���ܴ�С         */
    
    pid_t                   PMAPN_pid;                                  /*  ӳ����̵Ľ��̺�            */
    LW_RESOURCE_RAW         PMAPN_resraw;                               /*  ��Դ����ڵ�                */
} __PX_MAP_NODE;

typedef struct {
    LW_LIST_LINE            PMAPA_lineManage;                           /*  ˫������                    */
                                                                        /*  ��������ַ��С��������    */
    PVOID                   PMAPN_pvAddr;                               /*  ��ʼ��ַ                    */
    size_t                  PMAPN_stLen;                                /*  �ڴ泤��                    */
} __PX_MAP_AREA;
/*********************************************************************************************************
  mmap ���л���������
*********************************************************************************************************/
static LW_LIST_LINE_HEADER  _G_plineMMapHeader = LW_NULL;
/*********************************************************************************************************
** ��������: __mmapNodeFind
** ��������: ���� mmap node
** �䡡��  : pvAddr        ��ַ
** �䡡��  : mmap node
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static __PX_MAP_NODE  *__mmapNodeFind (void  *pvAddr)
{
    __PX_MAP_NODE      *pmapn;
    PLW_LIST_LINE       plineTemp;
    
    for (plineTemp  = _G_plineMMapHeader;
         plineTemp != LW_NULL;
         plineTemp  = _list_line_get_next(plineTemp)) {
         
        pmapn = (__PX_MAP_NODE *)plineTemp;
        
        if (((addr_t)pvAddr >= (addr_t)pmapn->PMAPN_pvAddr) && 
            ((addr_t)pvAddr <  ((addr_t)pmapn->PMAPN_pvAddr + pmapn->PMAPN_stLen))) {
            return  (pmapn);
        }
    }
    
    return  (LW_NULL);
}
/*********************************************************************************************************
** ��������: __mmapNodeFindShare
** ��������: ���� mmap ���� share �Ľڵ�.
** �䡡��  : pmapnAbort         mmap node
**           ulAbortAddr        ����ȱҳ�жϵĵ�ַ (ҳ�����)
**           pfuncCallback      �ص�����
** �䡡��  : �ص���������ֵ
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
#if LW_CFG_VMM_EN > 0

static PVOID __mmapNodeFindShare (__PX_MAP_NODE  *pmapnAbort,
                                  addr_t          ulAbortAddr,
                                  PVOID         (*pfuncCallback)(PVOID  pvStartAddr, size_t  stOffset))
{
    __PX_MAP_NODE      *pmapn;
    PLW_LIST_LINE       plineTemp;
    PVOID               pvRet = LW_NULL;
    off_t               oft;                                            /*  һ���� VMM ҳ�����         */
    
    oft = ((off_t)(ulAbortAddr - (addr_t)pmapnAbort->PMAPN_pvAddr) + pmapnAbort->PMAPN_off);
    
    for (plineTemp  = _G_plineMMapHeader;
         plineTemp != LW_NULL;
         plineTemp  = _list_line_get_next(plineTemp)) {
         
        pmapn = (__PX_MAP_NODE *)plineTemp;
        
        if (pmapn != pmapnAbort) {
            if ((pmapn->PMAPN_iFd      >= 0) &&
                (pmapnAbort->PMAPN_iFd >= 0) &&
                (pmapn->PMAPN_dev   == pmapnAbort->PMAPN_dev) &&
                (pmapn->PMAPN_ino64 == pmapnAbort->PMAPN_ino64)) {      /*  ӳ����ļ���ͬ              */
                
                
                if ((oft >= pmapn->PMAPN_off) &&
                    (oft <  (pmapn->PMAPN_off + pmapn->PMAPN_stLen))) { /*  ��Χ����                    */
                    
                    pvRet = pfuncCallback(pmapn->PMAPN_pvAddr,
                                          (size_t)(oft - pmapn->PMAPN_off));
                    if (pvRet) {
                        break;
                    }
                }
            }
        }
    }
    
    return  (pvRet);
}

#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
/*********************************************************************************************************
** ��������: __mmapNodeAreaInsert
** ��������: ��һ���ͷŵ������¼���� mmap node �����
** �䡡��  : pmapn     mmap node
**           pmaparea  �����¼
** �䡡��  : ���������ַ��ͻ, ���� PX_ERROR
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  __mmapNodeAreaInsert (__PX_MAP_NODE  *pmapn, __PX_MAP_AREA  *pmaparea)
{
             PLW_LIST_LINE       plineTemp;
             
    REGISTER __PX_MAP_AREA      *pmapareaTemp;
    REGISTER __PX_MAP_AREA      *pmapareaLeft;
    REGISTER __PX_MAP_AREA      *pmapareaRight;
    
    if (pmapn->PMAPN_plineUnmap == LW_NULL) {                           /*  ��û�� unmap ��             */
        _List_Line_Add_Ahead(&pmaparea->PMAPA_lineManage, 
                             &pmapn->PMAPN_plineUnmap);                 /*  �����ͷ                    */
        return  (ERROR_NONE);
    }
    
    for (plineTemp  = pmapn->PMAPN_plineUnmap;
         plineTemp != LW_NULL;
         plineTemp  = _list_line_get_next(plineTemp)) {
    
        pmapareaTemp = (__PX_MAP_AREA *)plineTemp;
        
        if ((addr_t)pmapareaTemp->PMAPN_pvAddr > (addr_t)pmaparea->PMAPN_pvAddr) {
            
            pmapareaRight = pmapareaTemp;                               /*  �������ǿ���Ķ���...       */
            /*
             *  ��⵱ǰ�����Ƿ����ұ��������ص�
             */
            if (((addr_t)pmaparea->PMAPN_pvAddr + pmaparea->PMAPN_stLen) >
                (addr_t)pmapareaRight->PMAPN_pvAddr) {
                return  (PX_ERROR);                                     /*  ���ص�                      */
            
            } else {
                _List_Line_Add_Left(&pmaparea->PMAPA_lineManage, 
                                    &pmapareaRight->PMAPA_lineManage);  /*  �嵽���                    */
                return  (ERROR_NONE);
            }
        } else {
            pmapareaLeft = pmapareaTemp;                                /*  �������ǿ���Ķ���...       */
            /*
             *  ����Ƿ��������ص�
             */
            if (((addr_t)pmapareaLeft->PMAPN_pvAddr + pmapareaLeft->PMAPN_stLen) >
                (addr_t)pmaparea->PMAPN_pvAddr) {
                return  (PX_ERROR);                                     /*  ���ص�                      */
            }
        }
    }
    
    /*
     *  ��ǰ�ڵ�ĵ�ַ�����еĽڵ㶼��
     */
    if (((addr_t)pmaparea->PMAPN_pvAddr + pmaparea->PMAPN_stLen) >
        ((addr_t)pmapn->PMAPN_pvAddr + pmapn->PMAPN_stLen)) {           /*  ���ܳ��� mmap ӳ�䷶Χ      */
        return  (PX_ERROR);
    
    } else {
        _List_Line_Add_Right(&pmaparea->PMAPA_lineManage,
                             &pmapareaTemp->PMAPA_lineManage);          /*  ���뵽���һ���ڵ��ұ�      */
        return  (ERROR_NONE);
    }
}
/*********************************************************************************************************
** ��������: __mmapNodeFree
** ��������: �ͷ�һ��ָ�� mmap node ���е���������ڴ�
** �䡡��  : pmapn     mmap node
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __mmapNodeAreaFree (__PX_MAP_NODE  *pmapn)
{
    REGISTER PLW_LIST_LINE  plineDel;
    
    while (pmapn->PMAPN_plineUnmap) {                                   /*  ɾ�����е�����ڵ�          */
        plineDel = pmapn->PMAPN_plineUnmap;
        _List_Line_Del(plineDel, &pmapn->PMAPN_plineUnmap);
        __SHEAP_FREE(plineDel);
    }
}
/*********************************************************************************************************
** ��������: __mmapMallocAreaFill
** ��������: ȱҳ�жϷ����ڴ��, ��ͨ���˺�������ļ����� (ע��, �˺����� vmm lock ��ִ��!)
** �䡡��  : pmapn              mmap node
**           ulDestPageAddr     ����Ŀ���ַ (ҳ�����)
**           ulMapPageAddr      ���ջᱻӳ���Ŀ���ַ (ҳ�����)
**           ulPageNum          �·������ҳ�����
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
#if LW_CFG_VMM_EN > 0

static INT  __mmapMallocAreaFill (__PX_MAP_NODE    *pmapn, 
                                  addr_t            ulDestPageAddr,
                                  addr_t            ulMapPageAddr, 
                                  ULONG             ulPageNum)
{
    off_t       offtRead;
    
    size_t      stReadLen;
    addr_t      ulMapStartAddr = (addr_t)pmapn->PMAPN_pvAddr;
    
    if (pmapn->PMAPN_pid != getpid()) {                                 /*  ������Ǵ�������            */
        goto    __full_with_zero;
    }
    
    if (!S_ISREG(pmapn->PMAPN_mode)) {                                  /*  �������ļ�����              */
        goto    __full_with_zero;
    }
    
    if ((ulMapPageAddr < ulMapStartAddr) || 
        (ulMapPageAddr > (ulMapStartAddr + pmapn->PMAPN_stLen))) {      /*  ��������, ҳ���ڴ��ַ����  */
        goto    __full_with_zero;
    }
    
    offtRead  = (off_t)(ulMapPageAddr - ulMapStartAddr);                /*  �ڴ��ַƫ��                */
    offtRead += pmapn->PMAPN_off;                                       /*  �����ļ���ʼƫ��            */
    
    stReadLen = (size_t)(ulPageNum * LW_CFG_VMM_PAGE_SIZE);             /*  ��Ҫ��ȡ�����ݴ�С          */
    
    /*
     *  ��ʱ�������ļ���ȡ��������. (ע��, �����ǽ��ļ����ݿ��� ulDestPageAddr)
     */
    {
        INT      iZNum;
        ssize_t  sstNum = API_IosPRead(pmapn->PMAPN_iFd, 
                                       (PCHAR)ulDestPageAddr, stReadLen,
                                       offtRead);                       /*  ��ȡ�ļ�����                */
        
        sstNum = (sstNum >= 0) ? sstNum : 0;
        iZNum = (INT)(stReadLen - sstNum);
        
        if (iZNum > 0) {
            lib_bzero((PVOID)(ulDestPageAddr + sstNum), iZNum);         /*  δʹ�ò�������              */
        }
    }
    
    return  (ERROR_NONE);
    
__full_with_zero:
    lib_bzero((PVOID)ulDestPageAddr, 
              (INT)(ulPageNum * LW_CFG_VMM_PAGE_SIZE));                 /*  ȫ������                    */
    
    return  (ERROR_NONE);
}

#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
/*********************************************************************************************************
** ��������: __mmapMalloc
** ��������: �����ڴ�
** �䡡��  : pmapnode      mmap node
**           stLen         �ڴ��С
**           pvArg         ��亯������
**           iFlags        mmap flags
**           ulFlag        �ڴ�����
** �䡡��  : �ڴ��ַ
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static PVOID  __mmapMalloc (__PX_MAP_NODE  *pmapnode, size_t  stLen, PVOID  pvArg, 
                            INT  iFlags, ULONG  ulFlag)
{
#if LW_CFG_VMM_EN > 0
    PVOID    pvMem = API_VmmMallocAreaEx(stLen, __mmapMallocAreaFill, pvArg, iFlags, ulFlag);
    
    if (pvMem == LW_NULL) {
        pvMem =  __SHEAP_ALLOC_ALIGN(stLen, LW_CFG_VMM_PAGE_SIZE);
        pmapnode->PMAPN_bIsHeapMem = LW_TRUE;
    } else {
        pmapnode->PMAPN_bIsHeapMem = LW_FALSE;
        API_VmmSetFindShare(pvMem, __mmapNodeFindShare, pvArg);
    }

#else
    PVOID    pvMem = __SHEAP_ALLOC_ALIGN(stLen, LW_CFG_VMM_PAGE_SIZE);
    
    pmapnode->PMAPN_bIsHeapMem = LW_TRUE;
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */

    return  (pvMem);
}
/*********************************************************************************************************
** ��������: __mmapFree
** ��������: �ڴ��ͷ�
** �䡡��  : pmapnode      mmap node
**           pvAddr        �ڴ��ַ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __mmapFree (__PX_MAP_NODE  *pmapnode, PVOID  pvAddr)
{
#if LW_CFG_VMM_EN > 0
    if (pmapnode->PMAPN_bIsHeapMem == LW_FALSE) {
        API_VmmFreeArea(pvAddr);
    } else {
        __SHEAP_FREE(pvAddr);
    }
#else
    __SHEAP_FREE(pvAddr);
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
}
/*********************************************************************************************************
** ��������: mmapfd
** ��������: ��� mmap ������ļ�������
** �䡡��  : pvAddr        ��ַ
** �䡡��  : �ļ�������
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
int  mmapfd (void  *pvAddr)
{
    __PX_MAP_NODE       *pmapnode;
    int                  iFd;
    
    __PX_LOCK();
    pmapnode = __mmapNodeFind(pvAddr);
    if (pmapnode == LW_NULL) {
        __PX_UNLOCK();
        errno = EINVAL;
        return  (PX_ERROR);
    }
    
    iFd = pmapnode->PMAPN_iFd;
    __PX_UNLOCK();
    
    return  (iFd);
}
/*********************************************************************************************************
** ��������: mmap
** ��������: �ڴ��ļ�ӳ�亯��, ���: http://www.opengroup.org/onlinepubs/000095399/functions/mmap.html
** �䡡��  : pvAddr        ��ʼ��ַ (�������Ϊ NULL, ϵͳ���Զ������ڴ�)
**           stLen         ӳ�䳤��
**           iProt         ҳ������
**           iFlag         ӳ���־
**           iFd           �ļ�������
**           off           �ļ�ƫ����
** �䡡��  : �ļ�ӳ�����ڴ��ַ
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : ����� REG �ļ�, Ҳ����ʹ����������� mmap. �����ڴ��豸�� mmap ֱ�ӻ����ӳ��.
                                           API ����
*********************************************************************************************************/
LW_API  
void  *mmap (void  *pvAddr, size_t  stLen, int  iProt, int  iFlag, int  iFd, off_t  off)
{
    __PX_MAP_NODE       *pmapnode;
    struct stat64        stat64Fd;
    ULONG                ulFlag = LW_VMM_FLAG_READ;
    
#if LW_CFG_VMM_EN > 0
    LW_DEV_MMAP_AREA     dmap;
    INT                  iError;
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
    
    INT                  iErrLevel = 0;

    if (pvAddr != LW_NULL) {                                            /*  �������Ϊ NULL             */
        errno = EINVAL;
        return  (MAP_FAILED);
    }
    
    if (iProt & (~(PROT_READ | PROT_WRITE | PROT_EXEC))) {
        errno = ENOTSUP;
        return  (MAP_FAILED);
    }
    
    if (iProt & PROT_WRITE) {
        ulFlag |= LW_VMM_FLAG_RDWR;                                     /*  �ɶ�д                      */
    }
    
    if (iProt & PROT_EXEC) {
        ulFlag |= LW_VMM_FLAG_EXEC;                                     /*  ��ִ��                      */
    }
    
    if (iFlag & MAP_FIXED) {                                            /*  ��֧�� FIX                  */
        errno = ENOTSUP;
        return  (MAP_FAILED);
    }
    
    if (!ALIGNED(off, LW_CFG_VMM_PAGE_SIZE) || stLen == 0) {            /*  off ����ҳ����              */
        errno = EINVAL;
        return  (MAP_FAILED);
    }
    
    if (!(iFlag & MAP_ANONYMOUS)) {                                     /*  ���ļ����������            */
        if (fstat64(iFd, &stat64Fd) < 0) {                              /*  ����ļ� stat               */
            errno = EBADF;
            return  (MAP_FAILED);
        }
        
        if (off > stat64Fd.st_size) {                                   /*  off Խ��                    */
            errno = ENXIO;
            return  (MAP_FAILED);
        }
        
    } else {
        iFd = PX_ERROR;
        stat64Fd.st_mode = (mode_t)0;                                   /*  �ļ��������޹�              */
    }
    
    if (iFlag & MAP_SHARED) {                                           /*  ����̹���                  */
#if LW_CFG_CACHE_EN > 0
        if (API_CacheLocation(DATA_CACHE) == CACHE_LOCATION_VIVT) {     /*  ������Ϊ�����ַ CACHE      */
            ulFlag &= ~(LW_VMM_FLAG_CACHEABLE | LW_VMM_FLAG_BUFFERABLE);/*  �����ڴ治���� CACHE        */
        }
#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
    }
    
    pmapnode = (__PX_MAP_NODE *)__SHEAP_ALLOC(sizeof(__PX_MAP_NODE));   /*  �������ƿ�                  */
    if (pmapnode == LW_NULL) {
        errno = ENOMEM;
        return  (MAP_FAILED);
    }
    
    pmapnode->PMAPN_pvAddr = __mmapMalloc(pmapnode, stLen, (PVOID)pmapnode, iFlag, ulFlag);
    if (pmapnode->PMAPN_pvAddr == LW_NULL) {                            /*  ����ӳ���ڴ�                */
        errno     = ENOMEM;
        iErrLevel = 1;
        goto    __error_handle;
    }
    
    pmapnode->PMAPN_stLen    = stLen;
    pmapnode->PMAPN_ulFlag   = ulFlag;
    pmapnode->PMAPN_iFd      = iFd;
    pmapnode->PMAPN_mode     = stat64Fd.st_mode;
    pmapnode->PMAPN_off      = off;
    pmapnode->PMAPN_offFSize = stat64Fd.st_size;
    pmapnode->PMAPN_iFlag    = iFlag;
    pmapnode->PMAPN_bBusy    = LW_FALSE;
    
    pmapnode->PMAPN_dev      = stat64Fd.st_dev;
    pmapnode->PMAPN_ino64    = stat64Fd.st_ino;
    
    pmapnode->PMAPN_plineUnmap   = LW_NULL;                             /*  û�� unmap                  */
    pmapnode->PMAPN_stTotalUnmap = 0;
    
    if (!(iFlag & MAP_ANONYMOUS)) {                                     /*  ���ļ����������            */
    
        API_IosFdRefInc(iFd);                                           /*  ���ļ����������� ++         */
    
        if (S_ISREG(stat64Fd.st_mode)) {                                /*  ��ͨ�����ļ�                */
            if (pmapnode->PMAPN_bIsHeapMem) {                           /*  ��ȱҳ�ж��ڴ�              */
                pread(iFd, (PVOID)pmapnode->PMAPN_pvAddr, stLen, off);  /*  ��ȡ�ļ�����                */
            
            } else {                                                    /*  ʹ��ȱҳ�ж�                */
#if LW_CFG_VMM_EN > 0
                dmap.DMAP_pvAddr   = pmapnode->PMAPN_pvAddr;
                dmap.DMAP_stLen    = pmapnode->PMAPN_stLen;
                dmap.DMAP_offPages = (pmapnode->PMAPN_off >> LW_CFG_VMM_PAGE_SHIFT);
                dmap.DMAP_ulFlag   = ulFlag;
                
                iError = API_IosMmap(iFd, &dmap);                       /*  ���Ե����豸����            */
                if ((iError < ERROR_NONE) && 
                    (errno != ERROR_IOS_DRIVER_NOT_SUP)) {              /*  �������򱨸����            */
                    iErrLevel = 2;
                    goto    __error_handle;
                }
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
            }
        } else {                                                        /*  �� REG �ļ�                 */
            if (pmapnode->PMAPN_bIsHeapMem) {
                errno     = ENOSYS;
                iErrLevel = 2;
                goto    __error_handle;
            }
#if LW_CFG_VMM_EN > 0
            dmap.DMAP_pvAddr   = pmapnode->PMAPN_pvAddr;
            dmap.DMAP_stLen    = pmapnode->PMAPN_stLen;
            dmap.DMAP_offPages = (pmapnode->PMAPN_off >> LW_CFG_VMM_PAGE_SHIFT);
            dmap.DMAP_ulFlag   = ulFlag;
            
            iError = API_IosMmap(iFd, &dmap);                           /*  �����豸����                */
            if ((iError < ERROR_NONE) && 
                (errno != ERROR_IOS_DRIVER_NOT_SUP)) {                  /*  �������򱨸����            */
                iErrLevel = 2;
                goto    __error_handle;
            }
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
        }
    }
    
    __PX_LOCK();
    _List_Line_Add_Ahead(&pmapnode->PMAPN_lineManage, 
                         &_G_plineMMapHeader);                          /*  �����������                */
    __PX_UNLOCK();
    
    pmapnode->PMAPN_pid = getpid();                                     /*  ��õ�ǰ���� pid            */
    __resAddRawHook(&pmapnode->PMAPN_resraw, (VOIDFUNCPTR)munmap, 
                    pmapnode->PMAPN_pvAddr, (PVOID)stLen, 0, 0, 0, 0);  /*  ������Դ������              */

    MONITOR_EVT_LONG5(MONITOR_EVENT_ID_VMM, MONITOR_EVENT_VMM_MMAP,
                      pmapnode->PMAPN_pvAddr, 
                      iFd, stLen, iProt, iFlag, LW_NULL);

    return  (pmapnode->PMAPN_pvAddr);
    
__error_handle:
    if (iErrLevel > 1) {
        API_IosFdRefDec(iFd);                                           /*  ���ļ����������� --         */
        __mmapFree(pmapnode, pmapnode->PMAPN_pvAddr);
    }
    if (iErrLevel > 0) {
        __SHEAP_FREE(pmapnode);
    }
    
    return  (MAP_FAILED);
}
/*********************************************************************************************************
** ��������: mmap64 (sylixos �ڲ� off_t ������� 64bit)
** ��������: �ڴ��ļ�ӳ�亯��, ���: http://www.opengroup.org/onlinepubs/000095399/functions/mmap.html
** �䡡��  : pvAddr        ��ʼ��ַ (�������Ϊ NULL, ϵͳ���Զ������ڴ�)
**           stLen         ӳ�䳤��
**           iProt         ҳ������
**           iFlag         ӳ���־
**           iFd           �ļ�������
**           off           �ļ�ƫ����
** �䡡��  : �ļ�ӳ�����ڴ��ַ
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : ����� REG �ļ�, Ҳ����ʹ����������� mmap. �����ڴ��豸�� mmap ֱ�ӻ����ӳ��.
                                           API ����
*********************************************************************************************************/
LW_API  
void  *mmap64 (void  *pvAddr, size_t  stLen, int  iProt, int  iFlag, int  iFd, off64_t  off)
{
    return  (mmap(pvAddr, stLen, iProt, iFlag, iFd, (off_t)off));
}
/*********************************************************************************************************
** ��������: munmap
** ��������: ȡ���ڴ��ļ�ӳ��
** �䡡��  : pvAddr        ��ʼ��ַ
**           stLen         ӳ�䳤��
** �䡡��  : ERROR_NONE or PX_ERROR
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
int  munmap (void  *pvAddr, size_t  stLen)
{
    __PX_MAP_NODE       *pmapnode;
    __PX_MAP_AREA       *pmaparea;
    
#if LW_CFG_VMM_EN > 0
    LW_DEV_MMAP_AREA     dmap;
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
    
    
    if (!ALIGNED(pvAddr, LW_CFG_VMM_PAGE_SIZE) || stLen == 0) {         /*  pvAddr ����ҳ����           */
        errno = EINVAL;
        return  (PX_ERROR);
    }
    
    pmaparea = (__PX_MAP_AREA *)__SHEAP_ALLOC(sizeof(__PX_MAP_AREA));   /*  �����ڴ�����ڵ�            */
    if (pmaparea == LW_NULL) {
        errno = ENOMEM;
        return  (PX_ERROR);
    }
    
    __PX_LOCK();
    pmapnode = __mmapNodeFind(pvAddr);
    if (pmapnode == LW_NULL) {
        __PX_UNLOCK();
        __SHEAP_FREE(pmaparea);
        errno = EINVAL;
        return  (PX_ERROR);
    }
    if (pmapnode->PMAPN_bBusy) {
        __PX_UNLOCK();
        __SHEAP_FREE(pmaparea);
        errno = EBUSY;
        return  (PX_ERROR);
    }
    
    pmaparea->PMAPN_pvAddr = pvAddr;
    pmaparea->PMAPN_stLen  = stLen;
    
    if (__mmapNodeAreaInsert(pmapnode, pmaparea) < ERROR_NONE) {
        __PX_UNLOCK();
        __SHEAP_FREE(pmaparea);
        errno = EINVAL;                                                 /*  ���ӳ����ڴ��������      */
        return  (PX_ERROR);
    }
    
    pmapnode->PMAPN_stTotalUnmap += stLen;
    
    if (pmapnode->PMAPN_stTotalUnmap >= pmapnode->PMAPN_stLen) {        /*  ���е��ڴ涼�� unmap ��     */
        _List_Line_Del(&pmapnode->PMAPN_lineManage, 
                       &_G_plineMMapHeader);                            /*  �ӹ���������ɾ��            */
        __PX_UNLOCK();
        
        if ((pmapnode->PMAPN_iFd >= 0) &&
            (pmapnode->PMAPN_pid == getpid())) {                        /*  ע��: ���Ǵ������̲���������*/
            
#if LW_CFG_VMM_EN > 0
            dmap.DMAP_pvAddr   = pmapnode->PMAPN_pvAddr;
            dmap.DMAP_stLen    = pmapnode->PMAPN_stLen;
            dmap.DMAP_offPages = (pmapnode->PMAPN_off >> LW_CFG_VMM_PAGE_SHIFT);
            dmap.DMAP_ulFlag   = pmapnode->PMAPN_ulFlag;
            API_IosUnmap(pmapnode->PMAPN_iFd, &dmap);                   /*  �����豸����                */
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
            
            API_IosFdRefDec(pmapnode->PMAPN_iFd);                       /*  ���ļ����������� --         */
        }
        
        __mmapNodeAreaFree(pmapnode);                                   /*  �ͷ���������ڴ�            */
        
        __mmapFree(pmapnode, pmapnode->PMAPN_pvAddr);                   /*  �ͷ��ڴ�                    */
        
#if LW_CFG_MODULELOADER_EN > 0
        __resDelRawHook(&pmapnode->PMAPN_resraw);
#endif                                                                  /*  LW_CFG_MODULELOADER_EN      */
        
        __SHEAP_FREE(pmapnode);
        
        MONITOR_EVT_LONG2(MONITOR_EVENT_ID_VMM, MONITOR_EVENT_VMM_MUNMAP,
                          pvAddr, stLen, LW_NULL);
        
        return  (ERROR_NONE);
    
    } else {
        pmapnode->PMAPN_bBusy = LW_TRUE;                                /*  ����Ϊæ״̬                */
        __PX_UNLOCK();
        
#if LW_CFG_VMM_EN > 0
        API_VmmInvalidateArea(pmapnode->PMAPN_pvAddr, pvAddr, stLen);   /*  �ͷ�������������ڴ�        */
                                                                        /*  SylixOS �ݲ�֧������ռ��ɢ*/
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
        pmapnode->PMAPN_bBusy = LW_FALSE;
        
        return  (ERROR_NONE);
    }
}
/*********************************************************************************************************
** ��������: msync
** ��������: ���ڴ���ӳ����ļ����ݻ�д�ļ�
** �䡡��  : pvAddr        ��ʼ��ַ
**           stLen         ӳ�䳤��
**           iFlag         ��д����
** �䡡��  : ERROR_NONE or PX_ERROR
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
int  msync (void  *pvAddr, size_t  stLen, int  iFlag)
{
    __PX_MAP_NODE       *pmapnode;
    off_t                off;
    size_t               stWriteLen;
    
    if (!ALIGNED(pvAddr, LW_CFG_VMM_PAGE_SIZE) || stLen == 0) {         /*  pvAddr ����ҳ����           */
        errno = EINVAL;
        return  (PX_ERROR);
    }
    
    __THREAD_CANCEL_POINT();                                            /*  ����ȡ����                  */
    
    __PX_LOCK();
    pmapnode = __mmapNodeFind(pvAddr);
    if (pmapnode == LW_NULL) {
        __PX_UNLOCK();
        errno = EINVAL;
        return  (PX_ERROR);
    }
    if (pmapnode->PMAPN_bBusy) {
        __PX_UNLOCK();
        errno = EBUSY;
        return  (PX_ERROR);
    }
    if (!S_ISREG(pmapnode->PMAPN_mode)) {
        __PX_UNLOCK();
        return  (ERROR_NONE);                                           /*  �������ļ�ֱ���˳�          */
    }
    if (((addr_t)pvAddr + stLen) > 
        ((addr_t)pmapnode->PMAPN_pvAddr + pmapnode->PMAPN_stLen)) {
        __PX_UNLOCK();
        errno = ENOMEM;                                                 /*  ��Ҫͬ�����ڴ�Խ��          */
        return  (PX_ERROR);
    }
    pmapnode->PMAPN_bBusy = LW_TRUE;                                    /*  ����Ϊæ״̬                */
    __PX_UNLOCK();
    
    off = pmapnode->PMAPN_off 
        + (off_t)((addr_t)pvAddr - (addr_t)pmapnode->PMAPN_pvAddr);     /*  ����д��/��Ч�ļ�ƫ����     */
        
    stWriteLen = (size_t)(pmapnode->PMAPN_offFSize - off);
    stWriteLen = (stWriteLen > stLen) ? stLen : stWriteLen;             /*  ȷ��д��/��Ч�ļ��ĳ���     */
    
    if (!(iFlag & MS_INVALIDATE)) {
                 INT        i;
        REGISTER ULONG      ulPageNum = (ULONG) (stWriteLen >> LW_CFG_VMM_PAGE_SHIFT);
        REGISTER size_t     stExcess  = (size_t)(stWriteLen & ~LW_CFG_VMM_PAGE_MASK);
                 addr_t     ulAddr    = (addr_t)pvAddr;
#if LW_CFG_VMM_EN > 0
                 ULONG      ulFlag;
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
                 
        for (i = 0; i < ulPageNum; i++) {
#if LW_CFG_VMM_EN > 0
            if ((API_VmmGetFlag((PVOID)ulAddr, &ulFlag) == ERROR_NONE) &&
                (ulFlag & LW_VMM_FLAG_WRITABLE)) {                      /*  �ڴ������Ч�ſ�д          */
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
                if (pwrite(pmapnode->PMAPN_iFd, (CPVOID)ulAddr, 
                           LW_CFG_VMM_PAGE_SIZE, off) != 
                           LW_CFG_VMM_PAGE_SIZE) {                      /*  д���ļ�                    */
                    pmapnode->PMAPN_bBusy = LW_FALSE;
                    return  (PX_ERROR);
                }
#if LW_CFG_VMM_EN > 0
            }
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
            ulAddr += LW_CFG_VMM_PAGE_SIZE;
            off    += LW_CFG_VMM_PAGE_SIZE;
        }
        
        if (stExcess) {
#if LW_CFG_VMM_EN > 0
            if ((API_VmmGetFlag((PVOID)ulAddr, &ulFlag) == ERROR_NONE) &&
                (ulFlag & LW_VMM_FLAG_WRITABLE)) {                      /*  �ڴ������Ч�ſ�д          */
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
                if (pwrite(pmapnode->PMAPN_iFd, (CPVOID)ulAddr, 
                           stExcess, off) != stExcess) {                /*  д���ļ�                    */
                    pmapnode->PMAPN_bBusy = LW_FALSE;
                    return  (PX_ERROR);
                }
#if LW_CFG_VMM_EN > 0
            }
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
        }
                 
        if (iFlag & MS_SYNC) {
            ioctl(pmapnode->PMAPN_iFd, FIOSYNC, 0);
        }
    } else {                                                            /*  ��Ҫ�����ļ�����            */
    
        if (pmapnode->PMAPN_bIsHeapMem) {                               /*  heap �����ڴ�               */
            caddr_t     pcAddr  = (caddr_t)pvAddr;
            size_t      stTotal = 0;
            ssize_t     sstReadNum;
            off_t       oftSeek = off;
            
            while (stTotal < stWriteLen) {
                sstReadNum = pread(pmapnode->PMAPN_iFd, &pcAddr[stTotal], (stWriteLen - stTotal), oftSeek);
                if (sstReadNum <= 0) {
                    break;
                }
                stTotal += (size_t)sstReadNum;
                oftSeek += (off_t)sstReadNum;
            }
            
            if (stTotal < stWriteLen) {
                pmapnode->PMAPN_bBusy = LW_FALSE;
                errno = EIO;
                return  (PX_ERROR);
            }
        } else {                                                        /*  ȱҳ�ж�ʽ�����ڴ�          */
#if LW_CFG_VMM_EN > 0
            API_VmmInvalidateArea(pmapnode->PMAPN_pvAddr, 
                                  pvAddr, 
                                  stLen);                               /*  �ͷŶ�Ӧ�������ڴ�          */
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
        }
    }

    pmapnode->PMAPN_bBusy = LW_FALSE;
    
    MONITOR_EVT_LONG3(MONITOR_EVENT_ID_VMM, MONITOR_EVENT_VMM_MSYNC,
                      pvAddr, stLen, iFlag, LW_NULL);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: mmapShow
** ��������: ��ʾ��ǰϵͳӳ��������ļ��ڴ�.
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
VOID  mmapShow (VOID)
{
    const CHAR          cMmapInfoHdr[] = 
    "  ADDR     SIZE        OFFSET      WRITE SHARE  PID   FD\n"
    "-------- -------- ---------------- ----- ----- ----- ----\n";
    
    __PX_MAP_NODE      *pmapn;
    PLW_LIST_LINE       plineTemp;
    PCHAR               pcWrite;
    PCHAR               pcShare;
    
    printf(cMmapInfoHdr);
    
    __PX_LOCK();
    for (plineTemp  = _G_plineMMapHeader;
         plineTemp != LW_NULL;
         plineTemp  = _list_line_get_next(plineTemp)) {
        
        pmapn = (__PX_MAP_NODE *)plineTemp;
        
        if (pmapn->PMAPN_ulFlag & LW_VMM_FLAG_WRITABLE) {
            pcWrite = "true ";
        } else {
            pcWrite = "false";
        }
        
        if (pmapn->PMAPN_iFlag & MAP_SHARED) {
            pcShare = "true ";
        } else {
            pcShare = "false";
        }
        
        printf("%08lx %8lx %16llx %s %s %5d %4d\n", 
               (addr_t)pmapn->PMAPN_pvAddr,
               (ULONG)pmapn->PMAPN_stLen,
               pmapn->PMAPN_off,
               pcWrite,
               pcShare,
               pmapn->PMAPN_pid,
               pmapn->PMAPN_iFd);
    }
    __PX_UNLOCK();
    
    printf("\n");
}
/*********************************************************************************************************
** ��������: shm_open
** ��������: establishes a connection between a shared memory object and a file descriptor.
** �䡡��  : name      file name
**           oflag     create flag like open()
**           mode      create mode like open()
** �䡡��  : filedesc
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
int  shm_open (const char *name, int oflag, mode_t mode)
{
    CHAR    cFullName[MAX_FILENAME_LENGTH] = "/dev/shm/";
    
    if (!name || (*name == PX_EOS)) {
        errno = EINVAL;
        return  (PX_ERROR);
    }
    
    if (name[0] == PX_ROOT) {
        lib_strlcat(cFullName, &name[1], MAX_FILENAME_LENGTH);
    } else {
        lib_strlcat(cFullName, name, MAX_FILENAME_LENGTH);
    }
    
    return  (open(cFullName, oflag, mode));
}
/*********************************************************************************************************
** ��������: shm_unlink
** ��������: removes the name of the shared memory object named by the string pointed to by name.
** �䡡��  : name      file name
** �䡡��  : filedesc
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
int  shm_unlink (const char *name)
{
    CHAR    cFullName[MAX_FILENAME_LENGTH] = "/dev/shm/";
    
    if (!name || (*name == PX_EOS)) {
        errno = EINVAL;
        return  (PX_ERROR);
    }
    
    if (name[0] == PX_ROOT) {
        lib_strlcat(cFullName, &name[1], MAX_FILENAME_LENGTH);
    } else {
        lib_strlcat(cFullName, name, MAX_FILENAME_LENGTH);
    }
    
    return  (unlink(cFullName));
}
#endif                                                                  /*  LW_CFG_POSIX_EN > 0         */
                                                                        /*  LW_CFG_DEVICE_EN > 0        */
/*********************************************************************************************************
  END
*********************************************************************************************************/
