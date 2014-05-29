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
** ��   ��   ��: EventSetTryGet.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2006 �� 12 �� 20 ��
**
** ��        ��: �������ȴ��¼�������¼�   (���� uCOS ���ƣ�������ʽ��Ӧ����)

** BUG
2008.01.13  ���� _DebugHandle() ���ܡ�
2009.04.08  ����� SMP ��˵�֧��.
2012.03.20  ���ٶ� _K_ptcbTCBCur ������, �������þֲ�����, ���ٶԵ�ǰ CPU ID ��ȡ�Ĵ���.
2013.07.18  ʹ���µĻ�ȡ TCB �ķ���, ȷ�� SMP ϵͳ��ȫ.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
             ERROR MACRO
*********************************************************************************************************/
#define  __EVENTSET_NOT_READY() do {                          \
             LW_SPIN_UNLOCK_QUICK(&pes->EVENTSET_slLock,      \
                                    iregInterLevel);          \
             __KERNEL_EXIT();                                 \
             _ErrorHandle(ERROR_THREAD_WAIT_TIMEOUT);         \
             return (ERROR_THREAD_WAIT_TIMEOUT);              \
         } while (0)
/*********************************************************************************************************
** ��������: API_EventSetTryGet
** ��������: �������ȴ��¼�������¼�
** �䡡��  : 
**           ulId            �¼������
**           ulEvent         �ȴ��¼�
**           ulOption        �ȴ�����ѡ��
** �䡡��  : �¼����
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
#if (LW_CFG_EVENTSET_EN > 0) && (LW_CFG_MAX_EVENTSETS > 0)

LW_API  
ULONG  API_EventSetTryGet (LW_OBJECT_HANDLE  ulId, 
                           ULONG             ulEvent,
                           ULONG             ulOption)
{
             INTREG                iregInterLevel;
             
             PLW_CLASS_TCB         ptcbCur;
    REGISTER UINT16                usIndex;
    REGISTER PLW_CLASS_EVENTSET    pes;
    REGISTER BOOL                  bIsReset;
    REGISTER UINT8                 ucWaitType;
    REGISTER ULONG                 ulEventRdy;
    
    usIndex = _ObjectGetIndex(ulId);
    
    if (ulOption & LW_OPTION_EVENTSET_RESET) {                          /*  �Ƿ���Ҫ��λ���λ          */
        bIsReset = LW_TRUE;
    } else {
        bIsReset = LW_FALSE;
    }
    
    ucWaitType = (UINT8)(ulOption & 0x0F);                              /*  ��õȴ�����                */
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);                                       /*  ��ǰ������ƿ�              */
    
#if LW_CFG_ARG_CHK_EN > 0
    if (!_ObjectClassOK(ulId, _OBJECT_EVENT_SET)) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "eventset handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
    if (_EventSet_Index_Invalid(usIndex)) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "eventset handle invalidate.\r\n");
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return  (ERROR_KERNEL_HANDLE_NULL);
    }
