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
** ��   ��   ��: powerMLib.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2008 �� 03 �� 06 ��
**
** ��        ��: ϵͳ�����Ĺ�����.

** BUG
2008.03.16  �����̲߳�����ʹ�� select() ����.
2009.04.09  ʹ����Դ�������.
2009.08.31  ����ע��.
2012.03.12  ʹ���Զ� attr
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#define  __POWERM_MAIN_FILE
#include "../SylixOS/system/include/s_system.h"
/*********************************************************************************************************
  �ü�����
*********************************************************************************************************/
#if (LW_CFG_POWERM_EN > 0) && (LW_CFG_MAX_POWERM_NODES > 0)
/*********************************************************************************************************
  ȫ�ֱ���
*********************************************************************************************************/
LW_CLASS_POWERM_NODE            _G__pmnControlNode[LW_CFG_MAX_POWERM_NODES];
LW_OBJECT_HANDLE                _G_ulPowerMLock;
LW_CLASS_WAKEUP                 _G_wuPowerM;
/*********************************************************************************************************
  ��������
*********************************************************************************************************/
static PVOID  _PowerMThread(PVOID  pvArg);
/*********************************************************************************************************
** ��������: _PowerMListInit
** ��������: ��ʼ��ϵͳ�����Ĺ���������
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  _PowerMListInit (VOID)
{
#if LW_CFG_MAX_POWERM_NODES == 1                                        /*  ����һ���ڵ�                */

    REGISTER PLW_CLASS_POWERM_NODE      p_pmnTemp1;
    
    _G_resrcPower.RESRC_pmonoFreeHeader = &_G__pmnControlNode[0].PMN_monoResrcList;
    
    p_pmnTemp1 = &_G__pmnControlNode[0];
    p_pmnTemp1->PMN_usIndex = 0;
    
    _INIT_LIST_MONO_HEAD(_G_resrcPower.RESRC_pmonoFreeHeader);
    
    _G_resrcPower.RESRC_pmonoFreeTail = _G_resrcPower.RESRC_pmonoFreeHeader;
    
#else                                                                   /*  ����������ƽڵ�            */
    
    REGISTER ULONG                  ulI;
    REGISTER PLW_LIST_MONO          pmonoTemp1;
    REGISTER PLW_LIST_MONO          pmonoTemp2;
    REGISTER PLW_CLASS_POWERM_NODE  p_pmnTemp1;
    REGISTER PLW_CLASS_POWERM_NODE  p_pmnTemp2;
    
    _G_resrcPower.RESRC_pmonoFreeHeader = &_G__pmnControlNode[0].PMN_monoResrcList;
    
    p_pmnTemp1 = &_G__pmnControlNode[0];
    p_pmnTemp2 = &_G__pmnControlNode[1];
    
    for (ulI = 0; ulI < ((LW_CFG_MAX_POWERM_NODES) - 1); ulI++) {
    
        p_pmnTemp1->PMN_usIndex = (UINT16)ulI;
        
        pmonoTemp1 = &p_pmnTemp1->PMN_monoResrcList;
        pmonoTemp2 = &p_pmnTemp2->PMN_monoResrcList;
        
        _LIST_MONO_LINK(pmonoTemp1, pmonoTemp2);
        
        p_pmnTemp1++;
        p_pmnTemp2++;
    }
    
    p_pmnTemp1->PMN_usIndex = (UINT16)ulI;
    
    pmonoTemp1 = &p_pmnTemp1->PMN_monoResrcList;
    
    _INIT_LIST_MONO_HEAD(pmonoTemp1);
    
    _G_resrcPower.RESRC_pmonoFreeTail = pmonoTemp1;
    
