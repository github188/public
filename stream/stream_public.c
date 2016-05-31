#include "stream_public.h"

typedef struct _ClientData {
	int bUsed;
	int sSocket;
} ClientData_t;

typedef struct _ClinetPosIndex {
	int pos;
	int index;
} Server_PosIndex_t;

#define	TRUE  1
#define	FALSE 0
#define	INVALID_SOCKET (-1)

static pthread_mutex_t g_send_m[RH_MAX_STREAMS];



static ClientData_t	g_Server_cliInfo[RH_MAX_STREAMS][RH_MAX_CLIENTS];

static void set_client_info(int sindex, int cli, int val)
{
	pthread_mutex_lock(&g_send_m[sindex]);
	g_Server_cliInfo[sindex][cli].sSocket = val;
	pthread_mutex_unlock(&g_send_m[sindex]);
}

static void set_client_used(int sindex, int cli, int val)
{	
	pthread_mutex_lock(&g_send_m[sindex]);
	g_Server_cliInfo[sindex][cli].bUsed = val;
	pthread_mutex_unlock(&g_send_m[sindex]);
}

static int is_used(int sindex, int cli)
{	
	int flag = 0;
	pthread_mutex_lock(&g_send_m[sindex]);
	if (TRUE == g_Server_cliInfo[sindex][cli].bUsed)
	{
		flag = 1;
	}
	pthread_mutex_unlock(&g_send_m[sindex]);
	return flag;
}

static int is_sock(int sindex, int cli)
{
	int flag = 0;
	pthread_mutex_lock(&g_send_m[sindex]);
	if (INVALID_SOCKET != g_Server_cliInfo[sindex][cli].sSocket);
	{
		flag = 1;
	}
	pthread_mutex_unlock(&g_send_m[sindex]);
	return flag;
}

static int get_sock(int sindex, int cli)
{
	int fd = -1;
	pthread_mutex_lock(&g_send_m[sindex]);
	fd = g_Server_cliInfo[sindex][cli].sSocket;
	pthread_mutex_unlock(&g_send_m[sindex]);
	return fd;
}

static void set_sock(int sindex, int cli, int val)
{
	pthread_mutex_lock(&g_send_m[sindex]);
	g_Server_cliInfo[sindex][cli].sSocket=val;
	pthread_mutex_unlock(&g_send_m[sindex]);
}

static void privServer_init_pos(int sindex)
{
	int cli ;

	for (cli = 0; cli < RH_MAX_CLIENTS; cli++)
	{
		set_client_info(sindex, cli, INVALID_SOCKET);
		set_client_used(sindex, cli, FALSE);
	}
}

static int privServer_get_nullPos(int sindex)
{
	int cli ;

	for (cli = 0; cli < RH_MAX_CLIENTS; cli++)
	{
		if (!is_used(sindex, cli))
		{
			return cli;
		}
	}

	return -1;
}


static int priv_check_head(Stream_Head_t *head)
{
	if ((head == NULL) || (head->check_start[0] != DEFAULT_CHECK_START_CODE)
		|| (head->check_start[1] != DEFAULT_CHECK_START_CODE)
		|| (head->check_start[2] != DEFAULT_CHECK_START_CODE)
		|| (head->check_start[3] != DEFAULT_CHECK_START_CODE)
		|| (head->check_end[0] != DEFAULT_CHECK_END_CODE)
		|| (head->check_end[1] != DEFAULT_CHECK_END_CODE)
		|| (head->check_end[2] != DEFAULT_CHECK_END_CODE)
		|| (head->check_end[3] != DEFAULT_CHECK_END_CODE))
	{
		printf("the head is not head\n");
		printf("[%c][%c][%c][%c][%c][%c][%c][%c]\n", head->check_start[0], head->check_start[1], head->check_start[2], head->check_start[3],
		      head->check_end[0], head->check_end[1], head->check_end[2], head->check_end[3]);
		return -1;
	}

	return 0;
}

static void clear_lost_client(int sindex)
{
	int cli ;
	for (cli = 0; cli < RH_MAX_CLIENTS; cli++)
	{
		if (!is_used(sindex, cli) && is_sock(sindex, cli))
		{
			close(get_sock(sindex, cli));
			set_sock(sindex, cli, INVALID_SOCKET);
		}
	}
}


