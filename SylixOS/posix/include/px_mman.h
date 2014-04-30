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
** ��   ��   ��: px_mman.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2010 �� 08 �� 12 ��
**
** ��        ��: POSIX IEEE Std 1003.1, 2004 Edition sys/mman.h
*********************************************************************************************************/

#ifndef __PX_MMAN_H
#define __PX_MMAN_H

#include "SylixOS.h"                                                    /*  ����ϵͳͷ�ļ�              */

/*********************************************************************************************************
  �ü�֧��
*********************************************************************************************************/
#if LW_CFG_POSIX_EN > 0

#if LW_CFG_DEVICE_EN > 0

#ifdef __cplusplus
extern "C" {
#endif                                                                  /*  __cplusplus                 */

/*********************************************************************************************************
  page flag
*********************************************************************************************************/

#define PROT_READ                       1                               /*  Page can be read.           */
#define PROT_WRITE                      2                               /*  Page can be written.        */
#define PROT_EXEC                       4                               /*  Page can be executed.       */
#define PROT_NONE                       0                               /*  Page cannot be accessed.    */

/*********************************************************************************************************
  map flag
*********************************************************************************************************/

#define MAP_FILE                        0                               /*  File                        */
#define MAP_SHARED                      1                               /*  Share changes.              */
#define MAP_PRIVATE                     2                               /*  Changes are private.        */
#define MAP_FIXED                       4                               /*  Interpret addr exactly.     */
#define MAP_ANONYMOUS                   8                               /*  no fd rela this memory      */

#ifndef MAP_ANON
#define MAP_ANON                        MAP_ANONYMOUS
#endif

/*********************************************************************************************************
  msync() flag
*********************************************************************************************************/

#define MS_ASYNC                        1                               /*  Perform asynchronous writes.*/
#define MS_SYNC                         2                               /*  Perform synchronous writes. */
#define MS_INVALIDATE                   4                               /*  Invalidate mappings.        */

/*********************************************************************************************************
  memory lock flag
*********************************************************************************************************/

#define MCL_CURRENT                     1                               /*  Lock currently mapped pages.*/
#define MCL_FUTURE                      2                               /*  Lock become mapped pages.   */

/*********************************************************************************************************
  map fail
*********************************************************************************************************/

#define MAP_FAILED                      ((void *)-1)

/*********************************************************************************************************
  anonymous fd
*********************************************************************************************************/

#define MAP_ANON_FD                     (-1)

/*********************************************************************************************************
  mremap() flag
*********************************************************************************************************/

#define MREMAP_MAYMOVE                  1
#define MREMAP_FIXED                    2

/*********************************************************************************************************
  API
*********************************************************************************************************/

LW_API int      mlock(const void  *pvAddr, size_t  stLen);
LW_API int      munlock(const void  *pvAddr, size_t  stLen);
LW_API int      mlockall(int  iFlag);
LW_API int      munlockall(void);
LW_API int      mprotect(void  *pvAddr, size_t  stLen, int  iProt);
LW_API int      mmapfd(void  *pvAddr);
LW_API void    *mmap(void  *pvAddr, size_t  stLen, int  iProt, int  iFlag, int  iFd, off_t  off);
LW_API void    *mmap64(void  *pvAddr, size_t  stLen, int  iProt, int  iFlag, int  iFd, off64_t  off);
LW_API void    *mremap(void *pvAddr, size_t stOldSize, size_t stNewSize, int iFlag, ...);
LW_API int      munmap(void  *pvAddr, size_t  stLen);
LW_API int      msync(void  *pvAddr, size_t  stLen, int  iFlag);

/*********************************************************************************************************
  share memory
*********************************************************************************************************/

LW_API int      shm_open(const char *name, int oflag, mode_t mode);
LW_API int      shm_unlink(const char * name);

/*********************************************************************************************************
  EXT API
*********************************************************************************************************/

LW_API VOID     mmapShow(VOID);

#ifdef __cplusplus
}
#endif                                                                  /*  __cplusplus                 */

#endif                                                                  /*  LW_CFG_POSIX_EN > 0         */
#endif                                                                  /*  LW_CFG_DEVICE_EN > 0        */
#endif                                                                  /*  __PX_MMAN_H                 */
/*********************************************************************************************************
  END
*********************************************************************************************************/
