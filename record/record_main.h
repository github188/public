#ifndef __RECORD_MAIN_H
#define __RECORD_MAIN_H
#include "list_use_lock.h"
#include "share_stream.h"
#include "record_communtication.h"
#include "edukit_port.h"
#include "EasyAACEncoderAPI.h"

#define RECORD_PATH			"/opt/course"
#define OLDFILE_TMP			"/tmp/oldfile_tmp.xml"
#define FILE_LEN 540
#define SURPLUS  95
#define SURPLUS_ABANDA 90
#define RECORD_NET_SOURCE	5
#define RECORD_ENC_SOURCE	3

#define RECORD_STREAM_MAX	(8)
#define RECORD_STREAM_NAME_MAX 8
#define RECORD_TYPE "mp4"
enum
{
	ENC_SWMS = 0,
	ENC_VGA  = 1,
	AUDIO    = 2,
	NET_PTEA  = 3,
	NET_PSTU  = 4,
	NET_BLB = 5,
	NET_TEA = 6,
	NET_STU = 7,
    RECORD_MAX = 8,
};

enum
{
	NET_0 = 0,
	NET_1 = 1,
	NET_2 = 2,
	NET_3 = 3,
	NET_4 = 4,
	NET_5 = 5,
	NET_6 = 6,
	NET_7 = 7,
};

enum
{
	CLOSE = 0,
	OPEN = 1,
};



typedef struct
{	
	char ip[RECORD_NET_SOURCE][IP_LEN];
	int  nPort[RECORD_NET_SOURCE];
	char streamName[RECORD_NET_SOURCE][RECORD_STREAM_NAME_MAX];
}Record_NetSource_Info_t;

typedef struct
{
	Record_NetSource_Info_t netSource_info;
	pthread_mutex_t net_mutex;
}Record_Net_t;

typedef struct Record_Info {
	int32_t Iframe;//
	int32_t channel;
	int32_t data_type;//Video or Audio
	int32_t height;
	uint32_t bitrate;
	int32_t width;
	uint32_t framerate;
	int32_t sample_rate;
	int32_t audio_BitRate;
	uint32_t time_tick;
	uint32_t decode_tick;
	int32_t moive_res;//record mode
	int32_t data_len;
	int32_t blue_flag;
	unsigned int packet_num;
	unsigned char *data;
} Media_Info_t;


typedef struct Record_time{
	int time;
} Record_Time_t;

typedef struct Record_Handle {
	Record_Status_t record_status;
	Record_Signal_t record_signal;
	Record_Time_t record_time;
	pthread_mutex_t signal_mutex;
	pthread_mutex_t time_mutex;
	pthread_mutex_t mutex;
} Record_Handle_t;


typedef struct Recv_NetInfo {
	unsigned int index;
	unsigned char ip[IP_ADDR_LEN];
	unsigned short port;
} Recv_NetInfo_t;

typedef struct Recv_ClientInfo {
	List_LockHandle_t* pListLock_Handle;
	unsigned int index;
	unsigned char ip[IP_ADDR_LEN];
	unsigned short port;
} Recv_ClientInfo_t;

typedef struct pthread_process{
	Recv_ClientInfo_t client_param;
	char filename[FILE_LEN];
}Pthread_Param_t;

typedef struct RECORD_CONCTROL{
	pthread_mutex_t mutex;
	int record_flag;
}Record_Conctrol_t;

typedef struct RECORD_SWITCH{
	pthread_mutex_t mutex;
	int	switch_flag[RECORD_NET_SOURCE];//用于判断是不是要开启录制
}Record_Switch_t;

typedef struct RECORD_STREAM{
	pthread_mutex_t mutex;
	int	stream_flag[RECORD_NET_SOURCE];//用于判断是不是要开启录制
}Record_Stream_t;

typedef struct RECORD_COURSE_INFO{
	pthread_mutex_t mutex;
	int total_size;
	char record_time[128];
	char start_time[128];
	char file_name[FILE_LEN];
	char main_teacher[132];
	char class_room[128];
	char course_subject[260];
	char notes[128];
}Record_CourseInfo_t;
#if 1
typedef struct RECORD_Stream_Handle
{
	int channel;
	int port;
	char iP[16];
	List_LockHandle_t* pListLock_Handle;
	int nRun_Status;
	int first_frame;
	char streamName[64];
	EasyAACEncoder_Handle handle;
	FILE *foutp;
	unsigned char *acc_data;
	unsigned char *g711_data;
}Record_RecvNet_Handle_t;
#endif

int record_process_stopMsg();
int record_process_pauseMsg();
void *record_process_startMsg(void *arg);


#endif

