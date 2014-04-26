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
** ��   ��   ��: ttinyShell.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2008 �� 07 �� 27 ��
**
** ��        ��: һ����С�͵� shell ϵͳ, ʹ�� tty/pty ���ӿ�, ��Ҫ���ڵ�����򵥽���.
*********************************************************************************************************/

#ifndef __TTINYSHELL_H
#define __TTINYSHELL_H

/*********************************************************************************************************
  �ü�����
*********************************************************************************************************/
#if LW_CFG_SHELL_EN > 0

/*********************************************************************************************************
  �ص���������
*********************************************************************************************************/

typedef INT               (*PCOMMAND_START_ROUTINE)(INT  iArgC, PCHAR  ppcArgV[]);

/*********************************************************************************************************
  API
*********************************************************************************************************/
LW_API VOID                 API_TShellInit(VOID);                       /*  ��װ tshell ϵͳ            */

LW_API INT                  API_TShellSetStackSize(size_t  stNewSize, size_t  *pstOldSize);
                                                                        /*  ���ö�ջ��С                */

LW_API LW_OBJECT_HANDLE     API_TShellCreate(INT  iTtyFd, 
                                             ULONG  ulOption);          /*  ����һ�� tshell �ն�        */

LW_API LW_OBJECT_HANDLE     API_TShellCreateEx(INT      iTtyFd, 
                                               ULONG    ulOption,
                                               FUNCPTR  pfuncRunCallback);
                                                                        /*  ����һ�� tshell �ն���չ    */

LW_API INT                  API_TShellGetUserName(uid_t  uid, PCHAR  pcName, size_t  stSize);
                                                                        /*  ͨ�� shell �����ȡ�û���   */
LW_API INT                  API_TShellGetGrpName(gid_t  gid, PCHAR  pcName, size_t  stSize);
                                                                        /*  ͨ�� shell �����ȡ����     */
LW_API VOID                 API_TShellFlushCache(VOID);                 /*  ˢ�� shell ���ֻ���         */

LW_API ULONG                API_TShellKeywordAdd(CPCHAR  pcKeyword, 
                                                 PCOMMAND_START_ROUTINE  pfuncCommand);  
                                                                        /*  �� tshell ϵͳ����ӹؼ���  */
LW_API ULONG                API_TShellKeywordAddEx(CPCHAR  pcKeyword, 
                                                   PCOMMAND_START_ROUTINE  pfuncCommand, 
                                                   ULONG  ulOption);    /*  �� tshell ϵͳ����ӹؼ���  */
LW_API ULONG                API_TShellFormatAdd(CPCHAR  pcKeyword, CPCHAR  pcFormat);
                                                                        /*  ��ĳһ���ؼ�����Ӹ�ʽ����  */
LW_API ULONG                API_TShellHelpAdd(CPCHAR  pcKeyword, CPCHAR  pcHelp);
                                                                        /*  ��ĳһ���ؼ�����Ӱ���      */
                                                                        
LW_API INT                  API_TShellExec(CPCHAR  pcCommandExec);      /*  �ڵ�ǰ�Ļ�����, ִ��һ��    */
                                                                        /*  shell ָ��                  */
LW_API INT                  API_TShellExecBg(CPCHAR  pcCommandExec, INT  iFd[3], BOOL  bClosed[3], 
                                             BOOL  bIsJoin, LW_OBJECT_HANDLE *pulSh);
                                                                        /*  ����ִ��һ�� shell ����     */
LW_API INT                  API_TShellCtlCharSend(ULONG  ulFunc, PCHAR  pcArg);
                                                                        /*  ����һ��ת������            */
                                                                        
#define tshellInit          API_TShellInit
#define tshellSetStackSize  API_TShellSetStackSize
#define tshellCreate        API_TShellCreate
#define tshellCreateEx      API_TShellCreateEx
#define tshellGetUserName   API_TShellGetUserName
#define tshellGetGrpName    API_TShellGetGrpName
#define tshellFlushCache    API_TShellFlushCache
#define tshellKeywordAdd    API_TShellKeywordAdd
#define tshellKeywordAddEx  API_TShellKeywordAddEx
#define tshellFormatAdd     API_TShellFormatAdd
#define tshellHelpAdd       API_TShellHelpAdd
#define tshellExec          API_TShellExec
#define tshellExecBg        API_TShellExecBg
#define tshellCtlCharSend   API_TShellCtlCharSend
                                                                        
#endif                                                                  /*  LW_CFG_SHELL_EN > 0         */
#endif                                                                  /*  __TTINYSHELL_H              */
/*********************************************************************************************************
  END
*********************************************************************************************************/
