#ifndef	__SOCKET_PUBLIC_H__
#define	__SOCKET_PUBLIC_H__
/*
author : ydl	   2016.01.09
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif
// *****************************************************
// function	: 设置socket为阻塞模式
// return   : Succes 0 / fail -1
// parameter: Fd > 0
//******************************************************
int setBlock_fd(int fd);

// *****************************************************
// function	: 设置socket为非阻塞模式
// return   : Succes 0 / fail -1
// parameter: Fd > 0
//******************************************************
int setNonblock_fd(int fd);

// *****************************************************
// function	: 获取Socket协议栈发送缓冲大小
// return   : Succes 缓冲大小 / fail -1
// parameter: Fd > 0
//******************************************************
int get_sendBufSize(int fd);

// *****************************************************
// function	: 设置Socket协议栈发送缓冲大小
// return   : Succes 0 / fail -1
// parameter: Fd > 0 / SndBufSize > 0
//******************************************************
int set_sendBufSize(int fd , int SndBufSize);

// *****************************************************
// function	: 获取Socket协议栈接收缓冲大小
// return   : Succes 缓冲大小 / fail -1
// parameter: Fd > 0 /
//******************************************************
int get_recvBufSize(int fd);

// *****************************************************
// function	: 设置Socket协议栈接收缓冲大小
// return   : Succes 0 / fail -1
// parameter: Fd > 0 / RcvBufSize > 0
//******************************************************
int set_recvBufSize(int Fd , int RcvBufSize);

// *****************************************************
// function	: 设置Socket接收超时时间
// return   : Succes 0 / fail -1
// parameter: Fd > 0 / TimeoutSec 大于等于 0 / TimeoutUsec 大于等于 0
// note		: 1.tv_sec 、tv_usec源接口  皆为long ；此函数不支持long型 精度
//			  2.此函数针对阻塞Socket
//******************************************************
int set_recvTimeout(int fd , int TimeoutSec,int TimeoutUsec);

// *****************************************************
// function	: 设置Socket发送超时时间
// return   : Succes 0 / fail -1
// parameter: Fd > 0 / TimeoutSec 大于等于 0 / TimeoutUsec 大于等于 0
// note		: 1.tv_sec 、tv_usec源接口  皆为long ；此函数不支持long型 精度
//			  2.此函数针对阻塞Socket
//******************************************************
int set_sendTimeout(int fd , int TimeoutSec,int TimeoutUsec);

// *****************************************************
// function	: 创建Tcp SOCKET Bind 模式
// return   : Succes fd / fail -1
// parameter: Lcreate_tcpBindocalPort > 0 /LocalIp 不为NULL
// note		: 当作为Client时 建议创建Socket不采用bind方式 ，再重连阶段 受服务器影响.

//            特殊需求再应用此函数(IP限定 端口限定 等....)  慎用!!!!!!!
//******************************************************
int create_tcpBind(int LocalPort,char *LocalIp); //特定需求 慎用!!!


// *****************************************************
// function	: 创建Tcp SOCKET Bind 模式
// return   : Succes fd / fail -1
// parameter: LocalPort > 0 /LocalIp 不为NULL
// note		:
// 特殊需求再应用此函数(IP限定 端口限定 等....)
//******************************************************
int create_udpBind(int LocalPort,char *LocalIp); //特定需求 还好!!!


// *****************************************************
// function	: 以阻塞模式SOCKET 连接服务器
// return   : Succes 0 / fail -1
// parameter: Fd > 0 / ServPort 大于等于 0 /ServIp 不为空 /Timeout 大于等于 0
// note		: 阻塞SOCKET
//******************************************************
int socket_connet_block(int fd,int ServPort,char *ServIp);


// *****************************************************
// function	: 以非阻塞模式SOCKET 连接服务器
// return   : Succes 0 / fail -1
// parameter: Fd > 0 / ServPort 大于等于 0 /ServIp 不为空 /Timeout 大于等于 0
// note		: 1.非阻塞SOCKET
// 			  2.Timeout 为0 为阻塞模式 当SOCKET异常返回
//            3.Timeout 大于0 为超时非阻塞
//******************************************************
int socket_connet_nonBlock(int fd,int servPort,char *servIp ,int timeout);


// *****************************************************
// function	: 以阻塞模式SOCKET 获取客户端连接
// return   : Succes Fd / fail -1
// parameter: Fd > 0
// note		: 1.阻塞SOCKET
//			  2.此函数为阻塞 <慎用!!!>
//******************************************************
int socket_accept_block(int fd);

// *****************************************************
// function	: 以非阻塞模式SOCKET 获取客户端连接
// return   : Succes fd / fail -1 /  超时 0
// parameter: Fd > 0 / Timeout 大于等于 0
// note		: 1.非阻塞SOCKET
//			  2.Timeout 为0时则此函数为阻塞，直到socket异常
//			  3.Timeout 为大于0时则等待时间未Timeout 秒
//******************************************************
int socket_accept_nonBlock(int fd ,int timeout,char *acceptIp);


// *****************************************************
// function	: 以非阻塞模式TCP SOCKET 发送数据
// return   : Succes 0  fail -1
// parameter: Fd > 0
//            SndBuf 不为NULL
//			  SndLen 不为NULL ；*SndLen等于要发送数据长度 ；函数返回后*SndLen为已发送长度
//  		  Timeout 小于0 此函数为无超时阻塞模式 ；
//			  Timeout 等于0 此函数为即时返回非阻塞模式；
//			  Timeout 大于0 此函数为有超时非阻塞模式；
//			  Timeout 单位毫秒  !!!!!!!
// note		: 1.非阻塞SOCKET
//			  2.发送大数据 KB单位 例如整帧数据
//			  3.Timeout 作为Select 设定超时  并不是此函数调用超时
//******************************************************
int socket_tcp_sendNonBlock(int Fd,char *SndBuf,int *SndLen,int Timeout);


// *****************************************************
// function	: 以阻塞模式TCP SOCKET 发送数据
// return   : Succes 0  fail -1
// parameter: Fd > 0
//            SndBuf 不为NULL
//			  SndLen 不为NULL ；*SndLen等于要发送数据长度 ；函数返回后*SndLen为已发送长度
// note		: 1.阻塞SOCKET
//			  2.此函数配合 RH_SetSndTimeoutFd 函数设定发送超时 更佳
//******************************************************
int socket_tcp_sendBlock(int Fd,char *SndBuf,int *SndLen);


// *****************************************************
// function	: 以非阻塞模式TCP SOCKET 接收数据
// return   : Succes 0  fail -1
// parameter: Fd > 0
//            RcvBuf 不为NULL
//			  RcvLen 不为NULL ；*RcvLen 等于要发送数据长度 ；函数返回后*RcvLen 为已发送长度
//  		  Timeout 小于0 此函数为无超时阻塞模式 ；
//			  Timeout 等于0 此函数为即时返回非阻塞模式；
//			  Timeout 大于0 此函数为有超时非阻塞模式；
//			  Timeout 单位毫秒  !!!!!!!
// note		: 1.非阻塞SOCKET
//			  2.接收大数据 KB单位 例如整帧数据
//			  3.Timeout 作为Select 设定超时  并不是此函数调用超时
//******************************************************
int socket_tcp_recvNonBlock(int Fd,char *RcvBuf,int *RcvLen,int Timeout);


// *****************************************************
// function	: 以阻塞模式TCP SOCKET 接收数据
// return   : Succes 0  fail -1
// parameter: Fd > 0
//            RcvBuf 不为NULL
//			  RcvLen 不为NULL ；*RcvLen等于要接收数据长度 ；函数返回后*RcvLen为已接收长度
// note		: 1.阻塞SOCKET
//			  2.此函数配合 RH_SetRcvTimeoutFd 函数设定发送超时 更佳
//******************************************************
int socket_tcp_recvBlock(int Fd,char *RcvBuf,int needlen,int *RcvLen);


// *****************************************************
// function	: 以非阻塞模式UDP SOCKET 发送数据
// return   : Succes 0  fail -1
// parameter: Fd > 0
//            SndBuf 不为NULL
//			  SndLen 不为NULL ；*SndLen等于要发送数据长度 ；函数返回后*SndLen为已发送长度
//  		  Timeout 小于0 此函数为无超时阻塞模式 ；
//			  Timeout 等于0 此函数为即时返回非阻塞模式；
//			  Timeout 大于0 此函数为有超时非阻塞模式；
//			  Timeout 单位毫秒  !!!!!!!
//			  SndIp   不为NULL 发送目的地IP
//			  SndPort 不为NULL 发送目的地端口
// note		: 1.非阻塞SOCKET
//			  2.发送大数据 KB单位 例如整帧数据
//			  3.Timeout 作为Select 设定超时  并不是此函数调用超时
//******************************************************
int socket_udp_sendNonBlock(int Fd,char *SndIp,int SndPort,char *SndBuf,int *SndLen,int Timeout);


// *****************************************************
// function	: 以阻塞模式UDP SOCKET 发送数据
// return   : Succes 0  fail -1
// parameter: Fd > 0
//            SndBuf 不为NULL
//			  SndLen 不为NULL ；*SndLen等于要发送数据长度 ；函数返回后*SndLen为已发送长度
//			  SndIp   不为NULL 发送目的地IP
//			  SndPort 不为NULL 发送目的地端口
// note		: 1.阻塞SOCKET
//			  2.此函数配合 RH_SetSndTimeoutFd 函数设定发送超时 更佳
//******************************************************
int socket_udp_sendBlock(int Fd,char *SndIp,int SndPort,char *SndBuf,int *SndLen);


// *****************************************************
// function	: 以非阻塞模式UDP SOCKET 接收数据
// return   : Succes 0  fail -1
// parameter: Fd > 0
//            RcvBuf 不为NULL
//			  RcvLen 不为NULL ；*RcvLen等于要接收数据长度 ；函数返回后*RcvLen为已接收长度
//  		  Timeout 小于0 此函数为无超时阻塞模式 ；
//			  Timeout 等于0 此函数为即时返回非阻塞模式；
//			  Timeout 大于0 此函数为有超时非阻塞模式；
//			  Timeout 单位毫秒  !!!!!!!
//			  RcvIp   不为NULL 则获取接收数据包的远端 ；否则依然
//			  RcvPort  不为NULL 则获取接收数据包的远端口 ；否则依然
// note		: 1.非阻塞SOCKET
//			  2.接收大数据 KB单位 例如整帧数据
//			  3.Timeout 作为Select 设定超时  并不是此函数调用超时
//******************************************************
int socket_udp_recvNonBlock(int Fd,char *SndIp,int *SndPort,char *RcvBuf,int *RcvLen,int Timeout);


// *****************************************************
// function	: 以阻塞模式UDP SOCKET 接收数据
// return   : Succes 0  fail -1
// parameter: Fd > 0
//            RcvBuf 不为NULL
//			  RcvLen 不为NULL ；*RcvLen等于要接收数据长度 ；函数返回后*RcvLen为已接收长度
//			  RcvIp   不为NULL 则获取接收数据包的缘地址 ；否则依然
//			  RcvPort  不为NULL 则获取接收数据包的缘端口 ；否则依然
// note		: 1.阻塞SOCKET
//			  2.此函数配合 RH_SetRcvTimeoutFd 函数设定发送超时 更佳
//******************************************************
int socket_udp_recvBlock(int Fd,char *SndIp,int SndPort,char *RcvBuf,int *RcvLen);

#ifdef __cplusplus
}
#endif

#endif