static int privClient_connect_server(char * ip, int port)
{
	int sockfd = -1;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		printf("privClient_connect_server is err!!!\n");
	}
	
	if (socket_connet_block(sockfd, port, ip) < 0)
	{
		close(sockfd);
		sockfd = -1;
	}

	return sockfd;
}


void stream_init_headType(Stream_Head_t *head)
{
	memset(head, 0, sizeof(Stream_Head_t));
	head->check_start[0] = head->check_start[1] = head->check_start[2] = head->check_start[3] = DEFAULT_CHECK_START_CODE;
	head->check_end[0] = head->check_end[1] = head->check_end[2] = head->check_end[3] = DEFAULT_CHECK_END_CODE;
}

static void *privCient_process_thread(void * arg) 
{
	Tcp_Stream_Recv_t *src = (Tcp_Stream_Recv_t *)arg;
	int sockfd = 0, ret = 0;
	char *data = NULL;
	int data_len = 0;
	void *pEmptyBufInfo = NULL;
	Stream_Head_t fh;
	const int fh_len = sizeof(Stream_Head_t);
	int readlen = fh_len;
	int msg_num = 0;

	int timeout = -1;

	timeout = src->timeout;

	int timeoutsec = 0 ;
	int timeoutusec = 0;

	if (timeout > 200)
	{
		timeoutsec = timeout / 1000;
		timeoutusec = 1000 * (timeout % 1000);
	}

	for (;;)
	{
		printf("connect server, ip = %s-port=%d,timeout=[%d]\n", src->ip, src->port, timeout);
		sockfd = privClient_connect_server((char *)(src->ip), src->port);
		if (sockfd < 0)
		{
			printf("sock privClient_connect_Srv failed!port =%d-<%s>-errno=%d-<%s>\n", src->port, src->ip, errno, strerror(errno));
			
			if (src->exception_msg)
			{
				src->exception_msg(-1, src->arg);
			}

			sleep(3);
			continue;
		}

		printf("connect success, sockfd = %d-port=[%s:%d]\n", sockfd, src->ip, src->port);
	
		if (timeout > 200)
		{
			printf("tiemoutsec.[%d.%d]\n", timeoutsec, timeoutusec);
			set_recvTimeout(sockfd, timeoutsec, timeoutusec);
		}

		for(;;)
		{
			msg_num = 0;
			memset(&fh, 0, sizeof(Stream_Head_t));
			ret = socket_tcp_recvBlock(sockfd, (char *)&fh, fh_len, &readlen);
			if (ret < 0 || fh_len != readlen || (priv_check_head(&fh) != 0))
			{
				printf("recv head port=%d -sockfd = %d,fh_len=%d,recv len=%d\n", src->port, sockfd, fh_len, readlen);
				msg_num = -1;
				sleep(3);
				break;
			}

			readlen = fh.nFrameLength;
			data = src->getEmptyBuf(src->arg, &pEmptyBufInfo, fh.nFrameLength);
			if (NULL == data)
			{
				printf("get [%s:%d]empty buf is NULL,len =%d\n", src->ip, src->port, fh.nFrameLength);
				msg_num = -2;
				sleep(3);
				break;
			}

			printf("the recv data is %d %d\n", sockfd, readlen);
			ret = socket_tcp_recvBlock(sockfd, data, readlen, &data_len);
			if (0 == ret)
			{
				ret = src->putStreaminfo(&fh, pEmptyBufInfo, src->arg, data);
				if (ret < 0)
				{
					printf("putStreaminfo failed ...\n");
					msg_num = -3;
					sleep(2);
					break;
				}
			}
			else
			{
				msg_num = -1;
				printf("recv_ error:[%s] len:<%d>recv len<%d>\n", strerror(errno), readlen, data_len);
				sleep(2);
				break;
			}
		}

		printf("--msg_num=%d-\n", msg_num);

		if (src->exception_msg)
		{
			src->exception_msg(msg_num, src->arg);
		}

		close(sockfd);
		sockfd = -1;

	}
	
	if (src)
	{
		free(src);
		src = NULL;
	}
	pthread_detach(pthread_self());
	pthread_exit(0);
	return NULL;
}


