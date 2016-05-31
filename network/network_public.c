/**************************************************************************************************
*													注意事项
*
*					1.
*					2.
*																																					write by zm
****************************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <asm/types.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <net/if.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/time.h>

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <linux/icmp.h>
#include <strings.h>
#include <netinet/ip_icmp.h>
#include <stdlib.h>

#include "edukit_port.h"
#include "share_socket.h"

//#include "reach_network.h"
#include "edukit_network.h"

#define BUFSIZE 8192
#define IPADDR_LEN 16

struct route_info {
	u_int dstAddr;
	u_int srcAddr;
	u_int gateWay;
	char ifName[IF_NAMESIZE];
};

/*
* 将MAC地址的字符串转换成字符数组中
*
* 如:  src = "00:11:22:33:44:55" --->
*		dst[0] = 0x00 dst[1] = 0x11 dst[2] = 0x22;
*
*/
int split_macAddr(char *src, unsigned char *dst, int num)
{
	char *p;
	char *q = src;
	int val = 0  , i = 0;

	for(i = 0 ; i < num ; i++) {
		p = strstr(q, ":");

		if(!p) {
			val = strtol(q, 0, 16);
			dst[i]  = val ;

			if(i == num - 1) {
				continue;
			} else {
				return -1;
			}
		}

		*(p++) = '\0';
		val = strtol(q, 0, 16);
		dst[i]  = val ;
		q = p;
	}

	return 0;
}

unsigned int get_gateWay(char *interface_name)
{
	if (NULL == interface_name)
	{
		printf("get_gateWay is err!!!\n");
		return -1;
	}
	
	FILE *fp;
	char buf[512];
	unsigned int gateway = 0;
	fp = fopen("/proc/net/route", "r");

	if (NULL == fp)
	{
		printf("open file /proc/net/route failed");
		return -1;
	}

	while (fgets((char *)buf, sizeof(buf), fp) != NULL)
	{
		unsigned long dest, gate;
		char buffer[20] = {0};
		dest = gate = 0;
		sscanf(buf, "%s%lx%lx", buffer, &dest, &gate);

		if (dest == 0 && gate != 0 && (0 == strcmp(buffer, (char *)interface_name)))
		{
			gateway = gate;
			struct in_addr *addr = (struct in_addr *)&gateway;
			printf("GateWay:%s\n", inet_ntoa(*addr));
			break;
		}
	}

	fclose(fp);
	fp = NULL;
	return gateway;
}


unsigned int get_ipAddr(char *interface_name)
{
	int s;
	unsigned int ip;

	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("ReachGetIPaddr:Socket:s:%d\n", s);
		return -1;
	}

	struct ifreq ifr;

	strcpy(ifr.ifr_name, interface_name);

	if (ioctl(s, SIOCGIFADDR, &ifr) < 0)
	{
		printf("ReachGetIPaddr:ioctl error");
		return -1;
	}

	struct sockaddr_in *ptr;

	ptr = (struct sockaddr_in *) &ifr.ifr_ifru.ifru_addr;

	printf("[ReachGetIPaddr] IP:%s\n", inet_ntoa(ptr->sin_addr));

	memcpy(&ip, &ptr->sin_addr, 4);

	close(s);

	return ip;
}


int get_ipAddrstring(char *interface_name, char *ipaddr)
{
	int s;

	if (NULL == ipaddr)
	{
		printf("ReachGetIPaddrstring:NULL == ipaddr");
		return -1;
	}

	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("ReachGetIPaddrstring:Socket s:%d\n", s);
		return -1;
	}

	struct ifreq ifr;

	strcpy(ifr.ifr_name, interface_name);

	if (ioctl(s, SIOCGIFADDR, &ifr) < 0)
	{
		printf("ReachGetIPaddrstring:ioctl");
		return -1;
	}

	struct sockaddr_in *ptr;

	ptr = (struct sockaddr_in *) &ifr.ifr_ifru.ifru_addr;

	snprintf(ipaddr, IPADDR_LEN, "%s", inet_ntoa(ptr->sin_addr));

	printf("ReachGetIPaddrstring:[GetIPaddr] IP:%s\n", ipaddr);

	close(s);

	return 0;
}


unsigned int get_netMask(char *interface_name)
{
	int s;
	unsigned int ip;

	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("ReachGetNetmask:Socket");
		return -1;
	}

	struct ifreq ifr;

	strcpy(ifr.ifr_name, interface_name);

	if (ioctl(s, SIOCGIFNETMASK, &ifr) < 0)
	{
		printf("ReachGetNetmask:ioctl");
		return -1;
	}

	struct sockaddr_in *ptr;

	ptr = (struct sockaddr_in *) &ifr.ifr_ifru.ifru_netmask;

	printf("ReachGetNetmask:Netmask:%s\n", inet_ntoa(ptr->sin_addr));

	memcpy(&ip, &ptr->sin_addr, 4);

	close(s);

	return ip;
}


unsigned int get_broadCast(char *interface_name)
{
	int s;
	unsigned int ip;

	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("GetBroadcast:Socket");
		return -1;
	}

	struct ifreq ifr;

	strcpy(ifr.ifr_name, interface_name);

	if (ioctl(s, SIOCGIFBRDADDR, &ifr) < 0)
	{
		printf("GetBroadcast:ioctl");
		return -1;
	}

	struct sockaddr_in *ptr;

	ptr = (struct sockaddr_in *) &ifr.ifr_ifru.ifru_broadaddr;

	printf("GetBroadcast:Broadcast:%s\n", inet_ntoa(ptr->sin_addr));

	memcpy(&ip, &ptr->sin_addr, 4);

	close(s);

	return ip;

}


#define DEFAULT_ETH0_DNS_CONFIG "/etc/resolv.conf"

#define SET_ETH_STATICIP(file,ip,gateway,netmask)\
	fprintf(file,"ifconfig eth0 %s netmask %s\n"\
	        "route add default gw %s\n"\
	        ,ip,netmask,gateway);

#define SET_ETH_DHCP(file) 	fprintf(file,"/sbin/dhcpcd eth0\n");

#define SET_ETH_DNS(file,dns) 	fprintf(file,"nameserver %s\n",dns);



int set_ip(char *filename, char *ip, char *netmask, char *gw)
{
	if (filename == NULL)
	{
		printf("file == NULL\n");
		return -1;
	}

	FILE *file = fopen(filename, "w");

	if (file != NULL)
	{
		SET_ETH_STATICIP(file, ip, gw, netmask);
		fclose(file);
		file = NULL;
	}

	chown(filename, 0, 0);
	return 0;
}

int set_dhcp(char *filename)
{
	FILE *file = fopen(filename, "w");

	if (file != NULL)
	{
		SET_ETH_DHCP(file);
		fclose(file);
		file = NULL;
	}

	chown(filename, 0, 0);
	return 0;
}

#define DNS_CFG	"/etc/resolv.conf"
int set_dns(char *dns)
{
	FILE *file = fopen(DNS_CFG, "w");

	if (file != NULL)
	{
		SET_ETH_DNS(file, dns);
		fclose(file);
		file = NULL;
	}

	chown(DNS_CFG, 0, 0);
	return 0;
}

