
#include "socket_public.h"
#define RHADDRMAXLEN	(16)


int setBlock_fd(int fd)
{

	if (fd < 0)
	{
		printf("<setBlock_fd param is error>  <fd : %d>\n", fd);
		return -1;
	}

	int opts;
	opts = fcntl(fd, F_GETFL);

	if (opts < 0)
	{
		printf("<setBlock_fd is error> <F_GETFL> <fd : %d> <error_s: %s> <error_d: %d>\n", fd, strerror(errno), errno);
		return -1;
	}

	opts = opts &~ O_NONBLOCK;

	if (fcntl(fd, F_SETFL, opts) < 0)
	{
		printf("<setBlock_fd is error> <F_SETFL> <fd : %d> <erros_s :%s> <error_d: %d>\n", fd, strerror(errno), errno);
		return -1;
	}

	return 0;

}

int setNonblock_fd(int fd)
{
	if (fd < 0)
	{
		printf("<setNonblock_fd is error>  <fd : %d>\n", fd);
		return -1;
	}

	int opts;
	opts = fcntl(fd, F_GETFL);

	if (opts < 0)
	{
		printf("<setNonblock_fd is error> <F_GETFL> <fd : %d> <ERROR_S : %s> <ERROR_D: %d>\n", fd, strerror(errno), errno);
		return -1;
	}

	opts = opts | O_NONBLOCK;

	if (fcntl(fd, F_SETFL, opts) < 0)
	{
		printf("<setNonblock_fd IS ERROR> <F_SETFL> <FD : %d> <ERROR_S :%s> <ERROR_D: %d>\n", fd, strerror(errno), errno);
		return -1;
	}

	return 0;
}

int get_sendBufSize(int fd)
{
	if (fd < 0)
	{
		printf("<get_sendBufSize IS ERROR>  <FD : %d>\n", fd);
		return -1;
	}

	int SndBufSize = 0;
	int OptLen = sizeof(int);

	if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &SndBufSize , (unsigned int *)&OptLen) < 0)
	{
		printf("<getsockopt IS ERROR> <SO_SNDBUF> <FD : %d> <ERROR_S :%s> <ERROR_D :%d>\n", fd, strerror(errno), errno);
		return -1;
	}

	return SndBufSize;
}

int set_sendBufSize(int fd , int SndBufSize)
{
	if (fd < 0 || SndBufSize < 0 || SndBufSize == 0)
	{
		printf("<set_sendBufSize IS ERROR>  <FD : %d>  <SndBufSize :%d>\n", fd, SndBufSize);
		return -1;
	}

	int OptLen = 1;

	if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &SndBufSize , sizeof(OptLen)) < 0)
	{
		printf("<RH_SetSndBufSizeFd IS ERROR> <SO_SNDBUF> <FD : %d> <ERROR_S :%s> <ERROR_D :%d> <SndBufSize :%d>\n",
		      fd, strerror(errno), errno, SndBufSize);
		return -1;
	}

	return 0;
}

int set_recvBufSize(int fd , int RcvBufSize)
{
	if (fd < 0 || RcvBufSize < 0 || RcvBufSize == 0)
	{
		printf("<RH_SetSndBufSizeFd IS ERROR>  <FD : %d>  <SndBufSize :%d>\n", fd, RcvBufSize);
		return -1;
	}

	int OptLen = 1;

	if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &RcvBufSize , sizeof(OptLen)) < 0)
	{
		printf("<RH_SetRcvBufSizeFd IS ERROR> <SO_RCVBUF> <FD : %d> <ERROR_S :%s> <ERROR_D :%d> <SndBufSize :%d>\n",
		      fd, strerror(errno), errno, RcvBufSize);
		return -1;
	}

	return 0;
}

int get_recvBufSize(int fd)
{
	if (fd < 0)
	{
		printf("<RH_GetSndBufSizeFd IS ERROR>  <FD : %d>\n", fd);
		return -1;
	}

	int RcvBufSize = 0;
	int OptLen = sizeof(int);

	if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &RcvBufSize , (unsigned int *)&OptLen) < 0)
	{
		printf("<RH_GetRcvBufSizeFd IS ERROR> <SO_RCVBUF> <FD : %d> <ERROR_S :%s> <ERROR_D :%d>\n", fd, strerror(errno), errno);
		return -1;
	}

	return 0;
}

