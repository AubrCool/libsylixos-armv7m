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
** ��   ��   ��: GetLastError.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2006 �� 12 �� 14 ��
**
** ��        ��: �û����Ե������ API ���ϵͳ���һ������

** BUG
2007.03.22  ����ϵͳû������ʱ�Ĵ��������
2012.03.20  ���ٶ� _K_ptcbTCBCur ������, �������þֲ�����, ���ٶԵ�ǰ CPU ID ��ȡ�Ĵ���.
2013.07.18  ʹ���µĻ�ȡ TCB �ķ���, ȷ�� SMP ϵͳ��ȫ.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** ��������: API_GetLastError
** ��������: ���ϵͳ���һ������
** �䡡��  : 
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
ULONG  API_GetLastError (VOID)
{
    INTREG          iregInterLevel;
    PLW_CLASS_CPU   pcpuCur;
    PLW_CLASS_TCB   ptcbCur;
    ULONG           ulLastError;
    
    iregInterLevel = KN_INT_DISABLE();                                  /*  �ر��ж�, ��ֹ���ȵ����� CPU*/
    
    pcpuCur = LW_CPU_GET_CUR();
    
    if (pcpuCur->CPU_ulInterNesting) {
        ulLastError = pcpuCur->CPU_ulInterError[pcpuCur->CPU_ulInterNesting];
    } else {
        ptcbCur = pcpuCur->CPU_ptcbTCBCur;
        if (ptcbCur) {
            ulLastError = ptcbCur->TCB_ulLastError;
        } else {
            ulLastError = _K_ulNotRunError;
        }
    }
    
    KN_INT_ENABLE(iregInterLevel);

    return  (ulLastError);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
