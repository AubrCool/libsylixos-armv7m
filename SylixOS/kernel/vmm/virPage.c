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
** ��   ��   ��: virPage.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2008 �� 12 �� 09 ��
**
** ��        ��: �����ڴ����.

** BUG:
2013.05.24  ��������ռ���뿪��.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
  ����ü�֧��
*********************************************************************************************************/
#if LW_CFG_VMM_EN > 0
#include "virPage.h"
/*********************************************************************************************************
** ��������: __vmmVirtualCreate
** ��������: ��������ռ�����.
** �䡡��  : ulAddr            ��ʼ��ַ
**           stSize            ����ռ��С
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
ULONG  __vmmVirtualCreate (addr_t  ulAddr, size_t  stSize)
{
    REGISTER PLW_MMU_CONTEXT  pmmuctx = __vmmGetCurCtx();
    REGISTER ULONG            ulError;

    ulError = __pageZoneCreate(&pmmuctx->MMUCTX_vmzoneVirtual, ulAddr, stSize, LW_ZONE_ATTR_NONE,
                               __VMM_PAGE_TYPE_VIRTUAL);
    if (ulError) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "kernel low memory.\r\n");
        return  (ulError);
    
    } else {
        _ErrorHandle(ERROR_NONE);
        return  (ERROR_NONE);
    }
}
/*********************************************************************************************************
** ��������: __vmmVirtualPageAlloc
** ��������: ����ָ������������ҳ��
** �䡡��  : ulPageNum     ��Ҫ���������ҳ�����
** �䡡��  : ҳ����ƿ�
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
PLW_VMM_PAGE  __vmmVirtualPageAlloc (ULONG  ulPageNum)
{
    REGISTER PLW_MMU_CONTEXT  pmmuctx = __vmmGetCurCtx();

    return  (__pageAllocate(&pmmuctx->MMUCTX_vmzoneVirtual, ulPageNum, 
                            __VMM_PAGE_TYPE_VIRTUAL));
}
/*********************************************************************************************************
** ��������: __vmmVirtualPageAllocAlign
** ��������: ����ָ������������ҳ�� (ָ�������ϵ)
** �䡡��  : ulPageNum     ��Ҫ���������ҳ�����
**           stAlign       �ڴ�����ϵ
** �䡡��  : ҳ����ƿ�
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
PLW_VMM_PAGE  __vmmVirtualPageAllocAlign (ULONG  ulPageNum, size_t  stAlign)
{
    REGISTER PLW_MMU_CONTEXT  pmmuctx = __vmmGetCurCtx();
    
    return  (__pageAllocateAlign(&pmmuctx->MMUCTX_vmzoneVirtual, ulPageNum, 
                                 stAlign, __VMM_PAGE_TYPE_VIRTUAL));
}
/*********************************************************************************************************
** ��������: __vmmVirtualPageFree
** ��������: ����ָ����������ҳ��
** �䡡��  : pvmpage       ҳ����ƿ�
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  __vmmVirtualPageFree (PLW_VMM_PAGE  pvmpage)
{
    REGISTER PLW_MMU_CONTEXT  pmmuctx = __vmmGetCurCtx();

    __pageFree(&pmmuctx->MMUCTX_vmzoneVirtual, pvmpage);
}
/*********************************************************************************************************
** ��������: __vmmVirtualPageGetMinContinue
** ��������: �������ҳ����, ��С������ҳ�ĸ���
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
ULONG  __vmmVirtualPageGetMinContinue (VOID)
{
    REGISTER PLW_MMU_CONTEXT  pmmuctx = __vmmGetCurCtx();

    return  (__pageGetMinContinue(&pmmuctx->MMUCTX_vmzoneVirtual));
}
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
