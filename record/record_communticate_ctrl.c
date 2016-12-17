#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <netinet/ether.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <strings.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include "nslog.h"
#include "communtication.h"

#include "Public_define.h"
#include "edukit_common.h"
#include "edukit_port.h"
#include "record_communtication.h"
#include "share_socket.h"
#include "edukit_network.h"
#include "record_main.h"
#include "live_communtication.h"

static Commutication_Handle_t  s_record_srvHandle = NULL;
extern Record_Net_t g_net_info;
extern Record_CourseInfo_t g_record_courseInfo;
extern int record_get_status(Record_Status_t *status);
extern int record_set_signal(Record_Signal_t status);
extern int priv_get_recordTime(int * time);
extern int fill_global_recordInfo(Record_CourseInfo_t course_info);
extern int  record_get_switch(int *switch_flag);
extern int  record_set_switch(int *switch_flag);


//extern int	g_switch_flag[RECORD_STREAM_MAX];
/*获取心跳的结构体数据*/
static int privRecord_get_heartValue(char *buff, int *len)
{
	if (NULL == buff || NULL == len || *len < sizeof(Record_heartbit_s))
	{
		nslog(NS_ERROR, "buf=%p  *len = %p ,parameter is invaild\n", buff, len);
		return -1;
	}

	Record_heartbit_s r_heartbit;
	memset(&r_heartbit, 0, sizeof(Record_heartbit_s));
	/*加入获取心跳参数的代码*/
	record_get_status(&(r_heartbit.record_status));
	priv_get_recordTime(&(r_heartbit.record_time));
	
	r_heartbit.init = 1;
	*len = sizeof(Record_heartbit_s);
	memcpy(buff, &r_heartbit, sizeof(Record_heartbit_s));
	nslog(NS_INFO, "\033[32m""->Heart<----Status=%d---time=%d\n""\033[0m", r_heartbit.record_status, r_heartbit.record_time);
	return 0;
}

static int priv_check_invalidChar(char *s1, char *s2, char *s3)
{
	int i = 0;
	char str[FILE_LEN] = {0};
	sprintf(str, "%s%s%s", s1, s2, s3);
	while (str[i])                                                                                                           
	{                                                                                                                   
		if(((ispunct(str[i]) && (str[i] <127) && (str[i] != '_' )) || ((str[i]) == ' ')))                                                       
		{                                                                                                                    
			return -1;                                                             
		}
		i++;                                                                                                                 
	}

	return 0;
}

static int privRecord_get_startInfo(Record_start_info_s *info)
{	
	if ((!info) || ((strlen(info->MainTeacher) + strlen(info->MainTeacher) + strlen(info->MainTeacher)) > 128))
	{
		return RET_PARAMER_ERR;
	}
	
	int len = 0;
	len = strlen(info->MainTeacher) + strlen(info->MainTeacher) + strlen(info->MainTeacher);
	nslog(NS_ERROR, "((strlen(info->MainTeacher) + strlen(info->MainTeacher) + strlen(info->MainTeacher))=%d\n", len);
	if (0 != len)
	{
		if (0 != priv_check_invalidChar(info->MainTeacher, info->CName, info->RoomName))
		{
			nslog(NS_ERROR, "privRecord_get_startInfo invalid char!!!!!\n");
			return RET_PARAMER_ERR;
		}
	}
	
	Record_Status_t record_status = -1;
	record_get_status(&record_status);
	if (RECORD_RESUME == record_status)
	{
		return RET_CONFLICT_CMD;//正在录像
	}

	if (RECORD_CLOSE == record_status)
	{
		Record_CourseInfo_t course_info;
		memset(&course_info, 0, sizeof(Record_CourseInfo_t));
		memcpy(course_info.notes, info->Notes, sizeof(course_info.notes));
		memcpy(course_info.main_teacher, info->MainTeacher, sizeof(course_info.main_teacher));
		memcpy(course_info.course_subject, info->CName,sizeof(course_info.course_subject));
		memcpy(course_info.class_room, info->RoomName, sizeof(course_info.class_room));
		fill_global_recordInfo(course_info);
		nslog(NS_INFO, "%s %s %s \n", info->MainTeacher, info->CName, info->RoomName);
	}
	
	record_set_signal(SIGNAL_START);
	return RET_SUCCESS;
}


