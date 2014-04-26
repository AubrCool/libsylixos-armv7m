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
** ��   ��   ��: Lw_Api_Vmm.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2006 �� 12 �� 14 ��
**
** ��        ��: ����ϵͳ�ṩ�� C / C++ �û����ں�Ӧ�ó���ӿڲ㡣
                 Ϊ����Ӧ��ͬ����ϰ�ߵ��ˣ�����ʹ���˺ܶ��ظ�����.
*********************************************************************************************************/

#ifndef __LW_API_VMM_H
#define __LW_API_VMM_H

/*********************************************************************************************************
    VMM API (Ӧ�ó�������Ҫʹ������ API)
*********************************************************************************************************/

#define Lw_Vmm_Malloc               API_VmmMalloc
#define Lw_Vmm_MallocEx             API_VmmMallocEx
#define Lw_Vmm_MallocAlign          API_VmmMallocAlign
#define Lw_Vmm_Free                 API_VmmFree

#define Lw_Vmm_MallocArea           API_VmmMallocArea
#define Lw_Vmm_MallocAreaEx         API_VmmMallocAreaEx
#define Lw_Vmm_MallocAreaAlign      API_VmmMallocAreaAlign
#define Lw_Vmm_FreeArea             API_VmmFreeArea
#define Lw_Vmm_InvalidateArea       API_VmmInvalidateArea
#define Lw_Vmm_AbortStatus          API_VmmAbortStatus

#define Lw_Vmm_DmaAlloc             API_VmmDmaAlloc
#define Lw_Vmm_DmaAllocAlign        API_VmmDmaAllocAlign
#define Lw_Vmm_DmaFree              API_VmmDmaFree

#define Lw_Vmm_IoRemap              API_VmmIoRemap
#define Lw_Vmm_IoUnmap              API_VmmIoUnmap
#define Lw_Vmm_IoRemapNocache       API_VmmIoRemapNocache

#define Lw_Vmm_Map                  API_VmmMap
#define Lw_Vmm_VirtualToPhysical    API_VmmVirtualToPhysical
#define Lw_Vmm_PhysicalToVirtual    API_VmmPhysicalToVirtual

#define Lw_Vmm_ZoneStatus           API_VmmZoneStatus
#define Lw_Vmm_VirtualStatus        API_VmmVirtualStatus

#endif                                                                  /*  __LW_API_VMM_H              */
/*********************************************************************************************************
    OBJECT
*********************************************************************************************************/

