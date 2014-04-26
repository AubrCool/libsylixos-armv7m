/*********************************************************************************************************
**
**                                    �й������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                       SylixOS(TM)
**
**                               Copyright  All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: pthread_spinlock.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2009 �� 12 �� 30 ��
**
** ��        ��: pthread ���������ݿ�.
**
** ע        ��: Upon successful completion, these functions shall return zero; otherwise, 
                 an error number shall be returned to indicate the error.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../include/px_pthread.h"                                      /*  �Ѱ�������ϵͳͷ�ļ�        */
/*********************************************************************************************************
  �ü�֧��
*********************************************************************************************************/
#if LW_CFG_POSIX_EN > 0
/*********************************************************************************************************
** ��������: pthread_spin_init
** ��������: ����һ��������.
** �䡡��  : pspinlock      ������
**           pshare         ���� (Ŀǰ����)
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_spin_init (pthread_spinlock_t  *pspinlock, int  pshare)
{
    if (pspinlock == LW_NULL) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    LW_SPIN_INIT(pspinlock);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: pthread_spin_destroy
** ��������: ����һ��������.
** �䡡��  : pspinlock      ������
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_spin_destroy (pthread_spinlock_t  *pspinlock)
{
    if (pspinlock == LW_NULL) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: pthread_spin_lock
** ��������: lock һ��������.
** �䡡��  : pspinlock      ������
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_spin_lock (pthread_spinlock_t  *pspinlock)
{
    if (pspinlock == LW_NULL) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    LW_SPIN_LOCK(pspinlock);
    
    return  (ERROR_NONE);                                               /*  ���� lock                   */
}
/*********************************************************************************************************
** ��������: pthread_spin_unlock
** ��������: unlock һ��������.
** �䡡��  : pspinlock      ������
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_spin_unlock (pthread_spinlock_t  *pspinlock)
{
    if (pspinlock == LW_NULL) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    LW_SPIN_UNLOCK(pspinlock);
    
    return  (ERROR_NONE);                                               /*  ���� unlock                 */
}
/*********************************************************************************************************
** ��������: pthread_spin_trylock
** ��������: trylock һ��������.
** �䡡��  : pspinlock      ������
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_spin_trylock (pthread_spinlock_t  *pspinlock)
{
    if (pspinlock == LW_NULL) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    if (LW_SPIN_TRYLOCK(pspinlock)) {
        return  (ERROR_NONE);                                           /*  ���� unlock                 */
    
    } else {
        errno = EBUSY;
        return  (EBUSY);
    }
}
/*********************************************************************************************************
** ��������: pthread_spin_lock_irq_np
** ��������: lock һ��������, ͬʱ�����ж�.
** �䡡��  : pspinlock      ������
**           irqctx         ��ϵ�ṹ����ж�״̬����ṹ (�û�����Ҫ���ľ�������)
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
#if LW_CFG_POSIXEX_EN > 0

LW_API 
int  pthread_spin_lock_irq_np (pthread_spinlock_t  *pspinlock, pthread_int_t *irqctx)
{
    if ((pspinlock == LW_NULL) || (irqctx == NULL)) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    LW_SPIN_LOCK_IRQ(pspinlock, irqctx);
    
    return  (ERROR_NONE);                                               /*  ���� lock                   */
}
/*********************************************************************************************************
** ��������: pthread_spin_unlock_irq_np
** ��������: unlock һ��������, ͬʱ�����ж�.
** �䡡��  : pspinlock      ������
**           irqctx         ��ϵ�ṹ����ж�״̬����ṹ (�û�����Ҫ���ľ�������)
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_spin_unlock_irq_np (pthread_spinlock_t  *pspinlock, pthread_int_t irqctx)
{
    if (pspinlock == LW_NULL) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    LW_SPIN_UNLOCK_IRQ(pspinlock, irqctx);
    
    return  (ERROR_NONE);                                               /*  ���� unlock                 */
}
/*********************************************************************************************************
** ��������: pthread_spin_trylock_irq_np
** ��������: trylock һ��������, ����ɹ�ͬʱ�����ж�.
** �䡡��  : pspinlock      ������
**           irqctx         ��ϵ�ṹ����ж�״̬����ṹ (�û�����Ҫ���ľ�������)
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_spin_trylock_irq_np (pthread_spinlock_t  *pspinlock, pthread_int_t *irqctx)
{
    if (pspinlock == LW_NULL) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    if (LW_SPIN_TRYLOCK_IRQ(pspinlock, irqctx)) {
        return  (ERROR_NONE);                                           /*  ���� unlock                 */
    
    } else {
        errno = EBUSY;
        return  (EBUSY);
    }
}
/*********************************************************************************************************
** ��������: pthread_spin_lock_quick_np
** ��������: lock һ��������, ͬʱ�����ж� (����ģʽ).
** �䡡��  : pspinlock      ������
**           irqctx         ��ϵ�ṹ����ж�״̬����ṹ (�û�����Ҫ���ľ�������)
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
** ע  ��  : ���� UNLOCK QUICK ���᳢�Ե���, ���� LOCK QUICK �� UNLOCK QUICK ֮�䲻�ܻ��м��������������
             �ı�����״̬�Ĳ���, �����, ����Ҫʹ�� LOCK IRQ �� UNLOCK IRQ ��ȡ��.
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_spin_lock_quick_np (pthread_spinlock_t  *pspinlock, pthread_int_t *irqctx)
{
    if ((pspinlock == LW_NULL) || (irqctx == NULL)) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    LW_SPIN_LOCK_QUICK(pspinlock, irqctx);
    
    return  (ERROR_NONE);                                               /*  ���� lock                   */
}
/*********************************************************************************************************
** ��������: pthread_spin_unlock_quick_np
** ��������: unlock һ��������, ͬʱ�����ж� (����ģʽ).
** �䡡��  : pspinlock      ������
**           irqctx         ��ϵ�ṹ����ж�״̬����ṹ (�û�����Ҫ���ľ�������)
** �䡡��  : ERROR CODE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API 
int  pthread_spin_unlock_quick_np (pthread_spinlock_t  *pspinlock, pthread_int_t irqctx)
{
    if (pspinlock == LW_NULL) {
        errno = EINVAL;
        return  (EINVAL);
    }
    
    LW_SPIN_UNLOCK_QUICK(pspinlock, irqctx);
    
    return  (ERROR_NONE);                                               /*  ���� unlock                 */
}
#endif                                                                  /*  W_CFG_POSIXEX_EN > 0        */
#endif                                                                  /*  LW_CFG_POSIX_EN > 0         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
