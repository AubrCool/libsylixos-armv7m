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
** ��   ��   ��: InterVectorEnable.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2007 �� 02 �� 02 ��
**
** ��        ��: ʹ��ָ���������ж�, ϵͳ����Ӧ�������ж�.

** BUG:
2013.08.28  �����ں��¼������.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** ��������: API_InterVectorEnable
** ��������: ʹ��ָ���������ж�
** �䡡��  : ulVector                      �ж�������
** �䡡��  : ERROR
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
ULONG  API_InterVectorEnable (ULONG  ulVector)
{
    if (_Inter_Vector_Invalid(ulVector)) {
        _ErrorHandle(ERROR_KERNEL_VECTOR_NULL);
        return  (ERROR_KERNEL_VECTOR_NULL);
    }

    __ARCH_INT_VECTOR_ENABLE(ulVector);
    
    MONITOR_EVT_LONG1(MONITOR_EVENT_ID_INT, MONITOR_EVENT_INT_VECT_EN, ulVector, LW_NULL);
    
    _ErrorHandle(ERROR_NONE);
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_InterVectorDisable
** ��������: ����ָ���������ж�
** �䡡��  : ulVector                      �ж�������
** �䡡��  : ERROR
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
ULONG  API_InterVectorDisable (ULONG  ulVector)
{
    if (_Inter_Vector_Invalid(ulVector)) {
        _ErrorHandle(ERROR_KERNEL_VECTOR_NULL);
        return  (ERROR_KERNEL_VECTOR_NULL);
    }

    __ARCH_INT_VECTOR_DISABLE(ulVector);
    
    MONITOR_EVT_LONG1(MONITOR_EVENT_ID_INT, MONITOR_EVENT_INT_VECT_DIS, ulVector, LW_NULL);
    
    _ErrorHandle(ERROR_NONE);
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: API_InterVectorIsEnable
** ��������: ���ϵͳ��ָ�������ж���Ӧ״̬
** �䡡��  : ulVector                      �ж�������
**           bIsEnable                     �Ƿ�ʹ��������ж�
** �䡡��  : ERROR
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
ULONG  API_InterVectorIsEnable (ULONG  ulVector, BOOL  *pbIsEnable)
{
    if (_Inter_Vector_Invalid(ulVector)) {
        _ErrorHandle(ERROR_KERNEL_VECTOR_NULL);
        return  (ERROR_KERNEL_VECTOR_NULL);
    }
    
    if (!pbIsEnable) {
        _ErrorHandle(ERROR_KERNEL_MEMORY);
        return  (ERROR_KERNEL_MEMORY);
    }

    *pbIsEnable = __ARCH_INT_VECTOR_ISENABLE(ulVector);
    
    _ErrorHandle(ERROR_NONE);
    return  (ERROR_NONE);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
