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
// function	: ����socketΪ����ģʽ
// return   : Succes 0 / fail -1
// parameter: Fd > 0
//******************************************************
int setBlock_fd(int fd);

// *****************************************************
// function	: ����socketΪ������ģʽ
// return   : Succes 0 / fail -1
// parameter: Fd > 0
//******************************************************
int setNonblock_fd(int fd);

// *****************************************************
// function	: ��ȡSocketЭ��ջ���ͻ����С
// return   : Succes �����С / fail -1
// parameter: Fd > 0
//******************************************************
int get_sendBufSize(int fd);

// *****************************************************
// function	: ����SocketЭ��ջ���ͻ����С
// return   : Succes 0 / fail -1
// parameter: Fd > 0 / SndBufSize > 0
//******************************************************
int set_sendBufSize(int fd , int SndBufSize);

// *****************************************************
// function	: ��ȡSocketЭ��ջ���ջ����С
// return   : Succes �����С / fail -1
// parameter: Fd > 0 /
//******************************************************
int get_recvBufSize(int fd);

// *****************************************************
// function	: ����SocketЭ��ջ���ջ����С
// return   : Succes 0 / fail -1
// parameter: Fd > 0 / RcvBufSize > 0
//******************************************************
int set_recvBufSize(int Fd , int RcvBufSize);

// *****************************************************
// function	: ����Socket���ճ�ʱʱ��
// return   : Succes 0 / fail -1
// parameter: Fd > 0 / TimeoutSec ���ڵ��� 0 / TimeoutUsec ���ڵ��� 0
// note		: 1.tv_sec ��tv_usecԴ�ӿ�  ��Ϊlong ���˺�����֧��long�� ����
//			  2.�˺����������Socket
//******************************************************
int set_recvTimeout(int fd , int TimeoutSec,int TimeoutUsec);

// *****************************************************
// function	: ����Socket���ͳ�ʱʱ��
// return   : Succes 0 / fail -1
// parameter: Fd > 0 / TimeoutSec ���ڵ��� 0 / TimeoutUsec ���ڵ��� 0
// note		: 1.tv_sec ��tv_usecԴ�ӿ�  ��Ϊlong ���˺�����֧��long�� ����
//			  2.�˺����������Socket
//******************************************************
int set_sendTimeout(int fd , int TimeoutSec,int TimeoutUsec);

// *****************************************************
// function	: ����Tcp SOCKET Bind ģʽ
// return   : Succes fd / fail -1
// parameter: Lcreate_tcpBindocalPort > 0 /LocalIp ��ΪNULL
// note		: ����ΪClientʱ ���鴴��Socket������bind��ʽ ���������׶� �ܷ�����Ӱ��.

//            ����������Ӧ�ô˺���(IP�޶� �˿��޶� ��....)  ����!!!!!!!
//******************************************************
int create_tcpBind(int LocalPort,char *LocalIp); //�ض����� ����!!!


// *****************************************************
// function	: ����Tcp SOCKET Bind ģʽ
// return   : Succes fd / fail -1
// parameter: LocalPort > 0 /LocalIp ��ΪNULL
// note		:
// ����������Ӧ�ô˺���(IP�޶� �˿��޶� ��....)
//******************************************************
int create_udpBind(int LocalPort,char *LocalIp); //�ض����� ����!!!


// *****************************************************
// function	: ������ģʽSOCKET ���ӷ�����
// return   : Succes 0 / fail -1
// parameter: Fd > 0 / ServPort ���ڵ��� 0 /ServIp ��Ϊ�� /Timeout ���ڵ��� 0
// note		: ����SOCKET
//******************************************************
int socket_connet_block(int fd,int ServPort,char *ServIp);


// *****************************************************
// function	: �Է�����ģʽSOCKET ���ӷ�����
// return   : Succes 0 / fail -1
// parameter: Fd > 0 / ServPort ���ڵ��� 0 /ServIp ��Ϊ�� /Timeout ���ڵ��� 0
// note		: 1.������SOCKET
// 			  2.Timeout Ϊ0 Ϊ����ģʽ ��SOCKET�쳣����
//            3.Timeout ����0 Ϊ��ʱ������
//******************************************************
int socket_connet_nonBlock(int fd,int servPort,char *servIp ,int timeout);