int set_recvTimeout(int fd , int TimeoutSec, int TimeoutUsec)
{
	if (fd < 0 || TimeoutSec < 0 || TimeoutUsec < 0)
	{
		printf("<RH_SetRcvTimeoutFd IS ERROR>  <FD : %d>   <TimeoutSec :%d> <TimeoutUsec :%d>\n", fd, TimeoutSec, TimeoutUsec);
		return -1;
	}

	struct timeval Time;

	Time.tv_sec = TimeoutSec;

	Time.tv_usec = TimeoutUsec;

	if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&Time, sizeof(struct timeval)) < 0)
	{
		printf("<RH_SetRcvTimeoutFd IS ERROR> <SO_RCVTIMEO> <FD : %d> <ERROR_S :%s> <ERROR_D :%d> <TimeoutSec :%d> <TimeoutUsec :%d>\n",
		      fd, strerror(errno), errno , TimeoutSec, TimeoutUsec);
		return -1;
	}

	return 0;
}

int set_sendTimeout(int fd , int TimeoutSec, int TimeoutUsec)
{
	if (fd < 0 || TimeoutSec < 0 || TimeoutUsec < 0)
	{
		printf("<RH_SetSndBufSizeFd IS ERROR>  <FD : %d>   <TimeoutSec :%d> <TimeoutUsec :%d>\n", fd, TimeoutSec, TimeoutUsec);
		return -1;
	}

	struct timeval Time;

	Time.tv_sec = TimeoutSec;

	Time.tv_usec = TimeoutUsec;

	if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&Time, sizeof(struct timeval)) < 0)
	{
		printf("<RH_SetSndTimeoutFd IS ERROR> <SO_SNDTIMEO> <FD : %d> <ERROR_S :%s> <ERROR_D :%d> <TimeoutSec :%d> <TimeoutUsec :%d>\n",
		      fd, strerror(errno), errno , TimeoutSec, TimeoutUsec);
		return -1;
	}

	return 0;
}

int create_tcpBind(int LocalPort, char *LocalIp)
{
	int fd = -1;

	if (LocalPort < 0 || LocalPort == 0)
	{
		printf("<RH_CreateTcpFd IS ERROR>  <FD : %d> <LocalPort :%d > <LocalIp :%s>\n", fd, LocalPort, LocalIp);
		return -1;
	}

	struct sockaddr_in LocalAddr;

	bzero(&LocalAddr, sizeof(LocalAddr));

	LocalAddr.sin_family = AF_INET;
	LocalAddr.sin_port = htons(LocalPort);
	if (LocalIp)
	{
		LocalAddr.sin_addr.s_addr = inet_addr((const char *)LocalIp);
	}
	else
	{
		LocalAddr.sin_addr.s_addr	= htonl(INADDR_ANY);
	}

	fd = socket(AF_INET, SOCK_STREAM, 0);

	if (fd < 0)
	{
		printf("<RH_CreateTcpBindFd IS ERROR> <SO_REUSEADDR> <FD : %d> <ERROR_S> <ERROR_D %s,%d>\n", fd, strerror(errno), errno);
		return -1;
	}

	int opt = 1;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)\
	{
		printf("<RH_CreateTcpBindFd IS ERROR> <SO_REUSEADDR> <FD : %d> <ERROR_S> <ERROR_D %s,%d>\n", fd, strerror(errno), errno);
		return -1;
	}


	if (bind(fd, (struct sockaddr *) &LocalAddr, sizeof(LocalAddr)) < 0)
	{
		printf("<RH_CreateTcpBindFd IS ERROR> <BIND> <FD : %d> <ERROR_%s> <ERROR_%d> <LocalPort :%d> <LocalIp :%s>\n",
		      fd, strerror(errno), errno, LocalPort, LocalIp);
		return -1;
	}

	return fd;
}

int create_udpBind(int LocalPort, char *LocalIp)
{
	int fd = -1;

	if (LocalPort < 0 || LocalPort == 0 || LocalIp == NULL)
	{
		printf("<RH_CreateTcpFd IS ERROR>  <FD : %d> <LocalPort :%d > <LocalIp :%s>\n", fd, LocalPort, LocalIp);
		return -1;
	}

	struct sockaddr_in LocalAddr;

	bzero(&LocalAddr, sizeof(LocalAddr));

	LocalAddr.sin_family = AF_INET;

	LocalAddr.sin_addr.s_addr = inet_addr((const char *)LocalIp);

	LocalAddr.sin_port = htons(LocalPort);

	fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (fd < 0)
	{
		printf("<RH_CreateTcpBindFd IS ERROR> <SO_REUSEADDR> <FD : %d> <ERROR_S : %s> <ERROR_D: %d>\n", fd, strerror(errno), errno);
		return -1;
	}

	int opt = 1;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		printf("<RH_CreateTcpBindFd IS ERROR> <SO_REUSEADDR> <FD : %d> <ERROR_S: %s> <ERROR_D: %d>\n", fd, strerror(errno), errno);
		return -1;
	}


	if (bind(fd, (struct sockaddr *) &LocalAddr, sizeof(LocalAddr)) < 0)
	{
		printf("<RH_CreateTcpBindFd IS ERROR> <BIND> <FD : %d> <ERROR_S :%s> <ERROR_D: %d> <LocalPort :%d> <LocalIp :%s>\n",
		      fd, strerror(errno), errno, LocalPort, LocalIp);
		return -1;
	}

	return fd;
}

