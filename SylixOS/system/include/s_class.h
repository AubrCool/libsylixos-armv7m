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
** ��   ��   ��: s_class.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2007 �� 02 �� 13 ��
**
** ��        ��: ����ϵͳϵͳ���ؼ����Ͷ����ļ���

** BUG
2007.04.08  ɾ���ü�����ƣ����͵Ķ��壬�Ͳü��޹�
2008.03.23  ������ 2008 ̨������쵼��ѡ�ٺ�ĵ�һ��, ��ϣ��������ƽͳһ.
            �Ľ� I/O ϵͳ, ���� BSD �� Linux ϵͳ����ع���.
2009.02.13  ���쿪ʼ�������Ľ� I/O ϵͳ, �޸�Ϊ���� UNIX/BSD I/O ����ģʽ, ����ʹ�������㷨, �ǳ��� API
            ����ʱ�临�Ӷ� O(1) ʱ�临�Ӷ�.
2009.08.31  �����Ӧ���豸�Ĺ��Ĺ���ģ��.
2009.10.02  �豸ͷ�򿪴���ʹ�� atomic_t ��.
2009.11.21  ���� io ��������.
2009.12.14  ����������ӿ��м��� symlink ����. ע��: ��������ֻ�в����ļ�ϵͳ֧��(���ֹ���)! 
2010.09.10  file_operations ���� fstat ��������.
2011.07.28  file_operations ���� mmap ��������.
2011.08.07  file_operations ���� lstat ��������.
2012.10.19  LW_FD_NODE �������ü��� FDNODE_ulRef, ���� mmap() ʱ��������. munmap() ʱ���ټ���, ������Ϊ 0
            ʱ�ر��ļ�.
2012.11.21  �ļ��ṹ�м����� unlink ��־, ��ʼ֧���ļ�û�йر�ʱ�� unlink ����.
2012.12.21  ����ȵĸ����ļ���������
            ����һ���µĻ���: FDENTRY_bRemoveReq ��ʾ�����ʱ�ļ��ṹ����æ, �������Ƴ�ɾ������һ���ڵ�,
            ����µĻ����±����ļ��⹹��ʱ, �������� IO ϵͳ.
2013.01.09  LW_IO_ENV �м��� umask ����.
2013.05.27  fd entry ����ȥ�����������������ʵ�ļ���.
*********************************************************************************************************/

#ifndef __S_CLASS_H
#define __S_CLASS_H

/*********************************************************************************************************
  IO ����
*********************************************************************************************************/

typedef struct {
    mode_t               IOE_modeUMask;                                 /*  umask                       */
    CHAR                 IOE_cDefPath[MAX_FILENAME_LENGTH];             /*  Ĭ�ϵ�ǰ·��                */
} LW_IO_ENV;
typedef LW_IO_ENV       *PLW_IO_ENV;

/*********************************************************************************************************
  IO �û���Ϣ
*********************************************************************************************************/

typedef struct {
    uid_t                IOU_uid;
    gid_t                IOU_gid;
} LW_IO_USR;
typedef LW_IO_USR       *PLW_IO_USR;

/*********************************************************************************************************
  ���Ĺ���ڵ� (ÿ���豸���Խ���������Ŀ��Ƶ�, ÿ�����Ŀ��Ƶ�����в�ͬ�Ĺ��Ŀ��Ƶȼ�, ������ʾ������
                �����ڸ��ݿ���ʱ��ѡ��ͬ������. ������������һ���豸��һ����Դ����ڵ㼴��, ������Ҫ
                ����˯���뻽�Ѽ���.)
  ע��         ÿ�β����豸ʱ(��, д, ���Ƶȵ�), ����Ҫ���� iosDevPowerMSignal(), �Ա�֪ͨ����ϵͳ����豸
               ���ڻ״̬, ����ϵͳ�Ḵλ��ع��Ķ�ʱ��.
               ����ʱ�䲻ʹ�ô��豸ʱ(���Ķ�ʱ�������), ϵͳ���������ػص���ʹ���豸����͹���״̬.
*********************************************************************************************************/