// *****************************************************
// function	: ������ģʽSOCKET ��ȡ�ͻ�������
// return   : Succes Fd / fail -1
// parameter: Fd > 0
// note		: 1.����SOCKET
//			  2.�˺���Ϊ���� <����!!!>
//******************************************************
int socket_accept_block(int fd);

// *****************************************************
// function	: �Է�����ģʽSOCKET ��ȡ�ͻ�������
// return   : Succes fd / fail -1 /  ��ʱ 0
// parameter: Fd > 0 / Timeout ���ڵ��� 0
// note		: 1.������SOCKET
//			  2.Timeout Ϊ0ʱ��˺���Ϊ������ֱ��socket�쳣
//			  3.Timeout Ϊ����0ʱ��ȴ�ʱ��δTimeout ��
//******************************************************
int socket_accept_nonBlock(int fd ,int timeout,char *acceptIp);


// *****************************************************
// function	: �Է�����ģʽTCP SOCKET ��������
// return   : Succes 0  fail -1
// parameter: Fd > 0
//            SndBuf ��ΪNULL
//			  SndLen ��ΪNULL ��*SndLen����Ҫ�������ݳ��� ���������غ�*SndLenΪ�ѷ��ͳ���
//  		  Timeout С��0 �˺���Ϊ�޳�ʱ����ģʽ ��
//			  Timeout ����0 �˺���Ϊ��ʱ���ط�����ģʽ��
//			  Timeout ����0 �˺���Ϊ�г�ʱ������ģʽ��
//			  Timeout ��λ����  !!!!!!!
// note		: 1.������SOCKET
//			  2.���ʹ����� KB��λ ������֡����
//			  3.Timeout ��ΪSelect �趨��ʱ  �����Ǵ˺������ó�ʱ
//******************************************************
int socket_tcp_sendNonBlock(int Fd,char *SndBuf,int *SndLen,int Timeout);


// *****************************************************
// function	: ������ģʽTCP SOCKET ��������
// return   : Succes 0  fail -1
// parameter: Fd > 0
//            SndBuf ��ΪNULL
//			  SndLen ��ΪNULL ��*SndLen����Ҫ�������ݳ��� ���������غ�*SndLenΪ�ѷ��ͳ���
// note		: 1.����SOCKET
//			  2.�˺������ RH_SetSndTimeoutFd �����趨���ͳ�ʱ ����
//******************************************************
int socket_tcp_sendBlock(int Fd,char *SndBuf,int *SndLen);


// *****************************************************
// function	: �Է�����ģʽTCP SOCKET ��������
// return   : Succes 0  fail -1
// parameter: Fd > 0
//            RcvBuf ��ΪNULL
//			  RcvLen ��ΪNULL ��*RcvLen ����Ҫ�������ݳ��� ���������غ�*RcvLen Ϊ�ѷ��ͳ���
//  		  Timeout С��0 �˺���Ϊ�޳�ʱ����ģʽ ��
//			  Timeout ����0 �˺���Ϊ��ʱ���ط�����ģʽ��
//			  Timeout ����0 �˺���Ϊ�г�ʱ������ģʽ��
//			  Timeout ��λ����  !!!!!!!
// note		: 1.������SOCKET
//			  2.���մ����� KB��λ ������֡����
//			  3.Timeout ��ΪSelect �趨��ʱ  �����Ǵ˺������ó�ʱ
//******************************************************
int socket_tcp_recvNonBlock(int Fd,char *RcvBuf,int *RcvLen,int Timeout);


// *****************************************************
// function	: ������ģʽTCP SOCKET ��������
// return   : Succes 0  fail -1
// parameter: Fd > 0
//            RcvBuf ��ΪNULL
//			  RcvLen ��ΪNULL ��*RcvLen����Ҫ�������ݳ��� ���������غ�*RcvLenΪ�ѽ��ճ���
// note		: 1.����SOCKET
//			  2.�˺������ RH_SetRcvTimeoutFd �����趨���ͳ�ʱ ����
//******************************************************
int socket_tcp_recvBlock(int Fd,char *RcvBuf,int needlen,int *RcvLen);


