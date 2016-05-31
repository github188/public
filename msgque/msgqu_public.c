#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#include "msgqu_public.h"

key_t create_key(const char *path)
{
	if (NULL == path)
	{
		printf("the path is err!!!\n");
		return -1;
	}
	
	key_t key;
	key = ftok(path, 0);
	if (-1 == key)
	{
		printf("ftok error\n");	
	}
	
	return key;
}

int msg_get(key_t key)
{
	if (key < 0)
	{
		printf("msg_get is err!!!\n");
		return -1;
	}
	
	int msgid = -1;
	msgid = msgget(key, IPC_CREAT | IPC_EXCL | 00666);
	if (-1 == msgid)
	{
		printf("msgget error!!!\n");
		return -1;
	}
	
	return msgid;
}

void msg_ctl(int msgid, int cmd, struct msqid_ds *buf)
{
	if (msgctl(msgid, cmd, buf) == -1)
	{
		printf("msgctl error");	
	}	
}


void msg_send(int msgid, const Msg_Info_t *msg)
{
	if(msgsnd(msgid, msg, sizeof(msg->data), IPC_NOWAIT) == -1)
	{
		printf("msgsnd error");	
	}
}

void msg_recv(int msgid, Msg_info_t *msg, long type)
{
	int retval;
	int err;
	
	while((retval = msgrcv(msgid, msg, sizeof(msg->data), type, MSG_NOERROR)) == -1 && (err = errno) == EINTR);
	
	if (retval == -1)
	{
		printf("msgrcv error");
	}
}

