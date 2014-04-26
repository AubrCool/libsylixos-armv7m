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
** ��   ��   ��: dtrace.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2011 �� 11 �� 18 ��
**
** ��        ��: SylixOS ���Ը�����, GDB server ����ʹ�ô˵��Ը���������װ�ص�ģ����߽���.
*********************************************************************************************************/

#ifndef __DTRACE_H
#define __DTRACE_H

/*********************************************************************************************************
  API
*********************************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif                                                                  /*  __cplusplus                 */

LW_API PVOID            dtrace_create(BOOL (*pfuncTrap)(), PVOID pvArg);
LW_API INT              dtrace_delete(PVOID pvDtrace);

LW_API INT              dtrace_read(PVOID pvDtrace, addr_t ulAddress, PVOID pvBuffer, size_t  stNbytes);
LW_API INT              dtrace_write(PVOID pvDtrace, addr_t ulAddress, 
                                     const PVOID pvBuffer, size_t  stNbytes);
                                     
LW_API INT              dtrace_continue(PVOID pvDtrace);
LW_API INT              dtrace_continue_one(PVOID pvDtrace);

/*********************************************************************************************************
  API (SylixOS internal use only!)
*********************************************************************************************************/

LW_API INT              dtrace_trap(PVOID              pvFrame,
                                    addr_t             ulAddress, 
                                    ULONG              ulType,
                                    LW_OBJECT_HANDLE   ulThread);
                                    
#ifdef __cplusplus
}
#endif                                                                  /*  __cplusplus                 */

#endif                                                                  /*  __DTRACE_H                  */
/*********************************************************************************************************
  END
*********************************************************************************************************/
