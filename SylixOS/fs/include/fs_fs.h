/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                       SylixOS(TM)
**
**                               Copyright  All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: fs_fs.h
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2008 年 09 月 28 日
**
** 描        述: 这是文件系统综合头文件库。

** BUG:
2013.01.22  只有内核可以使用各文件系统 api.
*********************************************************************************************************/

#ifndef __FS_FS_H
#define __FS_FS_H

/*********************************************************************************************************
  FS type
*********************************************************************************************************/
#include "fs_type.h"                                                    /*  文件系统类型                */
#ifdef __SYLIXOS_KERNEL
/*********************************************************************************************************
  ROOT FS
*********************************************************************************************************/
#include "../SylixOS/fs/rootFs/rootFs.h"                                /*  根文件系统                  */
/*********************************************************************************************************
  PROC FS
*********************************************************************************************************/
#include "../SylixOS/fs/procFs/procFs.h"                                /*  PROC 文件系统               */
/*********************************************************************************************************
  RAM FS
*********************************************************************************************************/
#include "../SylixOS/fs/ramFs/ramFs.h"                                  /*  ram 文件系统                */
/*********************************************************************************************************
  ROM FS
*********************************************************************************************************/
#include "../SylixOS/fs/romFs/romFs.h"                                  /*  rom 文件系统                */
/*********************************************************************************************************
  FAT FS
*********************************************************************************************************/
#include "../SylixOS/fs/fatFs/fatFs.h"                                  /*  FAT 文件系统                */
/*********************************************************************************************************
  NFS
*********************************************************************************************************/
#include "../SylixOS/fs/nfs/nfs_sylixos.h"                              /*  NFS 文件系统                */
/*********************************************************************************************************
  DISK PARTITION
*********************************************************************************************************/
#include "../SylixOS/fs/diskPartition/diskPartition.h"                  /*  磁盘分区表                  */
/*********************************************************************************************************
  BLOCK DISK CACHE
*********************************************************************************************************/
#include "../SylixOS/fs/diskCache/diskCache.h"                          /*  磁盘高速缓冲                */
#include "../SylixOS/fs/nandRCache/nandRCache.h"                        /*  nand flash read cache       */
/*********************************************************************************************************
  OEM DISK 
*********************************************************************************************************/
#include "../SylixOS/fs/oemDisk/oemDisk.h"                              /*  OEM 磁盘操作                */
#endif                                                                  /*  __SYLIXOS_KERNEL            */
/*********************************************************************************************************
  MOUNT LIB
*********************************************************************************************************/
#include "../SylixOS/fs/mount/mount.h"                                  /*  mount 库                    */
/*********************************************************************************************************
  YAFFS2
*********************************************************************************************************/
#ifdef   __SYLIXOS_YAFFS_DRV
#include "../SylixOS/fs/yaffs2/direct/yaffscfg.h"                       /*  yaffs 驱动相关              */
#include "../SylixOS/fs/yaffs2/direct/yaffsfs.h"                        /*  yaffs 文件系统底层接口      */
#include "../SylixOS/fs/yaffs2/yaffs_trace.h"
#include "../SylixOS/fs/yaffs2/yaffs_guts.h"
#include "../SylixOS/fs/yaffs2/yaffs_nand.h"
#ifdef   __SYLIXOS_YAFFS_MTD
#include "../SylixOS/fs/yaffs2/yaffs_mtdif.h"
#endif                                                                  /*  __SYLIXOS_YAFFS_MTD         */
#endif                                                                  /*  __SYLIXOS_YAFFS_DRV         */

#include "../SylixOS/fs/yaffs2/yaffs_sylixosapi.h"
#endif                                                                  /*  __FS_FS_H                   */
/*********************************************************************************************************
  END
*********************************************************************************************************/