#endif
    pes = &_K_esBuffer[usIndex];
    
    LW_SPIN_LOCK_QUICK(&pes->EVENTSET_slLock, &iregInterLevel);         /*  �ر��ж�ͬʱ��ס spinlock   */
    
    if (_EventSet_Type_Invalid(usIndex, LW_TYPE_EVENT_EVENTSET)) {
        LW_SPIN_UNLOCK_QUICK(&pes->EVENTSET_slLock, iregInterLevel);    /*  ���ж�, ͬʱ�� spinlock */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "eventset handle invalidate.\r\n");
        _ErrorHandle(ERROR_EVENTSET_TYPE);
        return  (ERROR_EVENTSET_TYPE);
    }
    
    __KERNEL_ENTER();                                                   /*  �����ں�                    */
    
    switch (ucWaitType) {
    
    case LW_OPTION_EVENTSET_WAIT_SET_ALL:
        ulEventRdy = (ulEvent & pes->EVENTSET_ulEventSets);
        if (ulEvent == ulEventRdy) {
            if (bIsReset) {                                             /*  ���ĵ����λ                */
                pes->EVENTSET_ulEventSets &= (~ulEventRdy);
                ptcbCur->TCB_ulEventSets = ulEventRdy;
                LW_SPIN_UNLOCK_QUICK(&pes->EVENTSET_slLock, 
                                       iregInterLevel);                 /*  ���ж�, ͬʱ�� spinlock */
                __KERNEL_EXIT();                                        /*  �˳��ں�                    */
                API_EventSetSet(ulId, 0, LW_OPTION_EVENTSET_SET);       /*  ���ܻ����������񱻼���      */
            } else {
                ptcbCur->TCB_ulEventSets = ulEventRdy;
                LW_SPIN_UNLOCK_QUICK(&pes->EVENTSET_slLock, 
                                       iregInterLevel);                 /*  ���ж�, ͬʱ�� spinlock */
                __KERNEL_EXIT();                                        /*  �˳��ں�                    */
            }
            return  (ERROR_NONE);
        } else {
            __EVENTSET_NOT_READY();
        }
        break;
        
    case LW_OPTION_EVENTSET_WAIT_SET_ANY:
        ulEventRdy = (ulEvent & pes->EVENTSET_ulEventSets);
        if (ulEventRdy) {
            if (bIsReset) {
                pes->EVENTSET_ulEventSets &= (~ulEventRdy);
                ptcbCur->TCB_ulEventSets = ulEventRdy;
                LW_SPIN_UNLOCK_QUICK(&pes->EVENTSET_slLock, 
                                       iregInterLevel);                 /*  ���ж�, ͬʱ�� spinlock */
                __KERNEL_EXIT();                                        /*  �˳��ں�                    */
                API_EventSetSet(ulId, 0, LW_OPTION_EVENTSET_SET);       /*  ���ܻ����������񱻼���      */
            } else {
                ptcbCur->TCB_ulEventSets = ulEventRdy;
                LW_SPIN_UNLOCK_QUICK(&pes->EVENTSET_slLock, 
                                       iregInterLevel);                 /*  ���ж�, ͬʱ�� spinlock */
                __KERNEL_EXIT();                                        /*  �˳��ں�                    */
            }
            return  (ERROR_NONE);
        } else {
            __EVENTSET_NOT_READY();
        }
        break;
        
    case LW_OPTION_EVENTSET_WAIT_CLR_ALL:
        ulEventRdy = (ulEvent & ~pes->EVENTSET_ulEventSets);
        if (ulEvent == ulEventRdy) {
            if (bIsReset) {
                pes->EVENTSET_ulEventSets |= ulEventRdy;                /*  ����������λ              */
                ptcbCur->TCB_ulEventSets = ulEventRdy;
                LW_SPIN_UNLOCK_QUICK(&pes->EVENTSET_slLock, 
                                       iregInterLevel);                 /*  ���ж�, ͬʱ�� spinlock */
                __KERNEL_EXIT();                                        /*  �˳��ں�                    */
                API_EventSetSet(ulId, 0, LW_OPTION_EVENTSET_SET);       /*  ���ܻ����������񱻼���      */
            } else {
                ptcbCur->TCB_ulEventSets = ulEventRdy;
                LW_SPIN_UNLOCK_QUICK(&pes->EVENTSET_slLock, 
                                       iregInterLevel);                 /*  ���ж�, ͬʱ�� spinlock */
                __KERNEL_EXIT();                                        /*  �˳��ں�                    */
            }
            return  (ERROR_NONE);
        } else {
            __EVENTSET_NOT_READY();
        }
        break;
        
    case LW_OPTION_EVENTSET_WAIT_CLR_ANY:
        ulEventRdy = (ulEvent & ~pes->EVENTSET_ulEventSets);
        if (ulEventRdy) {
            if (bIsReset) {
                pes->EVENTSET_ulEventSets |= ulEventRdy;                /*  ����������λ              */
                ptcbCur->TCB_ulEventSets = ulEventRdy;
                LW_SPIN_UNLOCK_QUICK(&pes->EVENTSET_slLock, 
                                       iregInterLevel);                 /*  ���ж�, ͬʱ�� spinlock */
                __KERNEL_EXIT();                                        /*  �˳��ں�                    */
                API_EventSetSet(ulId, 0, LW_OPTION_EVENTSET_SET);       /*  ���ܻ����������񱻼���      */
            } else {
                ptcbCur->TCB_ulEventSets = ulEventRdy;
                LW_SPIN_UNLOCK_QUICK(&pes->EVENTSET_slLock, 
                                       iregInterLevel);                 /*  ���ж�, ͬʱ�� spinlock */
                __KERNEL_EXIT();                                        /*  �˳��ں�                    */
            }
            return  (ERROR_NONE);
        } else {
            __EVENTSET_NOT_READY();
        }
        break;
        
    default:
        LW_SPIN_UNLOCK_QUICK(&pes->EVENTSET_slLock, iregInterLevel);    /*  ���ж�, ͬʱ�� spinlock */
        __KERNEL_EXIT();                                                /*  �˳��ں�                    */
        _ErrorHandle(ERROR_EVENTSET_WAIT_TYPE);
        return  (ERROR_EVENTSET_WAIT_TYPE);
    }
}

#endif                                                                  /*  (LW_CFG_EVENTSET_EN > 0)    */
                                                                        /*  (LW_CFG_MAX_EVENTSETS > 0)  */
/*********************************************************************************************************
  END
*********************************************************************************************************/
