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
** ��   ��   ��: ttinyShellSysCmd.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2008 �� 07 �� 27 ��
**
** ��        ��: һ����С�͵� shell ϵͳ, ϵͳ�ڲ������.
*********************************************************************************************************/

#ifndef __TTINYSHELLSYSCMD_H
#define __TTINYSHELLSYSCMD_H

/*********************************************************************************************************
  �ü�����
*********************************************************************************************************/
#if LW_CFG_SHELL_EN > 0

#define __LW_VT100_FUNC_COLOR          2
#define __LW_VT100_FUNC_ERASE          2
#define __LW_VT100_FUNC_VT52           3

#define __LW_VT100_COLOR_NONE          "m"  
#define __LW_VT100_COLOR_RED           "0;32;31m"  
#define __LW_VT100_COLOR_LIGHT_RED     "1;31m"  
#define __LW_VT100_COLOR_GREEN         "0;32;32m"  
#define __LW_VT100_COLOR_LIGHT_GREEN   "1;32m"  
#define __LW_VT100_COLOR_BLUE          "0;32;34m"  
#define __LW_VT100_COLOR_LIGHT_BLUE    "1;34m"  
#define __LW_VT100_COLOR_DARY_GRAY     "1;30m"  
#define __LW_VT100_COLOR_CYAN          "0;36m"  
#define __LW_VT100_COLOR_LIGHT_CYAN    "1;36m"  
#define __LW_VT100_COLOR_PURPLE        "0;35m"  
#define __LW_VT100_COLOR_LIGHT_PURPLE  "1;35m"  
#define __LW_VT100_COLOR_BROWN         "0;33m"  
#define __LW_VT100_COLOR_YELLOW        "1;33m"  
#define __LW_VT100_COLOR_LIGHT_GRAY    "0;37m"  
#define __LW_VT100_COLOR_WHITE         "1;37m"  

#define __LW_VT100_CURSOR_TO_END_LINE       "0K"
#define __LW_VT100_BIGEN_LINE_TO_CURSOR     "1K"
#define __LW_VT100_ENTIRE_LINE              "2K"
#define __LW_VT100_CURSOR_TO_END_SCREEN     "0J"
#define __LW_VT100_BIGEN_SCREEN_TO_CURSOR   "1J"

#define __LW_VT52_CURSOR_TO_HOME            "H"

VOID  __tshellSysCmdInit(VOID);

#endif                                                                  /*  LW_CFG_SHELL_EN > 0         */
#endif                                                                  /*  __TTINYSHELLSYSCMD_H        */
/*********************************************************************************************************
  END
*********************************************************************************************************/
