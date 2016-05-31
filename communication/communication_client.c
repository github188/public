#include "communication.h"
#include "stream_public.h"

static int communtication_process_clientHeartMsg(Communtication_Head_t *head, char *buf, int buflen, Communtication_Handle_t *handle)
{
	if (handle == NULL || handle->client_socket < 0)
	{
		return -1;
	}

	int socket = handle->client_socket;
	int sendlen = 0;
	Communtication_Head_t head_s ;
	memcpy(&head_s, head, sizeof(Communtication_Head_t));
	pthread_mutex_lock(&(handle->lock));
	sendlen = sizeof(head_s);
	socket_tcp_sendBlock(socket, (char *)&head_s, &sendlen);
	sendlen = head_s.total_len;
	socket_tcp_sendBlock(socket, buf, &sendlen);
	pthread_mutex_unlock(&(handle->lock));
	
	if (handle->DealHeartbitFuncPtr != NULL)
	{
		handle->DealHeartbitFuncPtr(buf);
	}

	return 0;
}

void * communtication_clientHeartThread(void *argv)
{
	char ip[16] = "127.0.0.1";
	unsigned short port = 3100;
	Commutication_Handle_t handle = (Commutication_Handle_t)argv;
	int ret = 0;
	char RecvBuf[1500] = {0};
	int RecvLen = 0;
	int client_socket = -1;
	int needlen = 0;
	Communtication_Head_t headbuf;

	if (handle == NULL)
	{
		printf("handle is NULL\n");
		pthread_detach(pthread_self());
		pthread_exit(0);
		return NULL;
	}

	snprintf(ip, sizeof(ip), "%s", handle->ip);
	port = handle->port;

	Communtication_Head_t *head = NULL;
	sleep(2);
REPEAT_CONNECT:
	printf("client will connet to server [%s:%d]\n", ip, port);

	handle->client_socket = -1;
	if (client_socket > 0)
	{
		close(client_socket);
		client_socket = -1;
	}

	client_socket = socket(AF_INET, SOCK_STREAM, 0);

	ret =  socket_connet_block(client_socket, port, ip);
	if (client_socket < 0)
	{
		printf("connet server fd is failed,the pot is [%d]\n", port);
		usleep(1000000);
		goto REPEAT_CONNECT;
	}

	printf("connet [%s:%d] is ok\n", ip, port);
	handle->client_socket = client_socket;
	communtication_set_handleStatus(handle, START_STATUS);

	if (handle->ConnectServerInitPtr != NULL)
	{
		handle->ConnectServerInitPtr();
	}

	set_recvTimeout(client_socket, 3, 0);

	while (communtication_get_handleStatus(handle) == START_STATUS)
	{
		needlen = sizeof(Communtication_Head_t);
		memset(&headbuf, 0, sizeof(headbuf));
	    ret = socket_tcp_recvBlock(client_socket, (char *)(&headbuf), needlen, &RecvLen);
		if (ret != 0 || RecvLen != needlen)
		{
			printf("tcp recv failed,the port is [%u].\n", port);
			usleep(500000);
			goto REPEAT_CONNECT;
		}

		//check head
		if (communtication_check_head(&headbuf) != 0)
		{
			printf("%s,communtication_check_head is failed,the port = [%u].\n",__func__, port);
			usleep(500000);
			goto REPEAT_CONNECT;
		}

		head = &headbuf;
		needlen = head->total_len;
		memset(&RecvBuf, 0, sizeof(RecvBuf));
		ret =  socket_tcp_recvBlock(client_socket, RecvBuf, needlen, &RecvLen);
		if (ret != 0 || RecvLen != needlen)
		{
			printf("Commutication tcp recv is failed,the cmd is [%u],the port = [%u]\n", head->cmd, port);
			usleep(500000);
			goto REPEAT_CONNECT;
		}

		if (head->cmd == HEARTBIT_CMD)
		{
			communtication_process_clientHeartMsg(head, RecvBuf, RecvLen, handle);
		}
		else
		{
			if (handle->DealCmdFuncPtr != NULL)
			{
				handle->DealCmdFuncPtr(head, RecvBuf, handle);
			}
		}
	}

	handle->client_socket = -1;

	if (client_socket > 0)
	{
		close(client_socket);
		client_socket = -1;
	}

	pthread_detach(pthread_self());
	pthread_exit(0);
	return NULL;
}


/*创建handle，同时产生后台的线程*/
Commutication_Handle_t communtication_create_clientHandle(char *dst_ip, unsigned short dst_port, DealCmdFunc func1, DealheartbitFunc func2, ConnectServerInitFunc func3)
{
	if (NULL == dst_ip || dst_port == 0)
	{
		printf("local_ip or local_port is error\n");
		return NULL;
	}

	printf("-------------dst_ip[%s]dst_port[%d]\n", dst_ip, dst_port);

	Commutication_Handle_t handle = NULL;
	char task_name[128] = {0};
	int ret = 0;
	handle = (Commutication_Handle_t)malloc(sizeof(Communtication_Handle_t));
	if (NULL == handle)
	{
		printf("Malloc handle is failed\n");
		return NULL;
	}

	pthread_mutex_t mutex;
	pthread_t tid;
	pthread_mutex_init(&mutex, NULL);

	//init handle
	memset(handle, 0, sizeof(Communtication_Handle_t));
	handle->lock = mutex;

	communtication_set_handleStatus(handle, NO_INIT_STATUS);
	handle->seq_num = 0;
	snprintf(handle->ip, sizeof(handle->ip), "%s", dst_ip);
	handle->port = dst_port;
	handle->DealCmdFuncPtr = func1;
	handle->DealHeartbitFuncPtr = func2;
	handle->ConnectServerInitPtr = func3;

	snprintf(task_name, sizeof(task_name), "clientcommutication_%d", dst_port);

	ret = pthread_create(&tid, NULL, communtication_clientHeartThread, (void *)(handle));
	if (ret != 0)
	{
		printf("crate communtication client thread failed\n");
		communtication_free_head(&handle);
		return NULL;
	}

	return handle;
}


