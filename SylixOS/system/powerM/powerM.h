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
** ��   ��   ��: powerM.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2008 �� 03 �� 06 ��
**
** ��        ��: ϵͳ�����Ĺ�����. �����û�����, �ɷ�Ϊ������Ŀ��Ƶ�, ÿ�����Ƶ���������ӳ�ʱ��.
                 ����, ϵͳ�к��� LCD, ��ô���Է�Ϊ�������Ŀ��Ƶ�. ������һ��ʱ�����˲�����, ����
                 �ر� LCD ����, �ھ���һ��ʱ�����˲�����, �ɹر� LCD ��ʾϵͳ�Խ��͹���.
*********************************************************************************************************/

#ifndef __POWERM_H
#define __POWERM_H

/*********************************************************************************************************
  �ü�����
*********************************************************************************************************/
#if (LW_CFG_POWERM_EN > 0) && (LW_CFG_MAX_POWERM_NODES > 0)

/*********************************************************************************************************
  ��Դ����
*********************************************************************************************************/

#ifdef __SYLIXOS_KERNEL

#ifdef  __POWERM_MAIN_FILE
       LW_CLASS_OBJECT_RESRC    _G_resrcPower;
#else
extern LW_CLASS_OBJECT_RESRC    _G_resrcPower;
#endif                                                                  /*  __POWERM_MAIN_FILE          */

#define __POWERM_LOCK()         API_SemaphoreMPend(_G_ulPowerMLock, LW_OPTION_WAIT_INFINITE)
#define __POWERM_UNLOCK()       API_SemaphoreMPost(_G_ulPowerMLock)

#endif                                                                  /*  __SYLIXOS_KERNEL            */

/*********************************************************************************************************
  ���Ĺ���ڵ�
*********************************************************************************************************/
typedef struct {
    LW_LIST_MONO            PMN_monoResrcList;                          /*  ��Դ����                    */
    BOOL                    PMN_bIsUsing;                               /*  �Ƿ񱻴�����                */
    
    LW_CLASS_WAKEUP_NODE    PMN_wunTimer;
#define PMN_bInQ            PMN_wunTimer.WUN_bInQ
#define PMN_ulCounter       PMN_wunTimer.WUN_ulCounter
    
    ULONG                   PMN_ulCounterSave;                          /*  ��������ֵ                  */
    PVOID                   PMN_pvArg;                                  /*  �ص�����                    */
    LW_HOOK_FUNC            PMN_pfuncCallback;                          /*  �ص�����                    */
    
    UINT16                  PMN_usIndex;                                /*  �����±�                    */
    CHAR                    PMN_cPowerMName[LW_CFG_OBJECT_NAME_SIZE];   /*  ����                        */
} LW_CLASS_POWERM_NODE;
typedef LW_CLASS_POWERM_NODE    *PLW_CLASS_POWERM_NODE;

/*********************************************************************************************************
  INLINE FUNCTION
*********************************************************************************************************/

static LW_INLINE INT  _PowerM_Index_Invalid (UINT16    usIndex)
{
    return  (usIndex >= LW_CFG_MAX_POWERM_NODES);
}

#endif                                                                  /*  LW_CFG_POWERM_EN            */
                                                                        /*  LW_CFG_MAX_POWERM_NODES     */
#endif                                                                  /*  __POWERM_H                  */
/*********************************************************************************************************
  END
*********************************************************************************************************/