#if (LW_CFG_POWERM_EN > 0) && (LW_CFG_MAX_POWERM_NODES > 0)

typedef struct {
    LW_LIST_LINE              DEVPM_lineManage;                         /*  ��Դ����ڵ�����            */
    LW_OBJECT_HANDLE          DEVPM_ulPowerM;                           /*  ��Դ����ڵ����            */
    
    FUNCPTR                   DEVPM_pfuncPowerOff;                      /*  ���豸���ڵ͹���ģʽ        */
    PVOID                     DEVPM_pvArgPowerOff;                      /*  �͹���ģʽ����              */
    FUNCPTR                   DEVPM_pfuncPowerSignal;                   /*  �����豸                    */
                                                                        /*  �ᱻ iosDevPowerMSignal ����*/
    PVOID                     DEVPM_pvArgPowerSignal;                   /*  �����豸����                */
    FUNCPTR                   DEVPM_pfuncRemove;                        /*  �Ƴ��豸ʱϵͳ����ô˺���  */
                                                                        /*  ��������û���Ҫ����Ľṹ  */
    PVOID                     DEVPM_pvArgRemove;                        /*  �Ƴ��豸����                */
} LW_DEV_PM;
typedef LW_DEV_PM            *PLW_DEV_PM;
#endif                                                                  /*  LW_CFG_POWERM_EN            */
                                                                        /*  LW_CFG_MAX_POWERM_NODES     */
/*********************************************************************************************************
  �豸ͷ
  
  ����ʹ�õ�����ͨ��˫������, ���ǵ�ֻ�д��豸������Ҫ�����豸��, ��������������� O(1) ʱ�临�Ӷ�.
  �����������ʹ���� hash ��ϣɢ�б�������.
  ����豸�ܶ�, �ֶ�Ƕ��ʽϵͳ������������Ҫ��, ����ʹ���Ի���(horner)����ʽ��Ϊ��ϣ�����Ĺ�ϣ������.
*********************************************************************************************************/

typedef struct {
    LW_LIST_LINE               DEVHDR_lineManage;                       /*  �豸ͷ��������              */
    UINT16                     DEVHDR_usDrvNum;                         /*  �豸��������������          */
    PCHAR                      DEVHDR_pcName;                           /*  �豸����                    */
    UCHAR                      DEVHDR_ucType;                           /*  �豸 dirent d_type          */
    atomic_t                   DEVHDR_atomicOpenNum;                    /*  �򿪵Ĵ���                  */
    LW_LIST_LINE_HEADER        DEVHDR_plinePowerMHeader;                /*  �豸���Ĺ���ڵ���          */
} LW_DEV_HDR;
typedef LW_DEV_HDR            *PLW_DEV_HDR;

static LW_INLINE INT  LW_DEV_INC_USE_COUNT(PLW_DEV_HDR  pdevhdrHdr)
{
    return  ((pdevhdrHdr) ? (API_AtomicInc(&pdevhdrHdr->DEVHDR_atomicOpenNum)) : (PX_ERROR));
}

static LW_INLINE INT  LW_DEV_DEC_USE_COUNT(PLW_DEV_HDR  pdevhdrHdr)
{
    return  ((pdevhdrHdr) ? (API_AtomicDec(&pdevhdrHdr->DEVHDR_atomicOpenNum)) : (PX_ERROR));
}

static LW_INLINE INT  LW_DEV_GET_USE_COUNT(PLW_DEV_HDR  pdevhdrHdr)
{
    return  ((pdevhdrHdr) ? (API_AtomicGet(&pdevhdrHdr->DEVHDR_atomicOpenNum)) : (PX_ERROR));
}

/*********************************************************************************************************
  �����������֤
*********************************************************************************************************/

typedef struct {
    PCHAR                      DRVLIC_pcLicense;                        /*  ���֤                      */
    PCHAR                      DRVLIC_pcAuthor;                         /*  ����                        */
    PCHAR                      DRVLIC_pcDescription;                    /*  ����                        */
} LW_DRV_LICENSE;

/*********************************************************************************************************
  ������������
*********************************************************************************************************/

