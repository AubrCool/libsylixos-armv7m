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
** ��   ��   ��: Lw_Api_Cache.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2006 �� 12 �� 14 ��
**
** ��        ��: ����ϵͳ�ṩ�� C / C++ �û����ں�Ӧ�ó���ӿڲ㡣
                 Ϊ����Ӧ��ͬ����ϰ�ߵ��ˣ�����ʹ���˺ܶ��ظ�����.
*********************************************************************************************************/

#ifndef __LW_API_CACHE_H
#define __LW_API_CACHE_H

/*********************************************************************************************************
    CACHE API
*********************************************************************************************************/

#define Lw_Cache_Location                   API_CacheLocation
#define Lw_Cache_Enable                     API_CacheEnable
#define Lw_Cache_Disable                    API_CacheDisable

#define Lw_Cache_Lock                       API_CacheLock
#define Lw_Cache_Unlock                     API_CacheUnlock

#define Lw_Cache_Flush                      API_CacheFlush
#define Lw_Cache_Invalidate                 API_CacheInvalidate
#define Lw_Cache_Clear                      API_CacheClear
#define Lw_Cache_CacheTextUpdate            API_CacheTextUpdate

#define Lw_Cache_DmaMalloc                  API_CacheDmaMalloc
#define Lw_Cache_DmaMallocAlgin             API_CacheDmaMallocAlign
#define Lw_Cache_DmaFree                    API_CacheDmaFree

#define Lw_Cache_DrvFlush                   API_CacheDrvFlush
#define Lw_Cache_DrvInvalidate              API_CacheDrvInvalidate

#define Lw_Cache_FuncsSet                   API_CacheFuncsSet

#endif                                                                  /*  __LW_API_CACHE_H            */
/*********************************************************************************************************
    OBJECT
*********************************************************************************************************/
