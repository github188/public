#ifndef	__NET_PUBLIC_H__
#define	__NET_PUBLIC_H__
/*
author : ydl	   2016.01.18
notes:if you want use these API,and the file size is > 4G,please -D_FILE_OFFSET_BITS=64
*/
#ifdef __cplusplus
extern "C" {
#endif



#define ETH0_INTERFACE	("eth0")

#define ETH1_INTERFACE	("eth1") 

int get_macAddr(char *src, unsigned char *dst, int num);
unsigned int get_gateWay(char *interface_name);
unsigned int get_ipAddr(char *interface_name);
int get_ipAddrstring(char *interface_name, char *ipaddr);
unsigned int get_netMask(char *interface_name);
unsigned int get_broadCast(char *interface_name);

int set_ip(char *filename,char *ip,char *netmask,char *gw);//注意临时生效
int set_dhcp(char *filename);
int set_dns(char *dns);

#endif
#endif