#define LW_DRV_TYPE_ORIG      0                                         /*  ԭʼ�豸����, ���� VxWorks  */
#define LW_DRV_TYPE_SOCKET    1                                         /*  SOCKET ���豸��������       */
#define LW_DRV_TYPE_NEW_1     2                                         /*  NEW_1 ���豸��������        */

/*********************************************************************************************************
  ����������ƿ� (���豸) (ֻ��ʹ�� SylixOS �µ�Ŀ¼�����ܲ���ʹ�÷�������)
*********************************************************************************************************/

typedef struct {
    LONGFUNCPTR                DEVENTRY_pfuncDevCreate;                 /*  ��������                    */
    FUNCPTR                    DEVENTRY_pfuncDevDelete;                 /*  ɾ������                    */
    
    LONGFUNCPTR                DEVENTRY_pfuncDevOpen;                   /*  �򿪺���                    */
    FUNCPTR                    DEVENTRY_pfuncDevClose;                  /*  �رպ���                    */
    
    SSIZETFUNCPTR              DEVENTRY_pfuncDevRead;                   /*  ���豸����                  */
    SSIZETFUNCPTR              DEVENTRY_pfuncDevWrite;                  /*  д�豸����                  */
    
    SSIZETFUNCPTR              DEVENTRY_pfuncDevReadEx;                 /*  ���豸������չ����          */
    SSIZETFUNCPTR              DEVENTRY_pfuncDevWriteEx;                /*  ���豸������չ����          */
    
    FUNCPTR                    DEVENTRY_pfuncDevIoctl;                  /*  �豸���ƺ���                */
    FUNCPTR                    DEVENTRY_pfuncDevSelect;                 /*  select ����                 */
    OFFTFUNCPTR                DEVENTRY_pfuncDevLseek;                  /*  lseek ����                  */
    
    FUNCPTR                    DEVENTRY_pfuncDevFstat;                  /*  fstat ����                  */
    FUNCPTR                    DEVENTRY_pfuncDevLstat;                  /*  lstat ����                  */
    
    FUNCPTR                    DEVENTRY_pfuncDevSymlink;                /*  ���������ļ�                */
    SSIZETFUNCPTR              DEVENTRY_pfuncDevReadlink;               /*  ��ȡ�����ļ�                */
    
    FUNCPTR                    DEVENTRY_pfuncDevMmap;                   /*  �ļ�ӳ��                    */
    FUNCPTR                    DEVENTRY_pfuncDevUnmap;                  /*  ӳ�����                    */
    
    BOOL                       DEVENTRY_bInUse;                         /*  �Ƿ�ʹ��                  */
    INT                        DEVENTRY_iType;                          /*  �豸��������                */
    
    LW_DRV_LICENSE             DEVENTRY_drvlicLicense;                  /*  �����������֤              */
} LW_DEV_ENTRY;
typedef LW_DEV_ENTRY          *PLW_DEV_ENTRY;

/*********************************************************************************************************
  �豸�ļ��������ƿ�
*********************************************************************************************************/

typedef struct file_operations {
    struct module              *owner;
    
    long                      (*fo_create)();                           /*  DEVENTRY_pfuncDevCreate     */
    int                       (*fo_release)();                          /*  DEVENTRY_pfuncDevDelete     */
    
    long                      (*fo_open)();                             /*  DEVENTRY_pfuncDevOpen       */
    int                       (*fo_close)();                            /*  DEVENTRY_pfuncDevClose      */
    
    ssize_t                   (*fo_read)();                             /*  DEVENTRY_pfuncDevRead       */
    ssize_t                   (*fo_read_ex)();                          /*  DEVENTRY_pfuncDevReadEx     */
    
    ssize_t                   (*fo_write)();                            /*  DEVENTRY_pfuncDevWrite      */
    ssize_t                   (*fo_write_ex)();                         /*  DEVENTRY_pfuncDevWriteEx    */
    
    int                       (*fo_ioctl)();                            /*  DEVENTRY_pfuncDevIoctl      */
    int                       (*fo_select)();                           /*  DEVENTRY_pfuncDevSelect     */
    
    int                       (*fo_lock)();                             /*  not support now             */
    off_t                     (*fo_lseek)();                            /*  DEVENTRY_pfuncDevLseek      */
    
    int                       (*fo_fstat)();                            /*  DEVENTRY_pfuncDevFstat      */
    int                       (*fo_lstat)();                            /*  DEVENTRY_pfuncDevLstat      */
    
    int                       (*fo_symlink)();                          /*  DEVENTRY_pfuncDevSymlink    */
    ssize_t                   (*fo_readlink)();                         /*  DEVENTRY_pfuncDevReadlink   */
    
    int                       (*fo_mmap)();                             /*  DEVENTRY_pfuncDevMmap       */
    int                       (*fo_unmap)();                            /*  DEVENTRY_pfuncDevUnmmap     */
    
    ULONG                       fo_pad[16];                             /*  reserve                     */
} FILE_OPERATIONS;
typedef FILE_OPERATIONS        *PFILE_OPERATIONS;