#endif                                                                  /*  LW_CFG_MAX_POWERM_NODES == 1*/

    _G_resrcPower.RESRC_uiUsed    = 0;
    _G_resrcPower.RESRC_uiMaxUsed = 0;

    __WAKEUP_INIT(&_G_wuPowerM);
}
/*********************************************************************************************************
** ��������: _PowerMInit
** ��������: ��ʼ��ϵͳ�����Ĺ�����
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _PowerMInit (VOID)
{
    LW_CLASS_THREADATTR       threadattr;

    _PowerMListInit();                                                  /*  ��ʼ����������              */
    
    _G_ulPowerMLock = API_SemaphoreMCreate("power_lock", LW_PRIO_DEF_CEILING,
                               LW_OPTION_WAIT_PRIORITY | LW_OPTION_DELETE_SAFE |
                               LW_OPTION_INHERIT_PRIORITY | LW_OPTION_OBJECT_GLOBAL,
                               LW_NULL);                                /*  �������ź���                */

    if (!_G_ulPowerMLock) {
        return;                                                         /*  ʧ��                        */
    }
    
    API_ThreadAttrBuild(&threadattr, 
                        LW_CFG_THREAD_POWERM_STK_SIZE, 
                        LW_PRIO_T_POWER, 
                        LW_OPTION_THREAD_STK_CHK | LW_OPTION_THREAD_SAFE | LW_OPTION_OBJECT_GLOBAL, 
                        LW_NULL);
    
	_S_ulPowerMId = API_ThreadCreate("t_power", 
	                                 _PowerMThread,
	                                 &threadattr,
	                                 LW_NULL);
}
/*********************************************************************************************************
** ��������: _PowerMThread
** ��������: ��ʼ��ϵͳ�����Ĺ�����
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : �Զ�ʱ���Ĳ���û�йر��ж�. ������Ҫ signal �Ĳ����������ж��е���.
*********************************************************************************************************/
static  PVOID  _PowerMThread (PVOID  pvArg)
{
    REGISTER PLW_CLASS_POWERM_NODE  p_pmnNode;
    REGISTER PLW_CLASS_WAKEUP_NODE  pwun;
    
    for (;;) {
        ULONG   ulCounter = LW_CFG_TICKS_PER_SEC;
        
        __POWERM_LOCK();                                                /*  ����                        */
                           
        __WAKEUP_PASS_FIRST(&_G_wuPowerM, pwun, ulCounter);
        
        p_pmnNode = _LIST_ENTRY(pwun, LW_CLASS_POWERM_NODE, PMN_wunTimer);
        
        _WakeupDel(&_G_wuPowerM, pwun);
        
        if (p_pmnNode->PMN_pfuncCallback) {
            __POWERM_UNLOCK();                                          /*  ��ʱ����                    */
            
            p_pmnNode->PMN_pfuncCallback(p_pmnNode->PMN_pvArg);
            
            __POWERM_LOCK();                                            /*  ����                        */
        }
        
        __WAKEUP_PASS_SECOND();
        
        __WAKEUP_PASS_END();
        
        __POWERM_UNLOCK();                                              /*  ����                        */
        
        API_TimeSleep(LW_CFG_TICKS_PER_SEC);                            /*  �ȴ�һ��                    */
    }
    
    return  (LW_NULL);
}
/*********************************************************************************************************
** ��������: _Allocate_PowerM_Object
** ��������: �ӿ��й��Ĺ���������, ���һ�����еĿ��ƿ�.
** �䡡��  : NONE
** �䡡��  : ��ÿ��ƿ��ַ, ʧ�ܷ��� NULL
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
PLW_CLASS_POWERM_NODE  _Allocate_PowerM_Object (VOID)
{
    REGISTER PLW_LIST_MONO          pmonoFree;
    REGISTER PLW_CLASS_POWERM_NODE  p_pmnTemp;
    
    if (_LIST_MONO_IS_EMPTY(_G_resrcPower.RESRC_pmonoFreeHeader)) {
        return  (LW_NULL);
    }
    
    pmonoFree = _list_mono_allocate_seq(&_G_resrcPower.RESRC_pmonoFreeHeader, 
                                        &_G_resrcPower.RESRC_pmonoFreeTail);
                                                                        /*  �����Դ                    */
    p_pmnTemp = _LIST_ENTRY(pmonoFree, 
                            LW_CLASS_POWERM_NODE, 
                            PMN_monoResrcList);                         /*  ��ÿ��ƿ��׵�ַ            */
                            
    _G_resrcPower.RESRC_uiUsed++;
    if (_G_resrcPower.RESRC_uiUsed > _G_resrcPower.RESRC_uiMaxUsed) {
        _G_resrcPower.RESRC_uiMaxUsed = _G_resrcPower.RESRC_uiUsed;
    }
    
    return  (p_pmnTemp);
}
/*********************************************************************************************************
** ��������: _Free_PowerM_Object
** ��������: ������ʹ�õĹ��Ĺ������Żؿ��п����
** �䡡��  : p_pmnFree     ����ʹ�õĹ��Ĺ�����
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _Free_PowerM_Object (PLW_CLASS_POWERM_NODE    p_pmnFree)
{
    REGISTER PLW_LIST_MONO          pmonoFree;
    
    pmonoFree = &p_pmnFree->PMN_monoResrcList;
    
    _list_mono_free_seq(&_G_resrcPower.RESRC_pmonoFreeHeader, 
                        &_G_resrcPower.RESRC_pmonoFreeTail, 
                        pmonoFree);
                        
    _G_resrcPower.RESRC_uiUsed--;
}
#endif                                                                  /*  LW_CFG_POWERM_EN            */
                                                                        /*  LW_CFG_MAX_POWERM_NODES     */
/*********************************************************************************************************
  END
*********************************************************************************************************/

