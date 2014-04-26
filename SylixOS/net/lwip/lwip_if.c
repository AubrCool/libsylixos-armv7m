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
** ��   ��   ��: lwip_if.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2011 �� 03 �� 10 ��
**
** ��        ��: posix net/if �ӿ�.

** BUG:
2011.07.07  _G_ulNetifLock ����.
2014.03.22  �Ż��������ӿ�.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
/*********************************************************************************************************
  �ü�����
*********************************************************************************************************/
#if LW_CFG_NET_EN > 0
#include "lwip/netif.h"
#include "net/if.h"
/*********************************************************************************************************
  ȫ�ֱ���
*********************************************************************************************************/
extern LW_OBJECT_HANDLE     _G_ulNetifLock;
/*********************************************************************************************************
  ����ӿ���
*********************************************************************************************************/
#define LWIP_NETIF_LOCK()   API_SemaphoreBPend(_G_ulNetifLock, LW_OPTION_WAIT_INFINITE)
#define LWIP_NETIF_UNLOCK() API_SemaphoreBPost(_G_ulNetifLock)
/*********************************************************************************************************
** ��������: if_nametoindex
** ��������: map a network interface name to its corresponding index
** �䡡��  : ifname        if name
** �䡡��  : index
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
unsigned  if_nametoindex (const char *ifname)
{
    struct netif    *pnetif;
    unsigned         uiIndex = 0;
    
    LWIP_NETIF_LOCK();                                                  /*  �����ٽ���                  */
    pnetif = netif_find((char *)ifname);
    if (pnetif) {
        uiIndex = (unsigned)pnetif->num;
    }
    LWIP_NETIF_UNLOCK();                                                /*  �˳��ٽ���                  */
    
    return  (uiIndex);
}
/*********************************************************************************************************
** ��������: if_indextoname
** ��������: map a network interface index to its corresponding name
** �䡡��  : ifindex       if index
**           ifname        if name buffer at least {IF_NAMESIZE} bytes
** �䡡��  : index
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
char *if_indextoname (unsigned  ifindex, char *ifname)
{
    struct netif    *pnetif;

    if (!ifname) {
        errno = EINVAL;
    }
    
    LWIP_NETIF_LOCK();                                                  /*  �����ٽ���                  */
    pnetif = (struct netif *)netif_get_by_index(ifindex);
    if (pnetif) {
        ifname[0] = pnetif->name[0];
        ifname[1] = pnetif->name[1];
        ifname[2] = (char)(pnetif->num + '0');
        ifname[3] = PX_EOS;
    }
    LWIP_NETIF_UNLOCK();                                                /*  �˳��ٽ���                  */
    
    if (pnetif) {
        return  (ifname);
    } else {
        errno = ENXIO;
        return  (LW_NULL);
    }
}
/*********************************************************************************************************
** ��������: if_nameindex
** ��������: return all network interface names and indexes
** �䡡��  : NONE
** �䡡��  : An array of structures identifying local interfaces
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
struct if_nameindex *if_nameindex (void)
{
    struct netif           *pnetif;
    int                     iNum = 1;                                   /*  ��Ҫһ�����е�λ��          */
    struct if_nameindex    *pifnameindexArry;
    
    LWIP_NETIF_LOCK();                                                  /*  �����ٽ���                  */
    for(pnetif = netif_list; pnetif != LW_NULL; pnetif = pnetif->next) {
        iNum++;
    }
    pifnameindexArry = (struct if_nameindex *)__SHEAP_ALLOC(sizeof(struct if_nameindex) * (size_t)iNum);
    if (pifnameindexArry) {
        int     i = 0;
        
        for(pnetif  = netif_list; 
            pnetif != LW_NULL; 
            pnetif  = pnetif->next) {
            
            pifnameindexArry[i].if_index = (unsigned)pnetif->num;
            pifnameindexArry[i].if_name_buf[0] = pnetif->name[0];
            pifnameindexArry[i].if_name_buf[1] = pnetif->name[1];
            pifnameindexArry[i].if_name_buf[2] = (char)(pnetif->num + '0');
            pifnameindexArry[i].if_name_buf[3] = PX_EOS;
            pifnameindexArry[i].if_name = pifnameindexArry[i].if_name_buf;
            i++;
        }
        
        pifnameindexArry[i].if_index = 0;
        pifnameindexArry[i].if_name_buf[0] = PX_EOS;
        pifnameindexArry[i].if_name = pifnameindexArry[i].if_name_buf;
        
    } else {
        errno = ENOMEM;
    }
    LWIP_NETIF_UNLOCK();                                                /*  �˳��ٽ���                  */
    
    return  (pifnameindexArry);
}
/*********************************************************************************************************
** ��������: if_freenameindex
** ��������: free memory allocated by if_nameindex
             the application shall not use the array of which ptr is the address.
** �䡡��  : ptr           shall be a pointer that was returned by if_nameindex().
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
void  if_freenameindex (struct if_nameindex *ptr)
{
    if (ptr) {
        __SHEAP_FREE(ptr);
    }
}
#endif                                                                  /*  LW_CFG_NET_EN               */
/*********************************************************************************************************
  END
*********************************************************************************************************/