int socket_connet_block(int fd, int servPort, char *servIp) //,int Timeout)
{
	if (fd < 0 || servPort < 1 || NULL == servIp)
	{
		printf("<RH_ConnetNonblock IS ERROR>  <FD : %d> <ServPort: %d> <ServIp :%s>\n", fd, servPort, servIp);
		return -1;
	}

	struct sockaddr_in serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port	= htons(servPort);
	inet_aton((const char *)servIp, (struct in_addr *)&serv_addr.sin_addr);

	if (connect(fd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0)
	{
		printf("<RH_ConnetNonblock IS ERROR> <connect>  <ERROR_S :%s> <ERROR_D :%d> <FD : %d> <ServPort: %d> <ServIp :%s>\n",
		      strerror(errno), errno, fd, servPort, servIp);
		return -1;
	}

	return 0;
}

int socket_connet_nonBlock(int fd, int servPort, char *servIp , int timeout)
{

	if (fd < 0 || servPort < 1 || servIp == NULL || timeout > 0)
	{
		printf("<RH_ConnetNonblock IS ERROR>  <FD : %d> <ServPort: %d> <ServIp :%s>\n", fd, servPort, servIp);
		return -1;
	}

	struct timeval Time;

	fd_set set;

	int Len = sizeof(int);

	int RetSelect = -1;

	struct sockaddr_in serv_addr;

	bzero(&serv_addr, sizeof(serv_addr));

	int error = 0;

	serv_addr.sin_family = AF_INET;

	serv_addr.sin_port	= htons(servPort);

	inet_aton((const char *)servIp, (struct in_addr *)&serv_addr.sin_addr);

	if (connect(fd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0)
	{
		Time.tv_sec  = timeout;
		Time.tv_usec = 0;
		FD_ZERO(&set);
		FD_SET(fd, &set);
		RetSelect = select(fd + 1, NULL, &set, NULL, &Time);

		if (RetSelect > 0)
		{
			getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&Len);

			if (error == 0)
			{
				return 0;
			}
			else
			{
				printf("<RH_ConnetNonblock IS ERROR> <Connect error> <ERROR_S :%s> <ERROR_D :%d> <FD : %d> <ServPort: %d> <ServIp :%s> <Timeout :%d>\n",
				      strerror(errno), errno, fd, servPort, servIp, timeout);
				return -1;
			}
		}
		else if(0 == RetSelect)
		{
			printf("<RH_ConnetNonblock IS ERROR> <Connect timeout> <ERROR_S :%s> <ERROR_D :%d> <FD : %d> <ServPort: %d> <ServIp :%s> <Timeout :%d>\n",
			      strerror(errno), errno, fd, servPort, servIp, timeout);
			return -1;
		}
		else
		{
			printf("<RH_ConnetNonblock IS ERROR> <select> <ERROR_S :%s> <ERROR_D :%d> <FD : %d> <ServPort: %d> <ServIp :%s> <Timeout :%d>\n",
			      strerror(errno), errno, fd, servPort, servIp, timeout);
			return -1;
		}
	}

	return 0;
}

int socket_accept_block(int fd)
{
	if (fd < 0)
	{
		printf("<RH_GetConnectBlockFd IS ERROR>  <FD : %d>\n", fd);
		return -1;
	}

	int ClnFd = -1;
	int Len = sizeof(struct sockaddr_in);
	struct sockaddr_in ClnAddr;
	ClnFd = accept(fd, (void *)&ClnAddr, (socklen_t *)&Len);

	if (ClnFd < 0)
	{
		printf("<RH_GetConnectBlockFd IS ERROR> <accept> <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
		      strerror(errno), errno, fd);
		return -1;
	}
	else
	{
		printf("<RH_GetConnectBlockFd IS OK> <accept> <FD : %d> <ClientAddr :%s>\n", fd, inet_ntoa(ClnAddr.sin_addr));
		return ClnFd;
	}
}

int socket_accept_nonBlock(int fd , int timeout, char *acceptIp)
{
	if (fd < 0)
	{
		printf("<RH_GetConnectNonblockFd IS ERROR>  <FD : %d>\n", fd);
		return -1;
	}

	int ClnFd = -1;
	int RetSelect = -1;
	int Len = sizeof(struct sockaddr_in);

	fd_set set;

	struct timeval Time;
	Time.tv_sec  = timeout;
	Time.tv_usec = 0;
	FD_ZERO(&set);
	FD_SET(fd, &set);

	struct sockaddr_in ClnAddr;

	RetSelect = select(fd + 1, &set, NULL, NULL, &Time);

	if (RetSelect > 0)
	{
		ClnFd = accept(fd, (void *)&ClnAddr, (socklen_t *)&Len);

		if (ClnFd < 0)
		{
			printf("<RH_GetConnectNonblockFd IS ERROR> <listen> <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
			      strerror(errno), errno, fd);
			return -1;
		}
		else
		{
			printf("<RH_GetConnectNonblockFd IS OK> <accept> <FD : %d> <ClientAddr :%s>\n", fd, inet_ntoa(ClnAddr.sin_addr));
			memcpy(acceptIp, inet_ntoa(ClnAddr.sin_addr), RHADDRMAXLEN);
			return ClnFd;
		}

	}
	else if(0 == RetSelect)
	{
		printf("socket_accept_nonBlock time_out\n");
		return -1;
	}
	else
	{
		printf("<RH_GetConnectNonblockFd IS ERROR> <select> <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",  strerror(errno), errno, fd);
		return -1;
	}

}

int socket_tcp_sendNonBlock(int Fd, char *SndBuf, int *SndLen, int Timeout)
{

	if (Fd < 0 || NULL == SndBuf || SndLen == NULL)
	{
		printf("<RH_TcpSndNonblockFd IS ERROR> <FD : %d> <SndBuf :%p> <SndLen :%p> <Timeout :%d>\n",
		      Fd, SndBuf, SndLen, Timeout);
		return -1;
	}

	if (*SndLen < 0 || *SndLen == 0)
	{
		printf("<RH_TcpSndNonblockFd IS ERROR> <FD : %d> <SndBuf :%p> <SndLen :%d> <Timeout :%d>\n",
		      Fd, SndBuf, *SndLen, Timeout);
	}

	int RetSelect   = -1;
	int SndTotalLen = 0;
	int SndBytes	= 0;
	int SndTempLen  = *SndLen;
	*SndLen = 0;
	fd_set SndSet;
	struct timeval Time;
	FD_ZERO(&SndSet);
	FD_SET(Fd, &SndSet);

	while (SndTotalLen < SndTempLen)
	{
		if (Timeout < 0)
		{
			RetSelect = select(Fd + 1, NULL, &SndSet, NULL, NULL);
		}
		else
		{
			Time.tv_sec = Timeout / 1000;
			Time.tv_usec = 1000 * (Timeout % 1000);

			RetSelect = select(Fd + 1, NULL, &SndSet, NULL, &Time);
		}


		if(RetSelect < 0)
		{
			printf("<RH_TcpSndNonblockFd IS ERROR> <select> <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
			      strerror(errno), errno, Fd);
			return -1;
		}
		else if(RetSelect == 0)
		{
			printf("<RH_TcpSndNonblockFd IS ERROR> <Snd timeout> <ERROR_S :%s> <ERROR_D :%d> <FD : %d> <Timeout :%d>\n",
			      strerror(errno), errno, Fd, Timeout);
			return -1;
		}
		else
		{
			if (FD_ISSET(Fd, &SndSet))
			{
				SndBytes = send(Fd , SndBuf + SndTotalLen, SndTempLen - SndTotalLen, 0);

				if (SndBytes < 0)
				{
					printf("<RH_TcpSndNonblockFd IS ERROR> <Snd > <sendlen :%d> <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
					      SndTempLen - SndTotalLen, strerror(errno), errno, Fd);
					return -1;
				}
				else
				{
					SndTotalLen += SndBytes;
					*SndLen = SndTotalLen;
				}
			}
			else
			{
				printf("<RH_TcpSndNonblockFd IS ERROR> <FD_ISSET> <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
				      strerror(errno), errno, Fd);
				return -1;
			}
		}
	}

	return 0;
}

int socket_tcp_sendBlock(int Fd, char *SndBuf, int *SndLen)
{
	if (Fd < 0 || NULL == SndBuf || SndLen == NULL)
	{
		printf("<RH_TcpSndBlockFd IS ERROR> <FD : %d> <SndBuf :%p> <SndLen :%p>\n",
		      Fd, SndBuf, SndLen);
		return -1;
	}

	if (*SndLen < 0 || *SndLen == 0)
	{
		printf("<RH_TcpSndBlockFd IS ERROR> <FD : %d> <SndBuf :%p> <SndLen :%d>\n",
		      Fd, SndBuf, *SndLen);
	}

	int SndTotalLen = 0;
	int SndBytes	= 0;
	int SndTempLen  = *SndLen;
	*SndLen = 0;

	while (SndTotalLen < SndTempLen)
	{

		SndBytes = send(Fd , SndBuf + SndTotalLen, SndTempLen - SndTotalLen, 0);

		if (SndBytes < 0)
		{
			printf("<RH_TcpSndBlockFd IS ERROR> <Snd> <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
			      strerror(errno), errno, Fd);
			return -1;
		}
		else
		{
			SndTotalLen += SndBytes;
			*SndLen = SndTotalLen;
		}
	}

	return 0;

}

int socket_tcp_recvNonBlock(int Fd, char *RcvBuf, int *RcvLen, int Timeout)
{

	if(Fd < 0 || NULL == RcvBuf || RcvLen == NULL)
	{
		printf("\n\n%s,%s,%d,<RH_TcpRcvNonblockFd IS ERROR> <FD : %d> <RcvBuf :%p> <RcvLen :%p> <Timeout :%d>\n",__FILE__,__func__,__LINE__,
		      Fd, RcvBuf, RcvLen, Timeout);
		return -1;
	}

	if (*RcvLen <= 0)
	{
		printf("<RH_TcpRcvNonblockFd IS ERROR> <FD : %d> <RcvBuf :%p> <RcvLen :%d> <Timeout :%d>\n",
		      Fd, RcvBuf, *RcvLen, Timeout);
	}

	int RetSelect   = -1;
	int RcvTotalLen = 0;
	int RcvBytes	= 0;
	int RcvTempLen  = *RcvLen;
	*RcvLen = 0;
	fd_set RcvSet;
	struct timeval Time;
	FD_ZERO(&RcvSet);
	FD_SET(Fd, &RcvSet);

	*RcvLen = 0;
	while (RcvTotalLen < RcvTempLen)
	{

		if (Timeout < 0)
		{
			RetSelect = select(Fd + 1, &RcvSet, NULL, NULL, NULL);
		}
		else
		{
			Time.tv_sec = Timeout / 1000;
			Time.tv_usec = 1000 * (Timeout % 1000);

			RetSelect = select(Fd + 1, &RcvSet, NULL, NULL, &Time);
		}
		
		if(RetSelect < 0)
		{
			printf("<RH_TcpRcvNonblockFd IS ERROR> <select> <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
			      strerror(errno), errno, Fd);

			return -1;
		}
		else if(RetSelect == 0)
		{
			return -1;
		}
		else
		{
			if (FD_ISSET(Fd, &RcvSet))
			{
				RcvBytes = recv(Fd , RcvBuf + RcvTotalLen, RcvTempLen - RcvTotalLen, 0);

				if (RcvBytes < 0)
				{
					printf("<RH_TcpRcvNonblockFd IS ERROR> <Rcv Error> <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
					      strerror(errno), errno, Fd);
					return -1;
				}
				else if(RcvBytes == 0)
				{
					return -1;
				}
				else
				{
					RcvTotalLen += RcvBytes;
					*RcvLen = RcvTotalLen;
				}
			}
			else
			{
				printf("<RH_TcpRcvNonblockFd IS ERROR> <FD_ISSET> <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
				      strerror(errno), errno, Fd);
				return -1;
			}
		}
	}

	return 0;
}

int socket_tcp_recvBlock(int Fd, char *RcvBuf, int RcvLen , int *readlen)
{
	if (Fd < 0 || NULL == RcvBuf || readlen == NULL)
	{
		printf("<socket_tcp_recvBlock param IS ERROR> <FD : %d> <RcvBuf :%p> <RcvLen :%d>\n", Fd, RcvBuf, RcvLen);
		return -1;
	}

	if (RcvLen < 0 || RcvLen == 0)
	{
		printf("<socket_tcp_recvBlock recvlen is err> <FD : %d> <RcvBuf :%p> <RcvLen :%d>\n", Fd, RcvBuf, RcvLen);
		return -1;
	}

	int RcvTotalLen = 0;
	int RcvBytes	= 0;
	int timeout_cnt = 0;
	char *TempBuf = NULL;
	int RcvTempLen  = 0;
	*readlen = 0;

	while (RcvTotalLen < RcvLen)
	{
		TempBuf = RcvBuf + RcvTotalLen;
		RcvTempLen = RcvLen - RcvTotalLen;
		RcvBytes = recv(Fd , TempBuf, RcvTempLen, 0);
			
		if (RcvBytes < 0)
		{
			if (11 == errno)
			{
				timeout_cnt++;

				if (timeout_cnt < 5)
				{
					printf("recv failed timeout_cnt=%d errno=%d-<%s>\n", timeout_cnt, errno, strerror(errno));
					continue;
				}
			}
			printf("<RH_TcpRcvBlockFd IS ERROR> <RcvAdress : %p> <RcvTempLen :%d> <RcvLen :%d><readlen :%d><Rcv> <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
			     TempBuf,RcvTempLen, RcvLen, *readlen, strerror(errno), errno, Fd);
			return -1;
		}
		else if(RcvBytes == 0)
		{
			printf("ret =%d ---\n", RcvBytes);
			return -1;
		}
		else
		{
			RcvTotalLen += RcvBytes;
			*readlen = RcvTotalLen;
		}
	}

	return 0;
}

int socket_udp_sendNonblock(int Fd, char *SndIp, int SndPort, char *SndBuf, int *SndLen, int Timeout)
{

	if (Fd < 0 || NULL == SndBuf || NULL == SndLen || NULL == SndIp || SndPort == 0 || SndPort < 0)
	{
		printf("<RH_TcpSndNonblockFd IS ERROR> <FD : %d> <SndBuf :%p> <SndLen :%p> <Timeout :%d> <SndIp :%s> <SndPort :%d>\n",
		      Fd, SndBuf, SndLen, Timeout, SndIp, SndPort);
		return -1;
	}

	if (*SndLen < 0 || *SndLen == 0)
	{
		printf("<RH_TcpSndNonblockFd IS ERROR> <FD : %d> <SndBuf :%p> <SndLen :%d> <Timeout :%d>\n",
		      Fd, SndBuf, *SndLen, Timeout);
	}

	struct sockaddr_in SndAddr ;

	bzero(&SndAddr, sizeof(struct sockaddr_in));
	SndAddr.sin_family = AF_INET;
	SndAddr.sin_port = htons(SndPort);
	SndAddr.sin_addr.s_addr = inet_addr((const char *)SndIp);

	int RetSelect	= -1;
	int SndTotalLen = 0;
	int SndBytes	= 0;
	int SndTempLen	= *SndLen;

	*SndLen = 0;
	fd_set SndSet;
	struct timeval Time;

	FD_ZERO(&SndSet);
	FD_SET(Fd, &SndSet);

	while (SndTotalLen < SndTempLen)
	{
		if (Timeout < 0)
		{
			RetSelect = select(Fd + 1, NULL, &SndSet, NULL, NULL);
		}
		else
		{
			Time.tv_sec = Timeout / 1000;
			Time.tv_usec = 1000 * (Timeout % 1000);

			RetSelect = select(Fd + 1, NULL, &SndSet, NULL, &Time);
		}


		if (RetSelect < 0)
		{
			printf("<RH_TcpSndNonblockFd IS ERROR> <select> <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
			      strerror(errno), errno, Fd);
			return -1;
		}
		else if(RetSelect == 0)
		{
			printf("<RH_TcpSndNonblockFd IS ERROR> <Snd timeout> <ERROR_S :%s> <ERROR_D :%d> <FD : %d> <Timeout :%d>\n",
			      strerror(errno), errno, Fd, Timeout);
			return -1;
		}
		else
		{
			if (FD_ISSET(Fd, &SndSet))
			{
				SndBytes = sendto(Fd , SndBuf + SndTotalLen, SndTempLen - SndTotalLen, 0, (struct sockaddr *)&SndAddr, sizeof(SndAddr));

				if (SndBytes < 0)
				{
					printf("<RH_TcpSndNonblockFd IS ERROR> <Snd> <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
					      strerror(errno), errno, Fd);
					return -1;
				}
				else
				{
					SndTotalLen += SndBytes;
					*SndLen = SndTotalLen;
				}
			}
			else
			{
				printf("<RH_TcpSndNonblockFd IS ERROR> <FD_ISSET> <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
				      strerror(errno), errno, Fd);
				return -1;
			}
		}
	}

	return 0;

}

int socket_udp_sendBlock(int Fd, char *SndIp, int SndPort, char *SndBuf, int *SndLen)
{
	if (Fd < 0 || NULL == SndBuf || NULL == SndLen || NULL == SndIp || SndPort == 0 || SndPort < 0) {
		printf("<RH_TcpSndNonblockFd IS ERROR> <FD : %d> <SndBuf :%p> <SndLen :%p> <SndIp :%s> <SndPort :%d>\n",
		      Fd, SndBuf, SndLen, SndIp, SndPort);
		return -1;
	}

	if (*SndLen < 0 || *SndLen == 0)
	{
		printf("<RH_TcpSndNonblockFd IS ERROR> <FD : %d> <SndBuf :%p> <SndLen :%d> \n",
		      Fd, SndBuf, *SndLen);
	}

	struct sockaddr_in SndAddr ;

	bzero(&SndAddr, sizeof(struct sockaddr_in));
	SndAddr.sin_family = AF_INET;
	SndAddr.sin_port = htons(SndPort);
	SndAddr.sin_addr.s_addr = inet_addr((const char *)SndIp);

	int SndTotalLen = 0;
	int SndBytes	= 0;
	int SndTempLen  = *SndLen;

	*SndLen = 0;

	while (SndTotalLen < SndTempLen)
	{
		SndBytes = sendto(Fd , SndBuf + SndTotalLen, SndTempLen - SndTotalLen, 0, (struct sockaddr *)&SndAddr, sizeof(SndAddr));
		if (SndBytes < 0)
		{
			printf("<RH_TcpSndBlockFd IS ERROR> <Snd> <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
			      strerror(errno), errno, Fd);
			return -1;
		}
		else
		{
			SndTotalLen += SndBytes;
			*SndLen = SndTotalLen;
		}
	}

	return 0;
}

int socket_udp_recvNonBlock(int Fd, char *RcvIp, int *RcvPort, char *RcvBuf, int *RcvLen, int Timeout)
{

	if (Fd < 0 || NULL == RcvBuf || RcvLen == NULL)
	{
		printf("<RH_UdpRcvNonblockFd IS ERROR> <FD : %d> <RcvBuf :%p> <RcvLen :%p> <Timeout :%d>\n",
		      Fd, RcvBuf, RcvLen, Timeout);
		return -1;
	}

	if (*RcvLen < 0 || *RcvLen == 0)
	{
		printf("<RH_UdpRcvNonblockFd IS ERROR> <FD : %d> <RcvBuf :%p> <RcvLen :%d> <Timeout :%d>\n",
		      Fd, RcvBuf, *RcvLen, Timeout);
	}


	int RetSelect	= -1;
	int RcvTotalLen = 0;
	int RcvBytes	= 0;
	int RcvTempLen	= *RcvLen;
	*RcvLen = 0;
	fd_set RcvSet;
	struct timeval Time;
	FD_ZERO(&RcvSet);
	FD_SET(Fd, &RcvSet);

	struct sockaddr_in ClnAddr ;
	int Size = sizeof(ClnAddr);
	int SetClnAddr = 0;
	bzero(&ClnAddr, sizeof(struct sockaddr_in));

	char TempSndIp[RHADDRMAXLEN] = {0};

	while (RcvTotalLen < RcvTempLen)
	{
		if (Timeout < 0)
		{
			RetSelect = select(Fd + 1, NULL, &RcvSet, NULL, NULL);
		}
		else
		{
			Time.tv_sec = Timeout / 1000;
			Time.tv_usec = 1000 * (Timeout % 1000);
			RetSelect = select(Fd + 1, NULL, &RcvSet, NULL, &Time);
		}

		if (RetSelect < 0)
		{
			printf("<RH_UdpRcvNonblockFd IS ERROR> <select> <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
			      strerror(errno), errno, Fd);
			return -1;
		}
		else if(RetSelect == 0)
		{
			printf("<RH_UdpRcvNonblockFd IS ERROR> <Rcv timeout> <ERROR_S :%s> <ERROR_D :%d> <FD : %d> <Timeout :%d>\n",
			      strerror(errno), errno, Fd, Timeout);
			return -1;
		}
		else
		{
			if(FD_ISSET(Fd, &RcvSet))
			{
				RcvBytes = recvfrom(Fd , RcvBuf + RcvTotalLen, RcvTempLen - RcvTotalLen, 0, (struct sockaddr *)&ClnAddr, (socklen_t *)&Size);
				if (RcvBytes < 0)
				{
					printf("<RH_UdpRcvNonblockFd IS ERROR> <Rcv Error> <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
					      strerror(errno), errno, Fd);
					return -1;
				}
				else if(RcvBytes == 0)
				{
					printf("<RH_UdpRcvNonblockFd IS ERROR> <Rcv Close> <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
					      strerror(errno), errno, Fd);
					return -1;
				}
				else
				{
					if (0 == SetClnAddr)
					{
						if (RcvIp != NULL && RcvPort != NULL)
						{
							if (ClnAddr.sin_family == AF_INET)
							{
								if (inet_ntop(AF_INET, &ClnAddr.sin_addr, TempSndIp, sizeof(TempSndIp)) == NULL) {
									printf("<RH_UdpRcvNonblockFd IS ERROR> <inet_ntop SndIp> <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
									      strerror(errno), errno, Fd);
								}
								else
								{
									memcpy(RcvIp, TempSndIp, RHADDRMAXLEN);
								}

								if(ntohs(ClnAddr.sin_port) != 0)
								{
									*RcvPort = ntohs(ClnAddr.sin_port);
								}
							}
							else
							{
								printf("<RH_UdpRcvNonblockFd IS ERROR> <sa_family > <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
								      strerror(errno), errno, Fd);
							}

							SetClnAddr = 1;
						}
					}

					RcvTotalLen += RcvBytes;
					*RcvLen = RcvTotalLen;
				}
			} 
			else
			{
				printf("<RH_UdpRcvNonblockFd IS ERROR> <FD_ISSET> <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
				      strerror(errno), errno, Fd);
				return -1;
			}
		}
	}

	return 0;

}

int socket_udp_recvBlock(int Fd, char *RcvIp, int RcvPort, char *RcvBuf, int *RcvLen)
{

	if (Fd < 0 || NULL == RcvBuf || RcvLen == NULL)
	{
		printf("<RH_UdpRcvNonblockFd IS ERROR> <FD : %d> <RcvBuf :%p> <RcvLen :%p>\n",
		      Fd, RcvBuf, RcvLen);
		return -1;
	}

	if (*RcvLen < 0 || *RcvLen == 0)
	{
		printf("<RH_UdpRcvNonblockFd IS ERROR> <FD : %d> <RcvBuf :%p> <RcvLen :%d>\n",
		      Fd, RcvBuf, *RcvLen);
	}

	int RcvTotalLen = 0;
	int RcvBytes	= 0;
	int RcvTempLen	= *RcvLen;
	*RcvLen = 0;

	struct sockaddr_in ClnAddr ;
	int Size = sizeof(ClnAddr);
	int SetClnAddr = 0;
	bzero(&ClnAddr, sizeof(struct sockaddr_in));

	char TempSndIp[RHADDRMAXLEN] = {0};

	while (RcvTotalLen < RcvTempLen)
	{
		RcvBytes = recvfrom(Fd , RcvBuf + RcvTotalLen, RcvTempLen - RcvTotalLen, 0, (struct sockaddr *)&ClnAddr, (socklen_t *)&Size);
		if (RcvBytes < 0)
		{
			printf("<RH_UdpRcvNonblockFd IS ERROR> <Rcv Error> <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
			      strerror(errno), errno, Fd);
			return -1;
		}
		else if(RcvBytes == 0)
		{
			printf("<RH_UdpRcvNonblockFd IS ERROR> <Rcv Close> <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
			      strerror(errno), errno, Fd);
			return -1;
		}
		else
		{
			if (0 == SetClnAddr)
			{
				if (RcvIp != NULL && RcvPort > 0)
				{
					if (ClnAddr.sin_family == AF_INET)
					{
						if (inet_ntop(AF_INET, &ClnAddr.sin_addr, TempSndIp, sizeof(TempSndIp)) == NULL) {
							printf("<RH_UdpRcvNonblockFd IS ERROR> <inet_ntop SndIp> <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
							      strerror(errno), errno, Fd);
						}
						else
						{
							memcpy(RcvIp, TempSndIp, RHADDRMAXLEN);
						}

						if (ntohs(ClnAddr.sin_port) != 0)
						{
							RcvPort = ntohs(ClnAddr.sin_port);
						}
					}
					else
					{
						printf("<RH_UdpRcvNonblockFd IS ERROR> <sa_family > <ERROR_S :%s> <ERROR_D :%d> <FD : %d>\n",
						      strerror(errno), errno, Fd);
					}

					SetClnAddr = 1;
				}
			}

			RcvTotalLen += RcvBytes;
			*RcvLen = RcvTotalLen;
		}
	}

	return 0;
}

