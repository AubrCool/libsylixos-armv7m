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
** ��   ��   ��: KernelParam.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2014 �� 08 �� 08 ��
**
** ��        ��: ����ϵͳ�ں��������������ļ���
*********************************************************************************************************/
#define  __KERNEL_NCPUS_SET
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
  �ü�֧��
*********************************************************************************************************/
#if LW_CFG_VMM_EN > 0
#include "../SylixOS/kernel/vmm/virPage.h"
#endif
/*********************************************************************************************************
** ��������: API_KernelStartParam
** ��������: ϵͳ�ں���������
** �䡡��  : pcParam       ��������, ���Կո�ֿ���һ���ַ����б�ͨ������������ʽ:
                           ncpus=1 dlog=no derror=yes ... 
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                       API ����
*********************************************************************************************************/
LW_API
ULONG  API_KernelStartParam (CPCHAR  pcParam)
{
    CHAR        cParamBuffer[512];                                      /*  �������Ȳ��ó��� 512 �ֽ�   */
    PCHAR       pcDelim = " ";
    PCHAR       pcLast;
    PCHAR       pcTok;
    
#if LW_CFG_VMM_EN > 0
    PLW_MMU_VIRTUAL_DESC    pvirdesc = __vmmVirtualDesc();
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
    
    if (LW_SYS_STATUS_IS_RUNNING()) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "kernel is already start.\r\n");
        _ErrorHandle(ERROR_KERNEL_RUNNING);
        return  (ERROR_KERNEL_RUNNING);
    }
    
    lib_strlcpy(cParamBuffer, pcParam, 512);

    pcTok = lib_strtok_r(cParamBuffer, pcDelim, &pcLast);
    while (pcTok) {
        if (lib_strncmp(pcTok, "ncpus=", 6) == 0) {                     /*  CPU ����                    */
            INT     iCpus = lib_atoi(&pcTok[6]);
            if (iCpus > 0 && iCpus < LW_CFG_MAX_PROCESSORS) {
                _K_ulNCpus = (ULONG)iCpus;
            }
            
        } else if (lib_strncmp(pcTok, "kdlog=", 6) == 0) {              /*  �Ƿ�ʹ���ں� log ��ӡ       */
            if (pcTok[6] == 'n') {
                _K_pfuncKernelDebugLog = LW_NULL;
            } else {
                _K_pfuncKernelDebugLog = bspDebugMsg;
            }
            
        } else if (lib_strncmp(pcTok, "kderror=", 8) == 0) {            /*  �Ƿ�ʹ���ں˴����ӡ        */
            if (pcTok[8] == 'n') {
                _K_pfuncKernelDebugError = LW_NULL;
            } else {
                _K_pfuncKernelDebugError = bspDebugMsg;
            }
        
        } else if (lib_strncmp(pcTok, "kfpu=", 5) == 0) {               /*  �Ƿ�ʹ���ں˸���֧��        */
            if (pcTok[5] == 'n') {
                _K_bInterFpuEn = LW_FALSE;
            } else {
                _K_bInterFpuEn = LW_TRUE;
            }
        
        } else if (lib_strncmp(pcTok, "heapchk=", 8) == 0) {            /*  �Ƿ���ж��ڴ�Խ����      */
            if (pcTok[8] == 'n') {
                _K_bHeapCrossBorderEn = LW_FALSE;
            } else {
                _K_bHeapCrossBorderEn = LW_TRUE;
            }
        }
        
#if LW_CFG_VMM_EN > 0
          else if (lib_strncmp(pcTok, "varea=", 6) == 0) {              /*  �����ڴ���ʼ��              */
            pvirdesc->ulVirtualSwitch = lib_strtoul(&pcTok[6], LW_NULL, 16);
            pvirdesc->ulVirtualStart  = pvirdesc->ulVirtualSwitch 
                                      + LW_CFG_VMM_PAGE_SIZE;
          
        } else if (lib_strncmp(pcTok, "vsize=", 6) == 0) {              /*  �����ڴ��С                */
            pvirdesc->stSize = (size_t)lib_strtoul(&pcTok[6], LW_NULL, 16);
        }
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
        
        pcTok = lib_strtok_r(LW_NULL, pcDelim, &pcLast);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