/*********************************************************************************************************
  fo_mmap �� fo_unmap ��ַ��Ϣ����
*********************************************************************************************************/

typedef struct {
    PVOID                       DMAP_pvAddr;                            /*  ��ʼ��ַ                    */
    size_t                      DMAP_stLen;                             /*  �ڴ泤�� (��λ:�ֽ�)        */
    off_t                       DMAP_offPages;                          /*  �ļ�ӳ��ƫ���� (��λ:ҳ��)  */
    ULONG                       DMAP_ulFlag;                            /*  ��ǰ�����ַ VMM ����       */
} LW_DEV_MMAP_AREA;
typedef LW_DEV_MMAP_AREA       *PLW_DEV_MMAP_AREA;

/*********************************************************************************************************
  SylixOS I/O ϵͳ�ṹ
  
  (ORIG ������)
  
       0                       1                       N
  +---------+             +---------+             +---------+
  | FD_DESC |             | FD_DESC |     ...     | FD_DESC |
  +---------+             +---------+             +---------+
       |                       |                       |
       |                       |                       |
       \-----------------------/                       |
                   |                                   |
                   |                                   |
             +------------+                      +------------+
  HEADER ->  |  FD_ENTERY |   ->    ...   ->     |  FD_ENTERY |  ->  NULL
             +------------+                      +------------+
                   |                                   |
                   |                                   |
                   |                                   |
             +------------+                      +------------+
             |  DEV_HDR   |                      |  DEV_HDR   |
             +------------+                      +------------+
                   |                                   |
                   |                                   |
                   \-----------------------------------/
                                    |
                                    |
                              +------------+
                              |   DRIVER   |
                              +------------+
  
  (NEW_1 ������)
  
       0                       1                       N
  +---------+             +---------+             +---------+
  | FD_DESC |             | FD_DESC |     ...     | FD_DESC |
  +---------+             +---------+             +---------+
       |                       |                       |
       |                       |                       |
       \-----------------------/                       |
                   |                                   |
                   |                                   |
             +------------+                      +------------+
  HEADER ->  |  FD_ENTERY |   ->    ...   ->     |  FD_ENTERY |  ->  NULL
             +------------+                      +------------+
                   |                 |                 |
                   |                 |                 |
                   \-----------------/                 |
                             |                         |
                             |                         |
                      +------------+             +------------+
                      |   FD_NODE  |    ...      |   FD_NODE  |
                      | lockf->... |             | lockf->... |
                      +------------+             +------------+
                             |           |             |
                             |           |             |
                             \-----------/             |
                                   |                   |
                                   |                   |
                            +------------+       +------------+
                            |  DEV_HDR   |       |  DEV_HDR   |
                            +------------+       +------------+
                                   |                   |
                                   |                   |
                                   \-------------------/
                                             |
                                             |
                                       +------------+
                                       |   DRIVER   |
                                       +------------+
*********************************************************************************************************/
/*********************************************************************************************************
  �ļ���¼��
  ֻ�� NEW_1 ����߼����豸�������Ϳ���֧���ļ���¼��.
*********************************************************************************************************/