void *Stream_init_client(Tcp_Stream_Recv_t * inParm)
{
	if (NULL == inParm)
	{
		printf("Stream_init_client param is err!!\n");
		return NULL;
	}

	printf("the malloc is %d\n", sizeof(Tcp_Stream_Recv_t));
	Tcp_Stream_Recv_t * param = (Tcp_Stream_Recv_t *)malloc(sizeof(Tcp_Stream_Recv_t));
	if (NULL == param)
	{
		return NULL;
	}
	#if 1
	pthread_attr_t attr;
	struct sched_param param_pthread;
	memcpy(param, inParm, sizeof(Tcp_Stream_Recv_t));
	pthread_attr_init(&attr);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	param_pthread.sched_priority = 80;
	pthread_attr_setschedparam(&attr, &param_pthread);
	pthread_t    p;
	int record_val = -1;

	record_val = pthread_create(&p, &attr, privCient_process_thread, (void*)param);
	if (record_val)
	{
		printf("pthread_create is fail %d\n", inParm->port);
		pthread_attr_destroy(&attr);
		return NULL;
	}
	else
	{
		printf("pthread_create is suceess\n");
	}

	pthread_attr_destroy(&attr);
	#endif
	return param;
}


static int privServer_process_thread(void * arg)
{
	
	struct sockaddr_in	ClientAddr;
	int	sClientSocket = -1;
	int	ServSock = -1;
	pthread_t client_threadid[RH_MAX_CLIENTS] = {0};
	void *ret = 0;
	int clientsocket = 0;
	int nLen = 0;
	int ipos = 0;
	Server_PosIndex_t cli_info[RH_MAX_CLIENTS];
	Stream_Server_t *stream_net_info = (Stream_Server_t *)arg;
	privServer_init_pos(stream_net_info->sindex);
	for (ipos = 0; ipos < RH_MAX_CLIENTS; ipos++)
	{
		memset(&(cli_info[ipos]),0,sizeof(Server_PosIndex_t));
	}
SERVERSTARTRUN:
	ServSock = create_tcpBind(stream_net_info->port, stream_net_info->ip);
	printf("[%s:%d]\n", stream_net_info->ip, stream_net_info->port);
	printf("[%s:%d]--ServSock=%d\n",stream_net_info->ip, stream_net_info->port,ServSock);
	if (ServSock < 0)
	{
		printf("RH_CreateTcpBindFd error:%d,error msg: = %stream_net_info", errno, strerror(errno));
		if(errno != EAGAIN || errno != EINTR)
		{
			;
		}

		sleep(3);
		ServSock = INVALID_SOCKET;
		goto Exit_pthread;
	}

	if (listen(ServSock, 100) < 0)
	{
		printf("listen error:%d,error msg: = %s", errno, strerror(errno));
		close(ServSock);

		if (errno != EAGAIN || errno != EINTR)
		{
			;//	exit(0);
		}

		sleep(3);
		ServSock = INVALID_SOCKET;
		goto Exit_pthread;
	}

	if (setNonblock_fd(ServSock) < 0)
	{
		printf("RH_SetNonblockFd error:%d,error msg: = %s\n", errno, strerror(errno));
		close(ServSock);

		if (errno != EAGAIN || errno != EINTR)
		{
			//	exit(0);
		}

		sleep(3);
		ServSock = INVALID_SOCKET;
		goto Exit_pthread;
	}

	int FdSendBuf = 655350;

	nLen = sizeof(struct sockaddr_in);

	while(1)
	{
		memset(&ClientAddr, 0, sizeof(struct sockaddr_in));
		sClientSocket=0;
		nLen = sizeof(struct sockaddr_in);
		sClientSocket = accept(ServSock, (void *)&ClientAddr, (unsigned int *)&nLen);

		if (sClientSocket > 0)
		{
			printf("accept success!!!!ip=[%s]--sClientSocket=%d\n",inet_ntoa(ClientAddr.sin_addr), sClientSocket);

			int nPos = 0;
			clear_lost_client(stream_net_info->sindex);
			nPos = privServer_get_nullPos(stream_net_info->sindex);

			if (-1 == nPos)
			{
				close(sClientSocket);

				printf("MAXCLIENT ERR[%s:%d]\n", stream_net_info->ip, stream_net_info->port);
			}
			else
			{
				cli_info[nPos].index = stream_net_info->sindex;
				cli_info[nPos].pos = nPos;
				set_client_used(stream_net_info->sindex, nPos, TRUE);
				set_sock(stream_net_info->sindex, nPos, sClientSocket) ;

				if (set_sendBufSize(sClientSocket, FdSendBuf) != 0)
				{
					printf("RH_SetSndBufSizeFd Is Error!<fd :%d>\n", sClientSocket);
				}

				FdSendBuf = get_sendBufSize(sClientSocket);

				if (setNonblock_fd(sClientSocket) != 0)
				{
					printf("RH_SetSndBufSizeFd Is Error!<fd :%d>\n", sClientSocket);
				}
			}
		} 
		else
		{
			if (sClientSocket > 0)
			{
				close(sClientSocket);
				printf("close sClientSocket socket!!! %d\n", sClientSocket);
				sClientSocket = -1;
			}

			if(errno == ECONNABORTED || errno == EAGAIN)
				//软件原因中断
			{
				usleep(100000);
				continue;
			}

			if (ServSock > 0)
			{
				printf("close enclive socket!!! %d\n", ServSock);
				close(ServSock);
				ServSock = INVALID_SOCKET;
				sleep(1);
			}

			goto SERVERSTARTRUN;
		}

	}

Exit_pthread:
	printf("exit the drawtimethread \n");

	for (ipos = 0; ipos < RH_MAX_CLIENTS; ipos++)
	{
		if (client_threadid[ipos])
		{
			clientsocket = get_sock(stream_net_info->sindex, ipos);

			if (clientsocket != INVALID_SOCKET)
			{
				close(clientsocket);
				set_sock(stream_net_info->sindex, ipos, INVALID_SOCKET);
			}

			if (pthread_join(client_threadid[ipos], &ret) == 0)
			{
			
			}
		}
	}

	printf("close the encoder server socket \n");

	if (ServSock > 0)
	{
		printf("close gserv socket \n");
		close(ServSock);
	}

	if (stream_net_info)
	{
		free(stream_net_info);
		stream_net_info = NULL;
	}

	ServSock = INVALID_SOCKET;
	printf("End.\n");
	return 0;

}