// *****************************************************
// function	: �Է�����ģʽUDP SOCKET ��������
// return   : Succes 0  fail -1
// parameter: Fd > 0
//            SndBuf ��ΪNULL
//			  SndLen ��ΪNULL ��*SndLen����Ҫ�������ݳ��� ���������غ�*SndLenΪ�ѷ��ͳ���
//  		  Timeout С��0 �˺���Ϊ�޳�ʱ����ģʽ ��
//			  Timeout ����0 �˺���Ϊ��ʱ���ط�����ģʽ��
//			  Timeout ����0 �˺���Ϊ�г�ʱ������ģʽ��
//			  Timeout ��λ����  !!!!!!!
//			  SndIp   ��ΪNULL ����Ŀ�ĵ�IP
//			  SndPort ��ΪNULL ����Ŀ�ĵض˿�
// note		: 1.������SOCKET
//			  2.���ʹ����� KB��λ ������֡����
//			  3.Timeout ��ΪSelect �趨��ʱ  �����Ǵ˺������ó�ʱ
//******************************************************
int socket_udp_sendNonBlock(int Fd,char *SndIp,int SndPort,char *SndBuf,int *SndLen,int Timeout);


// *****************************************************
// function	: ������ģʽUDP SOCKET ��������
// return   : Succes 0  fail -1
// parameter: Fd > 0
//            SndBuf ��ΪNULL
//			  SndLen ��ΪNULL ��*SndLen����Ҫ�������ݳ��� ���������غ�*SndLenΪ�ѷ��ͳ���
//			  SndIp   ��ΪNULL ����Ŀ�ĵ�IP
//			  SndPort ��ΪNULL ����Ŀ�ĵض˿�
// note		: 1.����SOCKET
//			  2.�˺������ RH_SetSndTimeoutFd �����趨���ͳ�ʱ ����
//******************************************************
int socket_udp_sendBlock(int Fd,char *SndIp,int SndPort,char *SndBuf,int *SndLen);


// *****************************************************
// function	: �Է�����ģʽUDP SOCKET ��������
// return   : Succes 0  fail -1
// parameter: Fd > 0
//            RcvBuf ��ΪNULL
//			  RcvLen ��ΪNULL ��*RcvLen����Ҫ�������ݳ��� ���������غ�*RcvLenΪ�ѽ��ճ���
//  		  Timeout С��0 �˺���Ϊ�޳�ʱ����ģʽ ��
//			  Timeout ����0 �˺���Ϊ��ʱ���ط�����ģʽ��
//			  Timeout ����0 �˺���Ϊ�г�ʱ������ģʽ��
//			  Timeout ��λ����  !!!!!!!
//			  RcvIp   ��ΪNULL ���ȡ�������ݰ���Զ�� ��������Ȼ
//			  RcvPort  ��ΪNULL ���ȡ�������ݰ���Զ�˿� ��������Ȼ
// note		: 1.������SOCKET
//			  2.���մ����� KB��λ ������֡����
//			  3.Timeout ��ΪSelect �趨��ʱ  �����Ǵ˺������ó�ʱ
//******************************************************
int socket_udp_recvNonBlock(int Fd,char *SndIp,int *SndPort,char *RcvBuf,int *RcvLen,int Timeout);


// *****************************************************
// function	: ������ģʽUDP SOCKET ��������
// return   : Succes 0  fail -1
// parameter: Fd > 0
//            RcvBuf ��ΪNULL
//			  RcvLen ��ΪNULL ��*RcvLen����Ҫ�������ݳ��� ���������غ�*RcvLenΪ�ѽ��ճ���
//			  RcvIp   ��ΪNULL ���ȡ�������ݰ���Ե��ַ ��������Ȼ
//			  RcvPort  ��ΪNULL ���ȡ�������ݰ���Ե�˿� ��������Ȼ
// note		: 1.����SOCKET
//			  2.�˺������ RH_SetRcvTimeoutFd �����趨���ͳ�ʱ ����
//******************************************************
int socket_udp_recvBlock(int Fd,char *SndIp,int SndPort,char *RcvBuf,int *RcvLen);

#ifdef __cplusplus
}
#endif

#endif