typedef struct __fd_lockf {
    LW_OBJECT_HANDLE           FDLOCK_ulBlock;                          /*  ������                      */
    INT16                      FDLOCK_usFlags;                          /*  F_WAIT F_FLOCK F_POSIX      */
    INT16                      FDLOCK_usType;                           /*  F_RDLCK F_WRLCK             */
    off_t                      FDLOCK_oftStart;                         /*  start of the lock           */
    off_t                      FDLOCK_oftEnd;                           /*  end of the lock (-1=EOF)    */
    pid_t                      FDLOCK_pid;                              /*  resource holding process    */
    
    struct  __fd_lockf       **FDLOCK_ppfdlockHead;                     /*  back to the head of list    */
    struct  __fd_lockf        *FDLOCK_pfdlockNext;                      /*  Next lock on this fd_node,  */
                                                                        /*  or blocking lock            */
    
    LW_LIST_LINE_HEADER        FDLOCK_plineBlockHd;                     /*  �����ڵ�ǰ���ļ�¼������    */
    LW_LIST_LINE               FDLOCK_lineBlock;                        /*  ����ȴ����� header is lockf*/
    LW_LIST_LINE               FDLOCK_lineBlockQ;                       /*  ��������, header is fd_node */
} LW_FD_LOCKF;
typedef LW_FD_LOCKF           *PLW_FD_LOCKF;

#ifdef __SYLIXOS_KERNEL
#define F_WAIT                 0x10                                     /* wait until lock is granted   */
#define F_FLOCK                0x20                                     /* BSD semantics for lock       */
#define F_POSIX                0x40                                     /* POSIX semantics for lock     */
#define F_ABORT                0x80                                     /* lock wait abort!             */
#endif                                                                  /* __SYLIXOS_KERNEL             */

/*********************************************************************************************************
  �ļ��ڵ� 
  
  ֻ�� NEW_1 ����߼����豸�������ͻ��õ��˽ṹ
  һ�� dev_t �� һ�� ino_t ��ӦΨһһ��ʵ���ļ�, ����ϵͳͳ��δ�ͬһ��ʵ���ļ�ʱ, ֻ��һ���ļ��ڵ�
  ��� fd_entry ͬʱָ������ڵ�.
  ��Ҫ˵������ FDNODE_oftSize �ֶ���Ҫ NEW_1 ���������Լ���ά��.
  
  fd node ������ʱ, ��������д, Ҳ������ɾ��. ���ر��ļ�����ļ��� lock ���Զ����ͷ�.
*********************************************************************************************************/

typedef struct {
    LW_LIST_LINE               FDNODE_lineManage;                       /*  ͬһ�豸 fd_node ����       */
    
    LW_OBJECT_HANDLE           FDNODE_ulSem;                            /*  �ڲ�������                  */
    dev_t                      FDNODE_dev;                              /*  �豸                        */
    ino64_t                    FDNODE_inode64;                          /*  inode (64bit Ϊ�˼�����)    */
    mode_t                     FDNODE_mode;                             /*  �ļ� mode                   */
    uid_t                      FDNODE_uid;                              /*  �ļ������û���Ϣ            */
    gid_t                      FDNODE_gid;
    
    off_t                      FDNODE_oftSize;                          /*  ��ǰ�ļ���С                */
    
    struct  __fd_lockf        *FDNODE_pfdlockHead;                      /*  ��һ����                    */
    LW_LIST_LINE_HEADER        FDNODE_plineBlockQ;                      /*  ��ǰ�������ļ�¼������      */
    
    ULONG                      FDNODE_ulLock;                           /*  ����, ������д, ������ɾ��  */
    ULONG                      FDNODE_ulRef;                            /*  fd_entry ���ô� fd_node ����*/
    PVOID                      FDNODE_pvFile;                           /*  ������ʹ��, ��ǰδʹ��      */
} LW_FD_NODE;
typedef LW_FD_NODE            *PLW_FD_NODE;

