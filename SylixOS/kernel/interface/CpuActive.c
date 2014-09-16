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
** ��   ��   ��: CpuActive.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2014 �� 07 �� 21 ��
**
** ��        ��: SMP ϵͳ����/�ر�һ�� CPU.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
  �ü�����
*********************************************************************************************************/
#if LW_CFG_SMP_EN > 0
/*********************************************************************************************************
** ��������: API_CpuUp
** ��������: ����һ�� CPU. (�� 0 �� CPU)
** �䡡��  : ulCPUId       CPU ID
** �䡡��  : ERROR
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
ULONG  API_CpuUp (ULONG  ulCPUId)
{
    INTREG          iregInterLevel;
    PLW_CLASS_CPU   pcpu;

    if ((ulCPUId == 0) || (ulCPUId >= LW_NCPUS)) {
        _ErrorHandle(EINVAL);
        return  (EINVAL);
    }
    
    pcpu = LW_CPU_GET(ulCPUId);
    
    KN_SMP_MB();
    if (LW_CPU_IS_ACTIVE(pcpu)) {
        return  (ERROR_NONE);
    }
    
    iregInterLevel = KN_INT_DISABLE();
    bspCpuUp(ulCPUId);
    KN_INT_ENABLE(iregInterLevel);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_CpuDown
** ��������: �ر�һ�� CPU. (�� 0 �� CPU)
** �䡡��  : ulCPUId       CPU ID
** �䡡��  : ERROR
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
ULONG  API_CpuDown (ULONG  ulCPUId)
{
    PLW_CLASS_CPU   pcpu;

    if ((ulCPUId == 0) || (ulCPUId >= LW_NCPUS)) {
        _ErrorHandle(EINVAL);
        return  (EINVAL);
    }
    
    pcpu = LW_CPU_GET(ulCPUId);
    
    KN_SMP_MB();
    if (!LW_CPU_IS_ACTIVE(pcpu)) {
        return  (ERROR_NONE);
    }
    
    _SmpSendIpi(ulCPUId, LW_IPI_DOWN, 1);                               /*  ʹ�ú˼��ж�֪ͨ CPU ֹͣ   */
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_CpuIsUp
** ��������: ָ�� CPU �Ƿ�����.
** �䡡��  : ulCPUId       CPU ID
** �䡡��  : ERROR
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
BOOL  API_CpuIsUp (ULONG  ulCPUId)
{
    PLW_CLASS_CPU   pcpu;
    
    if (ulCPUId >= LW_NCPUS) {
        _ErrorHandle(EINVAL);
        return  (LW_FALSE);
    }
    
    pcpu = LW_CPU_GET(ulCPUId);
    
    KN_SMP_MB();
    if (LW_CPU_IS_ACTIVE(pcpu)) {
        return  (LW_TRUE);
    
    } else {
        return  (LW_FALSE);
    }
}

#endif                                                                  /*  LW_CFG_SMP_EN > 0           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
