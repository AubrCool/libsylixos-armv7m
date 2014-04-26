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
** ��   ��   ��: lib_strncat.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2006 �� 12 �� 25 ��
**
** ��        ��: ��

** BUG:
2009.07.18  ���� strlcat() ����.
*********************************************************************************************************/
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** ��������: lib_strncat
** ��������: At most strlen(pcDest) + iN + 1 bytes
** �䡡��  : 
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
PCHAR  lib_strncat (PCHAR  pcDest, CPCHAR  pcSrc, size_t  stN)
{    
    REGISTER PCHAR    pcDestReg = (PCHAR)pcDest;
    REGISTER PCHAR    pcSrcReg  = (PCHAR)pcSrc;
    
    while (*pcDestReg != '\0') {
        pcDestReg++;
    }
    
    while (*pcSrcReg != '\0' && stN > 0) {
        *pcDestReg++ = *pcSrcReg++;
        stN--;
    }
    *pcDestReg = '\0';
    
    return  (pcDest);
}
/*********************************************************************************************************
** ��������: lib_strlcat
** ��������: unlike strncat, stN is the full size of dst, not space left.
** �䡡��  : 
** �䡡��  : 
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
size_t  lib_strlcat (PCHAR  pcDest, CPCHAR  pcSrc, size_t  stN)
{
    REGISTER PCHAR    pcDestReg = (PCHAR)pcDest;
    REGISTER PCHAR    pcSrcReg  = (PCHAR)pcSrc;
             size_t   stCnt     = stN;
             size_t   stDlen;
    
    while ((stCnt > 0) && (*pcDestReg != '\0')) {
        pcDestReg++;
        stCnt--;
    }
    
    stDlen = (size_t)(pcDestReg - pcDest);
    stCnt  = stN - stDlen;
    
    if (stCnt == 0) {
        return  (stDlen + lib_strlen(pcSrc));
    }
    
    while (*pcSrcReg != '\0') {
        if (stCnt > 1) {
            *pcDestReg++ = *pcSrcReg;
            stCnt--;
        }
        pcSrcReg++;
    }
    *pcDestReg = '\0';
    
    /* 
     * Returns strlen(pcSrc) + MIN(stN, strlen(initial pcDest)).
     */
    return  (stDlen + (pcSrcReg - pcSrc));
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