/*********************************************************************************************************
  �ļ��ṹ (�ļ���)
  FDENTRY_ulCounter �����������ж�Ӧ�� LW_FD_DESC �������ܺ�. 
  ���û�� dup ��, �� FDENTRY_ulCounter Ӧ���� FDDESC_ulRef ��ͬ.
  
  ����� ORIG ��������:
  �� open �����������ײ���Ϊ FDENTRY_lValue.
  
  ����� NEW_1 ��������:
  �� open �����������ײ���Ϊ fd_entry ����, FDENTRY_lValue Ϊ open ����ֵ���ײ�������Ϊ LW_FD_NODE.
  ��Ҫ˵������ FDENTRY_oftPtr �ֶ���Ҫ���������Լ���ά��.
*********************************************************************************************************/

typedef struct {
    PLW_DEV_HDR                FDENTRY_pdevhdrHdr;                      /*  �豸ͷ                      */
    PCHAR                      FDENTRY_pcName;                          /*  �ļ���                      */
    PCHAR                      FDENTRY_pcRealName;                      /*  ȥ���������ӵ���ʵ�ļ���    */
    LW_LIST_LINE               FDENTRY_lineManage;                      /*  �ļ�������Ϣ������          */
    
#define FDENTRY_pfdnode        FDENTRY_lValue
    LONG                       FDENTRY_lValue;                          /*  ���������ڲ�����            */
                                                                        /*  ���Ϊ NEW_1 ������Ϊfd_node*/
    INT                        FDENTRY_iType;                           /*  �ļ����� (���������ж�)     */
    INT                        FDENTRY_iFlag;                           /*  �ļ�����                    */
    INT                        FDENTRY_iAbnormity;                      /*  �ļ��쳣                    */
    
    BOOL                       FDENTRY_bNeedUnlink;                     /*  �Ƿ����ļ�δ�ر�ʱ�� unlink */
    ULONG                      FDENTRY_ulCounter;                       /*  �����ü�����                */
    off_t                      FDENTRY_oftPtr;                          /*  �ļ���ǰָ��                */
                                                                        /*  ֻ�� NEW_1 ����߼�����ʹ�� */
    BOOL                       FDENTRY_bRemoveReq;                      /*  ɾ������                    */
} LW_FD_ENTRY;
typedef LW_FD_ENTRY           *PLW_FD_ENTRY;

/*********************************************************************************************************
  �ļ���������
*********************************************************************************************************/

typedef struct {
    PLW_FD_ENTRY               FDDESC_pfdentry;                         /*  �ļ��ṹ                    */
    BOOL                       FDDESC_bCloExec;                         /*  FD_CLOEXEC                  */
    ULONG                      FDDESC_ulRef;                            /*  ��Ӧ�ļ������������ü���    */
} LW_FD_DESC;
typedef LW_FD_DESC            *PLW_FD_DESC;

/*********************************************************************************************************
    �̳߳�
*********************************************************************************************************/

typedef struct {
    LW_LIST_MONO               TPCB_monoResrcList;                       /*  ��Դ����                   */
    PTHREAD_START_ROUTINE      TPCB_pthreadStartAddress;                 /*  �̴߳�����ڵ�             */
    PLW_LIST_RING              TPCB_pringFirstThread;                    /*  ��һ���߳�                 */
    LW_CLASS_THREADATTR        TPCB_threakattrAttr;                      /*  �߳̽������Կ�             */
    
    UINT16                     TPCB_usMaxThreadCounter;                  /*  ����߳�����               */
    UINT16                     TPCB_usThreadCounter;                     /*  ��ǰ�߳�����               */
    
    LW_OBJECT_HANDLE           TPCB_hMutexLock;                          /*  ������                     */
    
    UINT16                     TPCB_usIndex;                             /*  �����±�                   */
                                                                         /*  ����                       */
    CHAR                       TPCB_cThreadPoolName[LW_CFG_OBJECT_NAME_SIZE];
} LW_CLASS_THREADPOOL;
typedef LW_CLASS_THREADPOOL   *PLW_CLASS_THREADPOOL;

#endif                                                                   /*  __S_CLASS_H                */
/*********************************************************************************************************
  END
*********************************************************************************************************/
