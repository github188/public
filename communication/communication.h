#ifndef	__COMMUNICATION_PUBLIC_H__
#define	__COMMUNICATION_PUBLIC_H__
/*
author : ydl	   2016.01.14
notes:
*/
#include <pthread.h>


#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_CHECK_START_CODE '$'
#define DEFAULT_CHECK_END_CODE   '#'

#define HEARTBIT_CMD 0x99
#define DEFATULT_COMMUTICATION_TOTAL_LEN	(1024)

typedef enum {
	NO_INIT_STATUS = 0,
	START_STATUS =1,
	BEGIN_STOP_STATUS = 2,
	STOP_STATUS
}COMMUNTICATION_STATUS;

typedef enum{
	CLIENT_TO_SERVER = 0,
	REPEAT_CLIENT_TO_SERVER,
	SERVER_TO_CLIENT,
	REPEAT_SERVER_TO_CLIENT
}CMD_DIRECTION;

typedef enum{	
	RET_CMD_SUCCESS,
	RET_ERROR_STRUCT_LEN,
	RET_ERROR_INFO,
	RET_UNKNOWN_CMD
}CMD_RETURN_CODE;

typedef struct COMMUNTICATION_HEAD_S{
	unsigned char  check_start[4]; /*$$$$,��ʾ��*/
	unsigned int   identifier;     /*���ִ��Ǹ�client���Ǹ�server*/
	unsigned int   total_len;     /*������ص��ܳ���*/
	unsigned int   struct_len;    /*��ʵ�ṹ��ĳ��� ��= total_len*/
	unsigned int   cmd;     /*case */
	unsigned int   return_code;   /*����ֵ*/
	unsigned int   seq_num;       /*����ֵ*/
	CMD_DIRECTION direction;
	unsigned int   check_end[4]; /*####,��ʾ��*/
}Communtication_Head_t;

typedef  struct _COMMUTICATION_HANDLE * Commutication_Handle_t;

typedef int (*DealCmdFunc)(Communtication_Head_t *,void * ,Commutication_Handle_t );
typedef int (*getheartbitvalue)(char *,int *);
typedef int (*DealheartbitFunc)(char *);
typedef int (*ConnectServerInitFunc)();


typedef struct _COMMUTICATION_HANDLE{
	int connectNum;
	pthread_mutex_t lock;     //������
	int status; //��ǰ״̬ �����У�������
	unsigned int seq_num;  //�ۼ�ֵ����ʾ�ڼ������
	int client_socket;
	char ip[16];  /*server ,ip��ʾ����ip, client,ip��ʾĿ�ĵ�IP*/
	int  port;    /*server,port��ʾ���ؼ���port, client ,port��ʾĿ�ĵ�port*/
	DealCmdFunc DealCmdFuncPtr;
	getheartbitvalue GetHeartbitvaluePtr;
	DealheartbitFunc DealHeartbitFuncPtr;
	ConnectServerInitFunc ConnectServerInitPtr;
}Communtication_Handle_t;

Commutication_Handle_t communtication_create_clientHandle(char *dst_ip, unsigned short dst_port, DealCmdFunc func1, DealheartbitFunc func2, ConnectServerInitFunc func3);
Commutication_Handle_t communtication_create_serverHandle(char *local_ip, unsigned short local_port, DealCmdFunc func1, getheartbitvalue func2);
int communtication_send_serverMsg(Communtication_Head_t *head, char *date, int buflen, Communtication_Handle_t *handle);
int commutication_upload_heart(char *date, int len, Communtication_Handle_t *handle);
int communtication_set_handleStatus(Commutication_Handle_t handle, int status);
int communtication_get_handleStatus(Commutication_Handle_t handle);
int communtication_check_head(Communtication_Head_t *head);
void communtication_free_head(Commutication_Handle_t *handle);
int commutication_init_head(Communtication_Head_t *head, int identifier);
int communtication_send_clientMsg(Communtication_Head_t *head, char *date, int buflen, Communtication_Handle_t *handle);


#ifdef __cplusplus
}
#endif

#endif