static int privRecord_get_stopInfo(Record_stop_info_s * info)
{
	if (NULL == info)
	{
		nslog(NS_ERROR, "it is error params privRecord_get_stopInfo\n");
	}
	Record_Status_t record_status = -1;
	record_get_status(&record_status);
	if (RECORD_CLOSE == record_status)
	{
		return RET_CONFLICT_CMD;//关闭状态
	}

	if (1 == info->enable_endPic)//表示需要加片尾
	{
		usleep(500000);
	}
	record_set_signal(SIGNAL_CLOSE);
	
	return RET_SUCCESS;
}

//#define NET_SOURCE_NUMB 3
static int privRecord_set_netInfo(Live_ServerStream_Info_t * info)
{
	if (!info)
	{
		return RET_PARAMER_ERR;
	}
	
	int i = 0;
	pthread_mutex_lock(&(g_net_info.net_mutex));
	memset(&(g_net_info.netSource_info), 0, sizeof(Record_NetSource_Info_t));
	memcpy(g_net_info.netSource_info.ip[0], info->ip[3], IP_LEN);//教师全景
	memcpy(g_net_info.netSource_info.ip[1], info->ip[4], IP_LEN);//学生全景
	memcpy(g_net_info.netSource_info.ip[2], info->ip[6], IP_LEN);//板书
	memcpy(g_net_info.netSource_info.ip[3], info->ip[1], IP_LEN);//教师特写
	memcpy(g_net_info.netSource_info.ip[4], info->ip[2], IP_LEN);//学生特写
	
	memcpy(g_net_info.netSource_info.streamName[0], info->streamName0[3], LIVE_STREAM_NAME_MAX);//教师全景
	memcpy(g_net_info.netSource_info.streamName[1], info->streamName0[4], LIVE_STREAM_NAME_MAX);//学生全景
	memcpy(g_net_info.netSource_info.streamName[2], info->streamName0[6], LIVE_STREAM_NAME_MAX);//板书
	memcpy(g_net_info.netSource_info.streamName[3], info->streamName0[1], LIVE_STREAM_NAME_MAX);//教师特写
	memcpy(g_net_info.netSource_info.streamName[4], info->streamName0[2], LIVE_STREAM_NAME_MAX);//学生特写
	
/*
	for (i = 0; i < RECORD_NET_SOURCE; i ++)
	{
		memcpy(g_net_info.netSource_info.ip[i], info->ip[i + NET_SOURCE_NUMB], IP_LEN);
	}*/
	pthread_mutex_unlock(&(g_net_info.net_mutex));
	return RET_SUCCESS;
}

static int privRecord_set_camInfo(Record_cam_info_t *info)
{
	if (NULL == info)
	{
		nslog(NS_ERROR, "privRecord_set_camInfo param is err!!!\n");
		return -1;
	}
	int ret = 0;

	nslog(NS_ERROR, "the info->ppt:%d info->ptea:%d info->tea:%d info->blb:%d info->pstu:%d info->stu:%d\n", info->ppt, info->ptea,info->tea, info->blb, info->pstu,info->stu);

	int switch_flag[RECORD_NET_SOURCE] = {0};
	switch_flag[NET_0] = info->ptea;
	switch_flag[NET_1] = info->pstu;
	switch_flag[NET_2] = info->blb;
	switch_flag[NET_3] = info->tea;
	switch_flag[NET_4] = info->stu;

	ret = record_set_switch(switch_flag);
	if (0 != ret)
	{
		nslog(NS_ERROR, "record_set_switch is err!!\n");
	}
	
	return 0;

}


static int privRecord_get_pauseInfo()
{
	Record_Status_t record_status = -1;
	record_get_status(&record_status);
	if (RECORD_RESUME != record_status)
	{
		return RET_CONFLICT_CMD;//不是正在录像
	}

	record_set_signal(SIGNAL_PAUSE);
	return RET_SUCCESS;
}

