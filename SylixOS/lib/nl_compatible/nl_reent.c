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
** ��   ��   ��: nl_reent.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2011 �� 08 �� 15 ��
**
** ��        ��: newlib fio ���ݲ�. (SylixOS �� VxWorks ����, ����ʹ���Լ��� libc ��)
                 �ܶ� gcc ʹ�� newlib ��Ϊ libc, �����Ŀ�Ҳ�������� newlib, ���� libstdc++ ��, 
                 SylixOS Ҫ��ʹ����Щ��, ������ṩһ�� newlib reent ���ݵĽӿ�.
                 
2012.11.09  lib_nlreent_init() ��������Ļ�������.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  �ü���
*********************************************************************************************************/
#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_FIO_LIB_EN > 0)
#include "../libc/stdio/lib_file.h"
/*********************************************************************************************************
  newlib compatible reent
*********************************************************************************************************/
typedef struct __nl_reent {
    /*
     *  ._errno ��ʹ��, Ϊ�˶����Ƽ���, �������ռλ������.
     *  ��Ϊ newlib errno �Ķ���Ϊ *__errno() �� SylixOS ��ͬ, �������ʹ���� errno ��һ���ᶨλ�� SylixOS
     *  �е� __errno() ����.
     */
    int     _errno_notuse;                                              /*  not use!                    */
    
    FILE    *_stdin, *_stdout, *_stderr;                                /*  ������׼�ļ�                */

    void    *_pad_notuse[48];                                           /*  not use!                    */
    
    FILE     _file[3];                                                  /*  ������׼�ļ�                */
} __NL_REENT;
/*********************************************************************************************************
  newlib compatible reent for all thread
*********************************************************************************************************/
static __NL_REENT _G_nlreentTbl[LW_CFG_MAX_THREADS];                    /*  ÿ������ӵ��һ�� reent    */
/*********************************************************************************************************
  newlib compatible reent
*********************************************************************************************************/
struct __nl_reent *__attribute__((weak)) _impure_ptr;                   /*  ��ǰ newlib reent ������    */
/*********************************************************************************************************
  stdio particular function
*********************************************************************************************************/
extern int fclose_nfree_fp(FILE *fp);
/*********************************************************************************************************
** ��������: __nlreent_swtich_hook
** ��������: nl reent ������� hook
** �䡡��  : ulOldThread   �����������������߳�
**           ulNewThread   ��Ҫ���е��߳�.
** �䡡��  : 0, 1, 2 ��ʾ��Ӧ���ļ����, -1 ��ʾû���� pnlreent->_file[] ��
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __nlreent_swtich_hook (LW_OBJECT_HANDLE  ulOldThread, LW_OBJECT_HANDLE  ulNewThread)
{
    __NL_REENT *pnlreent = &_G_nlreentTbl[_ObjectGetIndex(ulNewThread)];
    
    _impure_ptr = pnlreent;
}
/*********************************************************************************************************
** ��������: __nlreent_delete_hook
** ��������: nl reent ����ɾ�� hook
** �䡡��  : ulThread      �߳�
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  __nlreent_delete_hook (LW_OBJECT_HANDLE  ulThread)
{
    INT          i;
    __NL_REENT  *pnlreent = &_G_nlreentTbl[_ObjectGetIndex(ulThread)];
    
    /*
     *  ע��, �� stdin stdout �� stderr ���ض�����, �Ͳ��ùر���, ����ֻ�ر���ԭʼ������ std �ļ�.
     */
    for (i = 0; i < 3; i++) {
        if (pnlreent->_file[i]._flags) {
            fclose_nfree_fp(&pnlreent->_file[i]);
        }
    }
    
    pnlreent->_stdin  = LW_NULL;
    pnlreent->_stdout = LW_NULL;
    pnlreent->_stderr = LW_NULL;
}
/*********************************************************************************************************
** ��������: lib_nlreent_init
** ��������: ��ʼ��ָ���̵߳� nl reent �ṹ
** �䡡��  : ulThread      �߳� ID
** �䡡��  : newlib ���� reent �ṹ
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  lib_nlreent_init (LW_OBJECT_HANDLE  ulThread)
{
    static BOOL  bSwpHook = LW_FALSE;
    static BOOL  bDelHook = LW_FALSE;
    
    __NL_REENT  *pnlreent = &_G_nlreentTbl[_ObjectGetIndex(ulThread)];
    
    if (bSwpHook == LW_FALSE) {
        if (API_SystemHookAdd(__nlreent_swtich_hook, LW_OPTION_THREAD_SWAP_HOOK) == ERROR_NONE) {
            bSwpHook = LW_TRUE;
        }
    }
    
    if (bDelHook == LW_FALSE) {
        if (API_SystemHookAdd(__nlreent_delete_hook, LW_OPTION_THREAD_DELETE_HOOK) == ERROR_NONE) {
            bDelHook = LW_TRUE;
        }
    }
    
    pnlreent->_stdin  = &pnlreent->_file[STDIN_FILENO];
    pnlreent->_stdout = &pnlreent->_file[STDOUT_FILENO];
    pnlreent->_stderr = &pnlreent->_file[STDERR_FILENO];
    
    __stdioFileCreate(pnlreent->_stdin);
    __stdioFileCreate(pnlreent->_stdout);
    __stdioFileCreate(pnlreent->_stderr);
    
    /*
     *  stdin init flags
     */
    pnlreent->_stdin->_flags = __SRD;
#if LW_CFG_FIO_STDIN_LINE_EN > 0
    pnlreent->_stdin->_flags |= __SLBF;
#endif                                                                  /* LW_CFG_FIO_STDIN_LINE_EN     */

    /*
     *  stdout init flags
     */
    pnlreent->_stdout->_flags = __SWR;
#if LW_CFG_FIO_STDIN_LINE_EN > 0
    pnlreent->_stdout->_flags |= __SLBF;
#endif                                                                  /* LW_CFG_FIO_STDIN_LINE_EN     */

    /*
     *  stderr init flags
     */
    pnlreent->_stderr->_flags = __SWR | __SNBF;
    
    pnlreent->_stdin->_file  = STDIN_FILENO;
    pnlreent->_stdout->_file = STDOUT_FILENO;
    pnlreent->_stderr->_file = STDERR_FILENO;
}
/*********************************************************************************************************
** ��������: lib_nlreent_stdfile
** ��������: ��ȡָ���̵߳� stdfile �ṹ 
** �䡡��  : ulThread      �߳� ID
**           FileNo        �ļ���, 0, 1, 2
** �䡡��  : stdfile ָ���ַ
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
FILE **lib_nlreent_stdfile (LW_OBJECT_HANDLE  ulThread, INT  FileNo)
{
    __NL_REENT *pnlreent;

    if (!ulThread) {
        return  (LW_NULL);
    }
    
    pnlreent = &_G_nlreentTbl[_ObjectGetIndex(ulThread)];
    
    switch (FileNo) {
    
    case STDIN_FILENO:
        return  (&pnlreent->_stdin);
        
    case STDOUT_FILENO:
        return  (&pnlreent->_stdout);
        
    case STDERR_FILENO:
        return  (&pnlreent->_stderr);
        
    default:
        return  (LW_NULL);
    }
}
#endif                                                                  /*  LW_CFG_DEVICE_EN > 0        */
                                                                        /*  LW_CFG_FIO_LIB_EN > 0       */
/*********************************************************************************************************
  END
*********************************************************************************************************/