int stream_init_server(Stream_Server_t *arg)
{
	int result;
	pthread_t	serverThid;
	Stream_Server_t *s = (Stream_Server_t *)malloc(sizeof(Stream_Server_t));

	if (NULL == s)
	{
		printf("r_malloc  StreamServer_t failed\n");
		return -1;
	}

	pthread_attr_t attr;
	struct sched_param param;
	pthread_attr_init(&attr);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	param.sched_priority = 80;
	pthread_attr_setschedparam(&attr, &param);
	memcpy(s, (Stream_Server_t *)arg, sizeof(Stream_Server_t));
	pthread_mutex_init(&g_send_m[s->sindex], NULL);

	result = pthread_create(&serverThid, &attr, (void *)privServer_process_thread, s);
	if (result < 0)
	{
		printf("create EncoderServerThread() failed\n");
		pthread_attr_destroy(&attr);
		return -1;
	}
	
	sleep(1);
	pthread_attr_destroy(&attr);
	return 0;
}

void stream_send_dataToClient(int sindex, int nLen, unsigned char * pData, Stream_Head_t* fh)
{

	int nRet = -1;
	int cnt = 0;
	int	sendsocket = -1;
	int fh_len = sizeof(Stream_Head_t);
	int data_len = nLen;
	int send_len = 0;

	for (cnt = 0; cnt < RH_MAX_CLIENTS; cnt++)
	{
		if (is_used(sindex, cnt))
		{
			sendsocket = get_sock(sindex, cnt);

			if (sendsocket > 0)
			{
				send_len =  fh_len;
				nRet = socket_tcp_sendNonBlock(sendsocket, (char *)fh, &send_len, 300);
				if (nRet < 0 || send_len != fh_len)
				{
					close(sendsocket);
					set_client_used(sindex, cnt, FALSE);
					set_sock(sindex, cnt, INVALID_SOCKET);
					printf("xxx Error: SOCK = %d count =%d errno=%d<%s>  ret = %d\n", sendsocket, cnt, errno,strerror(errno), nRet);
					continue;
				}

				send_len = data_len;
				nRet = socket_tcp_sendNonBlock(sendsocket, (char *)pData, &send_len, 300);
				if (nRet < 0 || send_len != data_len)
				{
					close(sendsocket); 
					set_client_used(sindex, cnt, FALSE);
					set_sock(sindex, cnt, INVALID_SOCKET);
					printf("Error: SOCK = %d count = %d  errno = %d  ret = %d\n", sendsocket, cnt, errno, nRet);
					continue;
				}

			}
		}

	}
}