static int record_process_cmdFunc(Communtication_Head_t *head, void *msg , Commutication_Handle_t handle)
{

	if (NULL == head || NULL == msg ||   NULL == handle)
	{
		nslog(NS_ERROR, "head=%p,msg=%p,handle=%p\n", head, msg, handle);
		return -1;
	}

	if (check_contorl_to_record_cmd(head->cmd) != 0)
	{
		nslog(NS_ERROR, "the cmd[0x%04x] [0x%04x]is not invaild\n", head->cmd, (head->cmd) >> 8);
		return -1;
	}

	int unknow_cmd = 0;
	int error_struct_len = 0;
	int error_info = -1;
	int set_mode = 0;

	nslog(NS_DEBUG, "\033[32m""cmd = 0x%04x.[%s]\n""\033[0m", head->cmd, get_cmd_name(head->cmd));

	switch (head->cmd)
	{
		case CMD_RECORD_START:   //设置start
			if (head->struct_len != sizeof(Record_start_info_s))
			{
				error_struct_len = -1;
				nslog(NS_ERROR, "CMD_RECORD_START struct len =[%d ,%d]is error\n", head->struct_len, sizeof(Record_start_info_s));
			}
			else
			{
				Record_start_info_s *info = NULL;
				info = (Record_start_info_s *)msg;
				error_info = privRecord_get_startInfo(info);
			}

			break;

		case CMD_RECORD_PAUSE:   //设置pause
			if (head->struct_len != sizeof(Record_pause_info_s))
			{
				error_struct_len = -1;
				nslog(NS_ERROR, "CMD_RECORD_PAUSE struct len =[%d ,%d]is error\n", head->struct_len, sizeof(Record_pause_info_s));
			}
			else
			{
				error_info = privRecord_get_pauseInfo();
			}

			break;

		case CMD_RECORD_STOP:   //设置stop
			if (head->struct_len != sizeof(Record_stop_info_s))
			{
				error_struct_len = -1;
				nslog(NS_ERROR, "CMD_RECORD_STOP struct len =[%d ,%d]is error\n", head->struct_len, sizeof(Record_stop_info_s));
			}
			else
			{
				Record_stop_info_s *info = NULL;
				info = (Record_stop_info_s *)msg;
				error_info = privRecord_get_stopInfo(info);
			}
			break;

		case CMD_RECORD_SET_NET:   //设置网络摄像机信息
			if (head->struct_len != sizeof(Live_ServerStream_Info_t))
			{
				error_struct_len = -1;
				nslog(NS_ERROR, "CMD_RECORD_STOP struct len =[%d ,%d]is error\n", head->struct_len, sizeof(Live_ServerStream_Info_t));
			}
			else
			{
				Live_ServerStream_Info_t *info = NULL;
				info = (Live_ServerStream_Info_t *)msg;
				error_info = privRecord_set_netInfo(info);
			}
			break;
		case CMD_RECORD_SET_CAM:
			if (head->struct_len != sizeof(Record_cam_info_t))
			{
				error_struct_len = -1;
				nslog(NS_ERROR, "CMD_RECORD_SET_CAM struct len =[%d ,%d]is error\n", head->struct_len, sizeof(Record_cam_info_t));
			}
			else
			{
				Record_cam_info_t *info = NULL;
				info = (Record_cam_info_t *)msg;
				error_info = privRecord_set_camInfo(info);
			}
			break;
		case CMD_RECORD_SET_STATICS:
			
			break;
			
		default:
			unknow_cmd = -1;
			nslog(NS_WARN, "head->cmd=%d---CMD-Unknow!!!!\n", head->cmd);
			break;
	}

	//对异常消息的处理，通知回去
	head->return_code = (unsigned int)RET_CMD_SUCCESS;

	if (unknow_cmd != 0)
	{
		head->return_code = RET_UKNOW_CMD;
	}

	if (error_struct_len != 0)
	{
		head->return_code = RET_ERR_STRUCT;
	}

	if (error_info != 0)
	{
		head->return_code = RET_CONFLICT_CMD;
	}

	if (head->return_code != (unsigned int)RET_CMD_SUCCESS)
	{
		nslog(NS_WARN, "head->return_code = %d\n", head->return_code);
	}

	//some set cmd  must return get cmd;
	if (set_mode == 1)
	{
		head->cmd = head->cmd + 1;
		nslog(NS_DEBUG, "set cmd will chang to get cmd = [0x%04x]\n", head->cmd);
	}

	communtication_send_serverMsg(head, msg, head->struct_len, handle);
	
	return 0;
}
int record_init_communtication()
{
	int ret = -1;
	char IP[16] = {0};
	ret = ReachGetIPaddrstring(LOCAL_IP_ETH, IP);
	if (0 != ret)
	{
		nslog(NS_ERROR, "ReachGetIPaddrstring err!\n");
		strcpy(IP, "127.0.0.1");
	}
	
	s_record_srvHandle = communtication_create_serverHandle(IP, C_CONTROL_RECORD, record_process_cmdFunc, privRecord_get_heartValue);
	if (NULL == s_record_srvHandle)
	{
		nslog(NS_ERROR, "s_record_srvHandle=%p\n", s_record_srvHandle);
		return -1;
	}

	return 0;
}

