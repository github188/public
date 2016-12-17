#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <asm/types.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include "mid_timer.h"
#include "nslog.h"
#include "communtication.h"
#include "share_stream.h"
#include "list_use_lock.h"
#include "edukit_port.h"
#include "edukit_common.h"
#include "record_communtication.h"
#include "record_main.h"
#include "db_middle.h"
#include "tixmlconfig.h"
#include "edukit_network.h"
#include "Media.h"
#include "media_msg.h"
#include "rtspServer_base.h"
#include "non_linear_editing.h"
#include "edukit_conf.h"
#include "share_device.h"

#define RECORD_MAX_TIME		(4*60*60)

#define RECORD_MAX_BUFLEN	(1920*1080*2)

#define RECORD_MOIVE			2
#define RECORD_RES				1

static Record_Handle_t s_record_handle;

//static int s_stream_flag[RECORD_STREAM_MAX] = {0};
Record_Stream_t g_stream_flag;
static int g_stop_flag[RECORD_NET_SOURCE] = {0};
Record_Conctrol_t g_record_control;
Record_CourseInfo_t g_record_courseInfo;
Record_Switch_t g_switch_flag;

Recv_ClientInfo_t *pClient_handle[RECORD_NET_SOURCE] = {NULL};
Record_Net_t g_net_info;
RtSpClientHandle_t pRtspHandle[RECORD_NET_SOURCE] = {NULL};

typedef	void SIGFUNC(int);
SIGFUNC *SIGNAL(int signo, SIGFUNC *func)
{
	SIGFUNC	*sigfunc;
	if((SIG_ERR == (sigfunc = signal(signo, func))))
	{
		nslog(NS_ERROR,"ERROR:  signal error \n");
	}
	return (sigfunc);
}

int  record_set_switch(int *switch_flag)
{	
	if (NULL == switch_flag)
	{
		nslog(NS_ERROR, "record_set_switch param is err!!\n");
		return -1;
	}
	
	pthread_mutex_lock(&(g_switch_flag.mutex));
	memcpy(g_switch_flag.switch_flag, switch_flag, sizeof(g_switch_flag.switch_flag));
	pthread_mutex_unlock(&(g_switch_flag.mutex));
	return 0;
}

int  record_get_switch(int *switch_flag)
{	
	pthread_mutex_lock(&(g_switch_flag.mutex));
	memcpy(switch_flag, g_switch_flag.switch_flag, sizeof(g_switch_flag.switch_flag));
	pthread_mutex_unlock(&(g_switch_flag.mutex));
	return 0;
}

int  record_set_stream(int index, int flag)
{	
	if (((flag != 0) && (flag != 1)) || ((index < 0) || (index > 7)))
	{
		nslog(NS_ERROR, "record_set_stream param is err!!\n");
		return -1;
	}
	
	pthread_mutex_lock(&(g_stream_flag.mutex));
	g_stream_flag.stream_flag[index] = flag;
	pthread_mutex_unlock(&(g_stream_flag.mutex));
	return 0;
}

int  record_get_stream(int index, int *flag)
{	
	if ((index < 0) || (index > 7))
	{
		nslog(NS_ERROR, "record_get_stream param is err!!\n");
		return -1;
	}
	
	pthread_mutex_lock(&(g_stream_flag.mutex));
	*flag = g_stream_flag.stream_flag[index];
	pthread_mutex_unlock(&(g_stream_flag.mutex));
	return 0;
}

int  record_get_status(Record_Status_t *status)
{	
	pthread_mutex_lock(&(s_record_handle.mutex));
	*status = s_record_handle.record_status;
	pthread_mutex_unlock(&(s_record_handle.mutex));
	return 0;
}


void record_set_status(Record_Status_t status)
{
	Record_Status_t prev_status = 0 ;
	pthread_mutex_lock(&(s_record_handle.mutex));
	prev_status =  s_record_handle.record_status;

	if(RECORD_RESUME == status || RECORD_PAUSE == status || RECORD_CLOSE == status)
	{
		s_record_handle.record_status = status;
	}

	pthread_mutex_unlock(&(s_record_handle.mutex));
	nslog(NS_INFO, "Set Old status =%d  New status = %d ", prev_status, status);
/*
	if(status != prev_status) {
		record_report_controlMsg(status, 0);
	}
*/
	return ;
}

int  record_get_signal(void)
{
	return  s_record_handle.record_signal;
}

void record_set_signal(Record_Signal_t status)
{
	Record_Signal_t prev_status = 0 ;
	pthread_mutex_lock(&(s_record_handle.signal_mutex));
	prev_status =  s_record_handle.record_signal;

	if (SIGNAL_START == status || SIGNAL_PAUSE == status || SIGNAL_CLOSE == status)
	{
		s_record_handle.record_signal = status;
	}

	pthread_mutex_unlock(&(s_record_handle.signal_mutex));
	nslog(NS_INFO, "Set Old status =%d  New status = %d ", prev_status, status);
/*
	if(status != prev_status) {
		record_report_controlMsg(status, 0);
	}
*/
	return ;
}

int priv_get_recordTime(int *time)
{
	pthread_mutex_lock(&(s_record_handle.time_mutex));
	*time = s_record_handle.record_time.time;
	pthread_mutex_unlock(&(s_record_handle.time_mutex));
	return 0;
}

int record_set_time(int time)//0ÎªÇ°Ò»¸ö×´Ì¬ÊÇ¹Ø±Õ£¬1ÎªÖ®Ç°×´Ì¬ÎªÔÝÍ£
{
	
	pthread_mutex_lock(&(s_record_handle.time_mutex));
	s_record_handle.record_time.time = time;
	pthread_mutex_unlock(&(s_record_handle.time_mutex));
	return 0;
}


static void free_record_malloc(Media_Info_t **stream)
{		
	if (NULL != (*stream)->data)
	{
		r_free((*stream)->data);
		(*stream)->data = NULL;
	}
		
	if (NULL != *stream)
	{
		r_free(*stream);
		*stream = NULL;
	}
}

<<<<<<< Updated upstream
//Êý¾Ý½ÓÊÕÄ£¿é»Øµ÷º¯Êý
static int  record_process_data(RH_FRAMEHEAD_t *fh, void *pEmptyBufInfo, void *arg, void *data)
{
	if (NULL == arg || NULL == fh || NULL == data)
	{
		nslog(NS_WARN, "arg=%p,pEmptyBufInfo=%p,fh=%p,data=%p\n", arg, pEmptyBufInfo, fh, data);
		return -1;
	}
	
	Recv_ClientInfo_t *net_info = (Recv_ClientInfo_t *)arg;
	RH_FRAMEHEAD_t *head = fh;

	record_set_stream(net_info->index, 1);
	int switch_seat = FIV_SEAT;
	share_get_seat(&switch_seat);
	Record_Status_t record_status = RECORD_CLOSE;
	record_get_status(&record_status);
	if ((RECORD_CLOSE == record_status) || (RECORD_PAUSE == record_status))//¹Ø±Õ»òÕßÔÝÍ£×´Ì¬Ê±²»½ÓÊÜÊý¾Ý
	{
		if (NULL != data)
		{
			r_free(data);
			data = NULL;
		}
		return 0;
	}

	int switch_flag[RECORD_STREAM_MAX] = {0};
	record_get_switch(switch_flag);
	if (0 == switch_flag[net_info->index])
	{
		if (NULL != data)
		{
			r_free(data);
			data = NULL;
		}
		return 0;
	}


	static unsigned int time[RECORD_STREAM_MAX] = {0};
	
	if (net_info->index >= RECORD_STREAM_MAX || net_info->index < 0)
	{
		nslog(NS_WARN, "net_info->index=%d \n", net_info->index);
		r_free(data);
		data = NULL;
		return -1;
	}

	if (H264_CODEC_TYPE == head->nDataCodec)
	{
		Media_Info_t *stream = r_malloc(sizeof(Media_Info_t));
		if (NULL == stream)
		{
			nslog(NS_ERROR, "malloc is fail\n");
			return -1;
		}
		memset(stream, 0, sizeof(Media_Info_t));
		stream->data = r_malloc(head->nFrameLength);
		if (NULL == stream->data)
		{
			if (NULL != data)
			{
				r_free(data);
				data = NULL;
			}

			if (NULL != stream)
			{
				r_free(stream);
				stream = NULL;
			}
			nslog(NS_DEBUG,"malloc failed......\n");
			return -1;
		}

		stream->time_tick = head->nPTimeTick;
		stream->decode_tick = head->nDTimeTick;
		stream->data_len = head->nFrameLength;
		stream->channel = net_info->index;
		stream->packet_num = head->nPacketNumber;
		stream->data_type = R_VIDEO;
		stream->framerate = head->nFrameRate;
		stream->Iframe = head->nIframe;
		stream->width = head->nWidth;
		stream->height = head->nHight;
		stream->blue_flag =  head->s_blue;

		memcpy(stream->data, data, stream->data_len);
		
		if (mid_clock() - time[net_info->index] > 100)
		{
			nslog(NS_DEBUG, "-index=%d-timeval=%d---!!\n", net_info->index, mid_clock() - time[net_info->index]);
		}

		time[net_info->index] = mid_clock();

		/*ÍÆËÍvideo_data*/
	
		if (list_lockAndGet_size(net_info->pListLock_Handle) < 30)
		{
			list_lockAndPush_back(net_info->pListLock_Handle, stream);
		}
		else
		{
			nslog(NS_ERROR, "chennel=%d list is over %d time=%d\n", net_info->index, list_lockAndGet_size(net_info->pListLock_Handle), stream->decode_tick);
			free_record_malloc(&stream);
		}
		
	} 
	else if(AAC_CODEC_TYPE == head->nDataCodec)
	{
		Media_Info_t *stream_bak[RECORD_STREAM_MAX] = {NULL};
		int i = 0;
		for (i = 0; i < RECORD_STREAM_MAX; i++)
		{
			if ((AUDIO == i) || (0 == switch_flag[i]))
			{
				continue;
			}
		
			if ((THR_SEAT == switch_seat) && ((NET_BLB == i) || (NET_STU == i)))
			{
				continue;
			}

			if ((TWO_SEAT == switch_seat) && ((NET_BLB == i) || (NET_STU == i) || (NET_PSTU == i)))
			{
				
				continue;
			}

			if ((FOU_SEAT == switch_seat) && (i == NET_BLB))//ËÄ»úÎ»
			{
				continue;
			}
			
			stream_bak[i] = r_malloc(sizeof(Media_Info_t));
			if (NULL == stream_bak[i])
			{
				if (NULL != data)
				{
					r_free(data);
					data = NULL;
				}
				
				nslog(NS_ERROR, "malloc is fail\n");
				return -1;
			}
			memset(stream_bak[i], 0, sizeof(Media_Info_t));
			
			stream_bak[i]->data = r_malloc(head->nFrameLength);
			if (NULL == stream_bak[i]->data)
			{	
				if (NULL != data)
				{
					r_free(data);
					data = NULL;
				}
				if (NULL != stream_bak[i])
				{
					r_free(stream_bak[i]);
					stream_bak[i] = NULL;
				}
				nslog(NS_DEBUG,"malloc failed......\n");
				return -1;
			}
			
			stream_bak[i]->data_type = R_AUDIO;
			stream_bak[i]->sample_rate = head->nSamplerate;
			stream_bak[i]->channel = head->nChannel;
			stream_bak[i]->audio_BitRate = head->nSamplebit;
			stream_bak[i]->data_len = head->nFrameLength;
			stream_bak[i]->decode_tick = head->nDTimeTick;
			memcpy(stream_bak[i]->data, data, stream_bak[i]->data_len);

			int stream_flag = 0;
			record_get_stream(i, &stream_flag);
			if ((list_lockAndGet_size(pClient_handle[i]->pListLock_Handle) < 30) && (1 == stream_flag))
			{
				list_lockAndPush_back(pClient_handle[i]->pListLock_Handle, stream_bak[i]);
			}
			else
			{
				nslog(NS_ERROR, "ch = %d list is over %d flag=%d\n",pClient_handle[i]->index, list_lockAndGet_size(pClient_handle[i]->pListLock_Handle),stream_flag);
				free_record_malloc(&(stream_bak[i]));
			}
		}
	}

	if (NULL != data)
	{
		r_free(data);
		data = NULL;
	}
	return 0;
}

||||||| merged common ancestors
//Êý¾Ý½ÓÊÕÄ£¿é»Øµ÷º¯Êý
static int  record_process_data(RH_FRAMEHEAD_t *fh, void *pEmptyBufInfo, void *arg, void *data)
{
	if (NULL == arg || NULL == fh || NULL == data)
	{
		nslog(NS_WARN, "arg=%p,pEmptyBufInfo=%p,fh=%p,data=%p\n", arg, pEmptyBufInfo, fh, data);
		return -1;
	}
	
	Recv_ClientInfo_t *net_info = (Recv_ClientInfo_t *)arg;
	RH_FRAMEHEAD_t *head = fh;

	record_set_stream(net_info->index, 1);
	int switch_seat = FIV_SEAT;
	share_get_seat(&switch_seat);
	Record_Status_t record_status = RECORD_CLOSE;
	record_get_status(&record_status);
	if ((RECORD_CLOSE == record_status) || (RECORD_PAUSE == record_status))//¹Ø±Õ»òÕßÔÝÍ£×´Ì¬Ê±²»½ÓÊÜÊý¾Ý
	{
		if (NULL != data)
		{
			r_free(data);
			data = NULL;
		}
		return 0;
	}

	int switch_flag[RECORD_STREAM_MAX] = {0};
	record_get_switch(switch_flag);
	if (0 == switch_flag[net_info->index])
	{
		if (NULL != data)
		{
			r_free(data);
			data = NULL;
		}
		return 0;
	}


	static unsigned int time[RECORD_STREAM_MAX] = {0};
	
	if (net_info->index >= RECORD_STREAM_MAX || net_info->index < 0)
	{
		nslog(NS_WARN, "net_info->index=%d \n", net_info->index);
		r_free(data);
		data = NULL;
		return -1;
	}

	if (H264_CODEC_TYPE == head->nDataCodec)
	{
		Media_Info_t *stream = r_malloc(sizeof(Media_Info_t));
		if (NULL == stream)
		{
			nslog(NS_ERROR, "malloc is fail\n");
			return -1;
		}
		memset(stream, 0, sizeof(Media_Info_t));
		stream->data = r_malloc(head->nFrameLength);
		if (NULL == stream->data)
		{
			if (NULL != data)
			{
				r_free(data);
				data = NULL;
			}

			if (NULL != stream)
			{
				r_free(stream);
				stream = NULL;
			}
			nslog(NS_DEBUG,"malloc failed......\n");
			return -1;
		}

		stream->time_tick = head->nPTimeTick;
		stream->decode_tick = head->nDTimeTick;
		stream->data_len = head->nFrameLength;
		stream->channel = net_info->index;
		stream->packet_num = head->nPacketNumber;
		stream->data_type = R_VIDEO;
		stream->framerate = head->nFrameRate;
		stream->Iframe = head->nIframe;
		stream->width = head->nWidth;
		stream->height = head->nHight;
		stream->blue_flag =  head->s_blue;

		memcpy(stream->data, data, stream->data_len);
		
		if (mid_clock() - time[net_info->index] > 100)
		{
			nslog(NS_DEBUG, "-index=%d-timeval=%d---!!\n", net_info->index, mid_clock() - time[net_info->index]);
		}

		time[net_info->index] = mid_clock();

		/*ÍÆËÍvideo_data*/
	
		if (list_lockAndGet_size(net_info->pListLock_Handle) < 30)
		{
			list_lockAndPush_back(net_info->pListLock_Handle, stream);
		}
		else
		{
			nslog(NS_ERROR, "chennel=%d list is over %d time=%d\n", net_info->index, list_lockAndGet_size(net_info->pListLock_Handle), stream->decode_tick);
			free_record_malloc(&stream);
		}
		
	} 
	else if(AAC_CODEC_TYPE == head->nDataCodec)
	{
		Media_Info_t *stream_bak[RECORD_STREAM_MAX] = {NULL};
		int i = 0;
		for (i = 0; i < RECORD_STREAM_MAX; i++)
		{
			if ((AUDIO == i) || (0 == switch_flag[i]))
			{
				continue;
			}
		
			if ((THR_SEAT == switch_seat) && ((NET_BLB == i) || (NET_STU == i)))
			{
				continue;
			}

			if ((TWO_SEAT == switch_seat) && ((NET_BLB == i) || (NET_STU == i) || (NET_PSTU == i)))
			{
				
				continue;
			}

			if ((FOU_SEAT == switch_seat) && (i == NET_BLB))//ËÄ»úÎ»
			{
				continue;
			}
			
			stream_bak[i] = r_malloc(sizeof(Media_Info_t));
			if (NULL == stream_bak[i])
			{
				if (NULL != data)
				{
					r_free(data);
					data = NULL;
				}
				
				nslog(NS_ERROR, "malloc is fail\n");
				return -1;
			}
			memset(stream_bak[i], 0, sizeof(Media_Info_t));
			
			stream_bak[i]->data = r_malloc(head->nFrameLength);
			if (NULL == stream_bak[i]->data)
			{	
				if (NULL != data)
				{
					r_free(data);
					data = NULL;
				}
				if (NULL != stream_bak[i])
				{
					r_free(stream_bak[i]);
					stream_bak[i] = NULL;
				}
				nslog(NS_DEBUG,"malloc failed......\n");
				return -1;
			}
			
			stream_bak[i]->data_type = R_AUDIO;
			stream_bak[i]->sample_rate = head->nSamplerate;
			stream_bak[i]->channel = head->nChannel;
			stream_bak[i]->audio_BitRate = head->nSamplebit;
			stream_bak[i]->data_len = head->nFrameLength;
			stream_bak[i]->decode_tick = head->nDTimeTick;
			memcpy(stream_bak[i]->data, data, stream_bak[i]->data_len);

			int stream_flag = 0;
			record_get_stream(i, &stream_flag);
			if ((list_lockAndGet_size(pClient_handle[i]->pListLock_Handle) < 30) && (1 == stream_flag))
			{
				list_lockAndPush_back(pClient_handle[i]->pListLock_Handle, stream_bak[i]);
			}
			else
			{
				//nslog(NS_ERROR, "ch = %d list is over %d flag=%d\n",pClient_handle[i]->index, list_lockAndGet_size(pClient_handle[i]->pListLock_Handle),stream_flag);
				free_record_malloc(&(stream_bak[i]));
			}
		}
	}

	if (NULL != data)
	{
		r_free(data);
		data = NULL;
	}
	return 0;
}

=======
>>>>>>> Stashed changes
static int record_process_streamMsg(int msg_code, void *arg)
{
	Recv_ClientInfo_t *net_info = (Recv_ClientInfo_t *)arg;

	if (NULL == net_info)
	{
		nslog(NS_ERROR, "net_info ==NUL \n");
		return -1;
	}

	nslog(NS_ERROR, "--index=%d,No stream--msg_code=%d\n", net_info->index, msg_code);

	if (msg_code < 0)
	{
		nslog(NS_ERROR, "stream is err !net_info->index :%d", net_info->index);
		record_set_stream(net_info->index, 0);
	}

	return 0;
}

List_LockHandle_t* priv_create_listHande()
{
	List_LockHandle_t* pList_Handle = NULL;
	
	pList_Handle = list_lockAndCreate();
	return pList_Handle;
}

static int record_clear_list(Recv_ClientInfo_t *pClient_handle)
{
	if (NULL == pClient_handle)
	{
		nslog(NS_ERROR, "record_clear_list param is err!\n");
		return -1;
	}
	
	Media_Info_t *pStream_messgage;
	int list_size = 0;
	/*********Çå³ýÁ´±í**********/
	while ((list_size = list_lockAndGet_size(pClient_handle->pListLock_Handle)) > 0)
	{
		nslog(NS_DEBUG, "the list size is:%d %d\n", list_size, pClient_handle->index);
		pStream_messgage = (Media_Info_t *)list_lockAndPop_front(pClient_handle->pListLock_Handle);

		if (NULL != pStream_messgage->data)
		{
			r_free(pStream_messgage->data);
			pStream_messgage->data = NULL;
		}

		if (NULL != pStream_messgage)
		{
			r_free(pStream_messgage);
			pStream_messgage = NULL;
		}
	}
	return 0;
}
/*
static int record_write_aviFile(Recv_ClientInfo_t *pClient_handle, char * filename, int *time, int64_t * len_total)
{

	if ((NULL == pClient_handle) || (NULL == filename))
	{
		nslog(NS_ERROR, "record_write_mp4File param is err!\n");
		return -1;
	}


}
*/

static int record_over_time(int *time, Add_Frame_T *add_frame_info, int channel)
{
	
	if ((add_frame_info->Vlpts / 1000 >= RECORD_MAX_TIME) && (add_frame_info->index == ENC_SWMS))
	{
		record_set_signal(SIGNAL_CLOSE);
		*time = add_frame_info->Vlpts / 1000;
	}

	return 0;
}

static int record_write_mp4File(Recv_ClientInfo_t *pClient_handle, char * filename, int *time)
{
	if ((NULL == pClient_handle) || (NULL == filename))
	{
		nslog(NS_ERROR, "record_write_mp4File param is err!\n");
		return -1;
	}
	
	int VideoStreamIdx = -1;
	int AudioStreamIdx = -1;
	int flag_firstIframe = 0;
	
	uint32_t time_base = 0;
	uint32_t time_base_audio = 0;
	int list_size = 0;
	int ret=0;
	int ncount = 0;
	int reconect = 0;
	
	Media_Info_t *pStream_messgage = NULL;
	List_LockHandle_t* pListLock_Handle = NULL;
	pListLock_Handle = pClient_handle->pListLock_Handle;
	MuxWriter *pMW = r_malloc(sizeof(MuxWriter));
	if (NULL == pMW)
	{
		nslog(NS_ERROR, "it is malloc failed\n");
		return -1;
	}

	record_clear_list(pClient_handle);
	MediaWriterInit(pMW);
	if (MediaFileCreate(pMW, filename, 1) < 0)
	{
		nslog(NS_ERROR, "MediaFileCreate is failed: %s", filename);
		MediaWriterClose(pMW);
		if (NULL != pMW)
		{
			r_free(pMW);
			pMW = NULL;
		}
		return -1;
	}
	
	Add_Frame_T *add_frame_info = nonLinearEditing_init(pMW, &MediaWriteFrame);
	if (!add_frame_info )
	{
		nslog(NS_ERROR,"Create Add Frame failed!!!\n");
		return -1;
	}
	
	Record_Status_t status = RECORD_CLOSE;
	while (g_record_control.record_flag)
	{	
		record_get_status(&status);
		if (RECORD_PAUSE == status)
		{
			add_frame_info->priv_editingData.start_time = 0;
			add_frame_info->pause_flag = 1;
			add_frame_info->priv_audioData.a_ref_time = 0;
			add_frame_info->priv_audioData.count = 0;
			record_clear_list(pClient_handle);
			usleep(160000);
			nslog(NS_INFO, "the status is add_frame_info->Vlpts=%d add_frame_info->frame_total=%d RECORD_PAUSE\n", add_frame_info->Vlpts, add_frame_info->frame_total);;
			continue;
		}

		if ((list_size = list_lockAndGet_size(pListLock_Handle)) <  0)
		{		
			usleep(90000);
			continue;
		}
		else if (0 == list_size)
		{
<<<<<<< Updated upstream
			if (pClient_handle->index > AUDIO)
			{
				nslog(NS_INFO, "the count is %d\n", ncount);
				ncount++;
			}
||||||| merged common ancestors
			if (pClient_handle->index > AUDIO)
			{
				ncount++;
			}
=======
			ncount++;
>>>>>>> Stashed changes
			usleep(160000);
			continue;
		}
		
		/***************************/
<<<<<<< Updated upstream
		if ((video_flag >= 300) || (ncount >= 80))
||||||| merged common ancestors
		if ((video_flag >= 300) || (ncount >= 40))
=======
		if (ncount >= 100)//16ÃëÄÚÍ¨µÀÖÐÃ»ÓÐÊý¾ÝÖØÁ¬
>>>>>>> Stashed changes
		{
			nslog(NS_INFO, "rtsp_client_reConnect  ncount=%d pClient_handle->index=%d\n", ncount, pClient_handle->index);
			if ((pClient_handle->index > AUDIO) && (NULL != pRtspHandle[pClient_handle->index]))
			{
				ret = rtsp_client_reConnect(pRtspHandle[pClient_handle->index]);
				if (0 != ret)
				{
					nslog(NS_ERROR, "rtsp_client_reConnect is failed\n");
				}

				ncount = 0;
				reconect = 1;
			}
		}
		
		if (1 == reconect)
		{
			nslog(NS_ERROR, "we must clear the list\n");
			record_clear_list(pClient_handle);
			reconect = 0;
			add_frame_info->priv_editingData.start_time = 0;
			add_frame_info->pause_flag = 1;
			usleep(100);
			continue;
		}
		
		reconect = 0;
		ncount = 0;
		/*****************************/
		//nslog(NS_ERROR, "list_lockAndPop_front list is %p\n", pListLock_Handle);		
		pStream_messgage = (Media_Info_t *)list_lockAndPop_front(pListLock_Handle);
		if (NULL == pStream_messgage)
		{
			nslog(NS_ERROR, "list_lockAndPop_front data is err\n");				
			usleep(50000);
			goto EXIT;
		}

		ret = record_over_time(time, add_frame_info, pStream_messgage->channel);
		if (-1 == ret)
		{
			nslog(NS_INFO, "some pthread's time is over!!!!, but not every\n");
			goto EXIT;
		}
		
		if ((R_VIDEO == pStream_messgage->data_type) && (flag_firstIframe == 0) && (pStream_messgage->Iframe) && (-1 ==VideoStreamIdx))
		{
			if ((0 == pStream_messgage->framerate) && (pClient_handle->index > 2))
			{
				nslog(NS_ERROR, "the framerate is zero!!!\n");
				usleep(100);
				continue;
			}

			if ((pStream_messgage->framerate != 25) && (pStream_messgage->framerate != 30) && (pStream_messgage->framerate != 60))
			{
				nslog(NS_ERROR, "\033[35m""the frame=%d\n""\033[0m", pStream_messgage->framerate);
				pStream_messgage->framerate = 30;
			}
			
			if (NET_0 == add_frame_info->index)
			{
				record_set_time(0);
			}
			
			VideoStreamIdx = VideoStreamAdd(pMW, AV_CODEC_ID_H264, pStream_messgage->width, pStream_messgage->height, 0, pStream_messgage->framerate);
			if (-1 == VideoStreamIdx)
			{
				nslog(NS_ERROR, "VideoStreamAdd is err!\n");
				goto EXIT;
			}
			
			flag_firstIframe = 1;
			time_base = pStream_messgage->decode_tick;
			
			AudioStreamIdx = AudioStreamAdd(pMW, AV_CODEC_ID_AAC, 8000, 64000/*pStream_messgage->audio_BitRate*/, 2, 1);
			if (AudioStreamIdx == -1)
			{
				nslog(NS_ERROR, "AudioStreamAdd is failed!!!");
				goto EXIT;	
			}
			
		}

		if ((R_AUDIO == pStream_messgage->data_type) && (AudioStreamIdx > -1))
		{
			if( 0 == time_base_audio)
			{
				time_base_audio = pStream_messgage->decode_tick;
			}
		
			nonLinearEditing_process_audio(pStream_messgage->data, pStream_messgage->data_len, (int64_t)pStream_messgage->decode_tick- time_base_audio, AudioStreamIdx, add_frame_info);
		}

		if ((R_VIDEO == pStream_messgage->data_type) && (VideoStreamIdx > -1))
		{
			if ((pStream_messgage->framerate != 25) && (pStream_messgage->framerate != 30) && (pStream_messgage->framerate != 60))
			{
				pStream_messgage->framerate = 30;
			}
			
			nonLinearEditing_set_prm(pClient_handle->index, VideoStreamIdx, pStream_messgage->Iframe, (uint8_t*)pStream_messgage->data, pStream_messgage->data_len,
			(unsigned int )(pStream_messgage->decode_tick - time_base), pStream_messgage->framerate,  add_frame_info);
			ret  = nonLinearEditing_process(add_frame_info);
			if (ret < 0)
			{
				nslog(NS_WARN, "-video nonLinearEditing_process is failed:[%d]\n", ret);
			}

			if (((add_frame_info->frame_cnt % (pStream_messgage->framerate * 2)) == 0) && (NET_2 == add_frame_info->index))
			{
				record_set_time(add_frame_info->Vlpts / 1000);//ÉèÖÃÂ¼ÖÆÊ±¼ä
			}
		}

		
	EXIT:		
		if (NULL != pStream_messgage->data)
		{
			free(pStream_messgage->data);
			pStream_messgage->data = NULL;
		}
		
		if (NULL != pStream_messgage)
		{
			free(pStream_messgage);
			pStream_messgage = NULL;
		}
		
		usleep(10000);
	}
	
	ret = MediaWriteTrailer(pMW);
	if (0 != ret)
	{
		nslog(NS_ERROR, "MediaWriteTrailer ret is err!!!");
	}
	
	MediaWriterClose(pMW);
	nonLinearEditing_printf_info(add_frame_info);
	nonLinearEditing_deinit(&add_frame_info);
	
	if (NULL != pMW)
	{
		r_free(pMW);
		pMW = NULL;
	}
	
	record_clear_list(pClient_handle);
	return 0;
	
}

static int record_creat_mp4File(int index, char * filename)
{
	if ((index < 0) || (index > RECORD_STREAM_MAX) || (NULL == filename))
	{
		nslog(NS_ERROR, "record_prepare_file param is err index[%d] filename[%p]!!\n", index, filename);
		return -1;
	}

	char *file = NULL;
	char file_mp4[FILE_LEN] = {0};
	char file_tmp[FILE_LEN] = {0};
	sprintf(file_tmp, "%s", filename);
	file = strstr(file_tmp, "_tmp");
	if (NULL == file)
	{
		nslog(NS_ERROR, "record_creat_mp4File the filename is not right\n");
		return -1;
	}
	else
	{	
		for (;*file != 0;)
			*(file++) = 0;
	}

	if (NET_0 == index)
	{
		memset(file_mp4, 0, FILE_LEN);
		sprintf(file_mp4, "%s/%s/%s0.%s", RECORD_PATH, filename, file_tmp, RECORD_TYPE);
	}
	else if (NET_1 == index)
	{
		memset(file_mp4, 0, FILE_LEN);
		sprintf(file_mp4, "%s/%s/1.%s",RECORD_PATH, filename, RECORD_TYPE);
	}
	else if (NET_2 == index)
	{
		memset(file_mp4, 0, FILE_LEN);
		sprintf(file_mp4, "%s/%s/2.%s",RECORD_PATH, filename, RECORD_TYPE);
	}
	else if (NET_3 == index)
	{
		memset(file_mp4, 0, FILE_LEN);
		sprintf(file_mp4, "%s/%s/3.%s",RECORD_PATH, filename, RECORD_TYPE);
	}
	else if (NET_4 == index)
	{
		memset(file_mp4, 0, FILE_LEN);
		sprintf(file_mp4, "%s/%s/4.%s",RECORD_PATH, filename, RECORD_TYPE);
	}
	else if (NET_5 == index)
	{
		memset(file_mp4, 0, FILE_LEN);
		sprintf(file_mp4, "%s/%s/5.%s",RECORD_PATH, filename, RECORD_TYPE);
	}
	else if (NET_6 == index)
	{
		memset(file_mp4, 0, FILE_LEN);
		sprintf(file_mp4, "%s/%s/6.%s",RECORD_PATH, filename, RECORD_TYPE);
	}
	memcpy(filename, file_mp4, FILE_LEN);
	return 0;
}

static int  empty_global_recordInfo()
{
	pthread_mutex_lock(&(g_record_courseInfo.mutex));
	memset(g_record_courseInfo.course_subject, 0, sizeof(g_record_courseInfo.course_subject));
	memset(g_record_courseInfo.file_name, 0, sizeof(g_record_courseInfo.file_name));
	memset(g_record_courseInfo.main_teacher, 0, sizeof(g_record_courseInfo.main_teacher));
	memset(g_record_courseInfo.class_room, 0, sizeof(g_record_courseInfo.class_room));
	memset(g_record_courseInfo.notes, 0, sizeof(g_record_courseInfo.notes));
	memset(g_record_courseInfo.start_time, 0, sizeof(g_record_courseInfo.start_time));
	memset(g_record_courseInfo.start_time, 0, sizeof(g_record_courseInfo.start_time));
	//g_record_courseInfo.record_time = 0;
	g_record_courseInfo.total_size = 0;
	pthread_mutex_unlock(&(g_record_courseInfo.mutex));
	return 0;
}

int fill_global_recordInfo(Record_CourseInfo_t course_info)
{
	
	pthread_mutex_lock(&(g_record_courseInfo.mutex));
	if (strlen(course_info.record_time) > 0)
	{
		memcpy(g_record_courseInfo.record_time, course_info.record_time, sizeof(course_info.record_time));
	}
	
	if (course_info.total_size > 0)
	{
		g_record_courseInfo.total_size = course_info.total_size;
	}

	if (strlen(course_info.course_subject) > 0)
	{
		memcpy(g_record_courseInfo.course_subject, course_info.course_subject, sizeof(course_info.course_subject));
	}

	if (strlen(course_info.class_room) > 0)
	{
		memcpy(g_record_courseInfo.class_room, course_info.class_room, sizeof(course_info.class_room));
	}
	
	if (strlen(course_info.file_name) > 0)
	{
		memcpy(g_record_courseInfo.file_name, course_info.file_name, sizeof(course_info.file_name));
	}

	if (strlen(course_info.main_teacher) > 0)
	{
		memcpy(g_record_courseInfo.main_teacher, course_info.main_teacher, sizeof(course_info.main_teacher));
	}

	if (strlen(course_info.notes) > 0)
	{
		memcpy(g_record_courseInfo.notes, course_info.notes, sizeof(course_info.notes));
	}

	if (strlen(course_info.start_time) > 0)
	{
		memcpy(g_record_courseInfo.start_time, course_info.start_time, sizeof(course_info.start_time));
	}
	
	pthread_mutex_unlock(&(g_record_courseInfo.mutex));
	
	return 0;
}

static int  get_global_recordInfo(Record_CourseInfo_t *course_info)
{
	pthread_mutex_lock(&(g_record_courseInfo.mutex));
	memcpy(course_info->course_subject, g_record_courseInfo.course_subject, sizeof(course_info->course_subject));
	memcpy(course_info->file_name, g_record_courseInfo.file_name, sizeof(course_info->file_name));
	memcpy(course_info->main_teacher, g_record_courseInfo.main_teacher, sizeof(course_info->main_teacher));
	memcpy(course_info->class_room, g_record_courseInfo.class_room, sizeof(course_info->class_room));
	memcpy(course_info->notes, g_record_courseInfo.notes, sizeof(course_info->notes));
	memcpy(course_info->start_time, g_record_courseInfo.start_time, sizeof(course_info->start_time));
	memcpy(course_info->record_time, g_record_courseInfo.record_time, sizeof(course_info->record_time));
	course_info->total_size = g_record_courseInfo.total_size;
	pthread_mutex_unlock(&(g_record_courseInfo.mutex));
	return 0;
}

static void* record_process_thread(void* arg)
{
	Pthread_Param_t *pthread_param = (Pthread_Param_t *)arg;
	Recv_ClientInfo_t *pClient_handle = &(pthread_param->client_param);
	char filename[FILE_LEN] = {0};
	memcpy(filename, pthread_param->filename, FILE_LEN);
	if (NULL == pClient_handle || NULL == pClient_handle->pListLock_Handle || NULL == filename)
	{
		nslog(NS_ERROR, "record_process_thread param is NULL");
		return NULL;
	}
	
	int ret = -1;
	int time = 0;

	List_LockHandle_t* pListLock_Handle = NULL;
	pListLock_Handle = pClient_handle->pListLock_Handle;
	ret = record_creat_mp4File(pClient_handle->index, filename);
	if (0 != ret)
	{
		nslog(NS_ERROR, "record_prepare_file is err!!\n");
		return NULL;
	}

	ret = record_write_mp4File(pClient_handle, filename, &time);
	if (0 != ret)
	{
		nslog(NS_ERROR, "record_write_mp4File is err!!\n");
		return NULL;
	}
	
	Record_CourseInfo_t course_info;
	memset(&course_info, 0, sizeof(Record_CourseInfo_t));
	sprintf(course_info.record_time, "%d", time);
	fill_global_recordInfo(course_info);

	return NULL;
}

static void record_init_global()
{
	memset(&g_switch_flag, 0, sizeof(g_switch_flag));
	pthread_mutex_init(&(s_record_handle.mutex), NULL);
	pthread_mutex_init(&(s_record_handle.time_mutex), NULL);
	pthread_mutex_init(&(g_record_courseInfo.mutex), NULL);
	pthread_mutex_init(&(g_net_info.net_mutex), NULL);
	pthread_mutex_init(&(g_switch_flag.mutex), NULL);
	pthread_mutex_init(&(g_stream_flag.mutex), NULL);
	
	memset(&(g_net_info.netSource_info),0,sizeof(Record_NetSource_Info_t));
	s_record_handle.record_status = RECORD_CLOSE;
	s_record_handle.record_signal = SIGNAL_CLOSE;
	g_record_control.record_flag = 0;
	g_switch_flag.switch_flag[ENC_SWMS] = 1;
	g_switch_flag.switch_flag[AUDIO] = 1;
}

static int rtsp_state_callBack(Rtsp_ClientStream_State_t * param)
{
	if (param == NULL)
	{
		nslog(NS_WARN,"\033[33m""stateCallback is wrong\n");
		return -1;
	}

	/*
	Record_RecvNet_Handle_t* pClient_handle = (Record_RecvNet_Handle_t*)param->param;
	if (NULL == pClient_handle)
	{
		nslog(NS_ERROR, "net_info ==NULL \n");
		return -1;
	}

	if (param->status == RTSPCLIENT_STOP)
	{
		record_set_stream(pClient_handle->channel, 0);
		nslog(NS_INFO,"\033[33m""the client state %d ip:%s\n""\033[0m", param->status, pClient_handle->iP);
	}
	*/
	return 0;
}
static int record_free_streamMessage(Media_Info_t **pStream_messgage)
{
	if (*pStream_messgage == NULL)
	{
		nslog(NS_ERROR, "free_stream_message is fail\n");
		return -1;
	}
	
	if (NULL != (*pStream_messgage)->data)
	{
		free((*pStream_messgage)->data);
		(*pStream_messgage)->data = NULL;
	}
	
	free(*pStream_messgage);
	*pStream_messgage = NULL;
	return 0;
}

int record_g711_toAAc(EasyAACEncoder_Handle handle, unsigned char *aac_data, unsigned int *aac_len, unsigned char *g711u_data, int g711_len)
{
	if (NULL == handle || NULL == aac_data || NULL == g711u_data || g711_len <= 0)
	{
		nslog(NS_ERROR, "record_g711_toAAc param is err!!!\n");
		return -1;
	}

	
	if (Easy_AACEncoder_Encode(handle, g711u_data, g711_len, aac_data, aac_len) <= 0)
	{
		//nslog(NS_ERROR, "Easy_AACEncoder_Encode is err!!!!\n");
		return -1;
	}

	return 0;
}

static int rtsp_frame_call(Fream_Info_t* frame)
{
	
	Record_RecvNet_Handle_t* pClient_handle = (Record_RecvNet_Handle_t*)frame->param;
	//nslog(NS_ERROR, "\033[34m""pClient_handle=%p frame->type==%d frame->frame_size=%d""\033[0m", frame->param, frame->type, frame->frameSize);
	int ret = -1;
	unsigned int out_len = 0;
	unsigned char aac[800] = {0};
	unsigned char g711u[200] = {0};
	/*
	if (R_VIDEO != frame->type)
	{
		memcpy(g711u, frame->data, frame->frameSize);
		fwrite(g711u, 1, frame->frameSize, pClient_handle->foutp);
		nslog(NS_INFO, "it is start audio frame->frameSize=%d\n", frame->frameSize);
		return 0;
	}
	return 0;*/

	#if 1
	List_LockHandle_t* pList_handle = NULL;
	if (frame == NULL || frame->data == NULL || frame->frameSize < 4)
	{
		nslog(NS_ERROR,"\033[33m""frameCall is wrong\n");
		return -1;
	}
	
	Record_Status_t record_status = -1;
	record_get_status(&record_status);//Â¼ÖÆ×´Ì¬
	if ((RECORD_CLOSE == record_status) || (RECORD_PAUSE == record_status))//¹Ø±Õ»òÕßÔÝÍ£×´Ì¬Ê±²»½ÓÊÜÊý¾Ý
	{
		return 0;
	}
	
	record_set_stream(pClient_handle->channel, 1);//Á÷×´Ì¬
	
	int switch_flag[RECORD_STREAM_MAX] = {0};
	record_get_switch(switch_flag);//»òÕßÂ¼ÖÆÉèÖÃÊÇ·ñÒªÂ¼×´Ì¬
	if (0 == switch_flag[pClient_handle->channel])
	{
		return 0;
	}
	#endif
	if (NULL == pClient_handle || NULL == pClient_handle->pListLock_Handle)//Íâ²¿´«²Î
	{
		nslog(NS_ERROR, "callback is arg NULL pStream_recv_handle:%p  pStream_recv_handle->pListLock_Handle:%p\n",
				pClient_handle, pClient_handle->pListLock_Handle);
		return -1;
	}

	
	pList_handle = pClient_handle->pListLock_Handle;
	
	Media_Info_t * pStream_message = (Media_Info_t *)r_malloc(sizeof(Media_Info_t));
	if (pStream_message == NULL)
	{
		nslog(NS_ERROR, "malloc is fail\n");
		return -1;
	}
	
	memset(pStream_message, 0, sizeof(Media_Info_t));
	pStream_message->data = malloc(frame->frameSize);
	if (pStream_message->data == NULL)
	{
		nslog(NS_ERROR, "malloc pStream_message->stream_data is fail\n");
		record_free_streamMessage(&pStream_message);
		return -1;
	}
	
	pStream_message->data_type = frame->type;
	pStream_message->Iframe = frame->iFrame;
	pStream_message->data_len = frame->frameSize;
	pStream_message->decode_tick = get_run_time();//ÔÝÊ±Î´¼ÓÊ±¼ä´Á
	pStream_message->channel = pClient_handle->channel;
	pStream_message->width = frame->width;
	pStream_message->height = frame->height;
	pStream_message->framerate = frame->fps;
	if (R_VIDEO == frame->type)
	{
		memcpy(pStream_message->data, frame->data, frame->frameSize);
	}
	else
	{
		memcpy(pClient_handle->g711_data, frame->data, frame->frameSize);
		ret = record_g711_toAAc(pClient_handle->handle, pClient_handle->acc_data, &out_len, pClient_handle->g711_data, frame->frameSize);
		if ((0 != ret) ||(out_len == 0))
		{
			//nslog(NS_ERROR, "record_g711_toAAc is err!!!\n");
			record_free_streamMessage(&pStream_message);
			return -1;
		}
		
		pStream_message->data = realloc(pStream_message->data, out_len);
		memcpy(pStream_message->data, pClient_handle->acc_data, out_len);
		pStream_message->data_len = out_len;
		pStream_message->decode_tick = get_run_time();
		//nslog(NS_INFO, "it is start audio pStream_message->data_len=%d\n", pStream_message->data_len);
		
		if (list_lockAndGet_size(pList_handle) < 30)
		{
		
			list_lockAndPush_back(pList_handle, pStream_message);
		}
		else
		{
			nslog(NS_WARN,"\033[32m""AUDIOID:%d\n""\033[0m",  pClient_handle->channel);
			record_free_streamMessage(&pStream_message);
		}
		return 0;
		
	}
	
	if (frame->iFrame == 1 && pClient_handle->first_frame == 0)
	{
		nslog(NS_INFO,"we get I frame.......\n");
		pClient_handle->first_frame = 1;
	}
	
	if (pClient_handle->first_frame == 1 && list_lockAndGet_size(pList_handle) < 30)
	{
		list_lockAndPush_back(pList_handle, pStream_message);
	}
	else
	{
		nslog(NS_WARN,"\033[32m""pClient_handle->first_frame: %d ID:%d\n""\033[0m", pClient_handle->first_frame, pClient_handle->channel);
		record_free_streamMessage(&pStream_message);
	}
	
	return 0;
}
static int clean_malloc(Record_RecvNet_Handle_t* pStream_handle)
{
	if (pStream_handle == NULL)
	{
		nslog(NS_ERROR, "clean_alloc is fail\n");
		return -1;
	}
	
	if (pStream_handle)
	{
		if (pStream_handle->pListLock_Handle)
		{
			list_lockAndDestory(pStream_handle->pListLock_Handle);
			pStream_handle->pListLock_Handle = NULL;
		}
		
		free(pStream_handle);
		pStream_handle = NULL;
	}
	return 0;
}

static int create_stream_recv(Record_RecvNet_Handle_t* pRecv_stream_handle)
{
	int ret = 0;
	char rtspUrl[128] = {0};
	
	if (NULL == pRecv_stream_handle)
	{
		nslog(NS_ERROR, "init_stream_recv is fail\n");
		return -1;
	}
	
	Record_RecvNet_Handle_t* pStream_handle = r_malloc(sizeof(Record_RecvNet_Handle_t));
	if (pStream_handle == NULL)
	{
		nslog(NS_INFO, "pStream_handle malloc is err!!!\n");
		ret =-1;
		goto EXIT;
	}
	
	memcpy(pStream_handle, pRecv_stream_handle, sizeof(Record_RecvNet_Handle_t));

	Rtsp_Inparam  Inparam;
	Inparam.turnAudio = 1;

	if( strcmp(pRecv_stream_handle->streamName, "") == 0)
	{
		sprintf(rtspUrl,"rtsp://%s:554", pStream_handle->iP);
	}
	else
	{
		sprintf(rtspUrl,"rtsp://%s:1554/%s", pStream_handle->iP, pRecv_stream_handle->streamName);
	}
	
	nslog(NS_INFO, "url=%s %p\n", rtspUrl, pStream_handle);
	pRtspHandle[pStream_handle->channel] = rtsp_client_start(rtspUrl, rtsp_state_callBack, rtsp_frame_call, (void *)pStream_handle, &Inparam);
	if (NULL == pRtspHandle[pStream_handle->channel])
	{
		nslog(NS_ERROR, "%s : rtsp_client_start fail\n", __FILE__);
	}
	
EXIT:
	if(ret == -1)
	{
		nslog(NS_ERROR, "it is err we should clean_malloc.....................\n");
		clean_malloc(pStream_handle);
	}
	return ret;
}

static int record_creatStart_netSource(FILE* fpOut)
{
	int i = 0;
	
	Record_RecvNet_Handle_t recv_stream_handle[RECORD_NET_SOURCE];
	
	for (i = 0; i < RECORD_NET_SOURCE ; i++)
	{	
		memset(&(recv_stream_handle[i]), 0, sizeof(Record_RecvNet_Handle_t));
		while (1)
		{
	
			pthread_mutex_lock(&(g_net_info.net_mutex));
			strcpy(recv_stream_handle[i].iP, g_net_info.netSource_info.ip[i]);
			strcpy(recv_stream_handle[i].streamName, g_net_info.netSource_info.streamName[i]);
			pthread_mutex_unlock(&(g_net_info.net_mutex));
			if (recv_stream_handle[i].iP[0] != 0)
			{
				break;
			}
			sleep(1);
		}

		if ((strlen(recv_stream_handle[i].iP) == 0) || (strcmp(recv_stream_handle[i].iP, "0.0.0.0") == 0))
		{	
			usleep(2000);
			continue;
		}
		
		nslog(NS_INFO,"index=%d recvIP is =%s\n", i, recv_stream_handle[i].iP);
		pClient_handle[i] = (Recv_ClientInfo_t*)r_malloc(sizeof(Recv_ClientInfo_t));
		if (NULL == pClient_handle[i])
		{
			nslog(NS_ERROR, "record_creatStart_netSource malloc is failed\n");
			return -1;
		}
		
		(pClient_handle[i])->index = i;
		(pClient_handle[i])->pListLock_Handle = priv_create_listHande();
		if (NULL == (pClient_handle[i])->pListLock_Handle)
		{
			if (NULL != pClient_handle[i])
			{
				r_free(pClient_handle[i]);
				pClient_handle[i] = NULL;
			}
			nslog(NS_ERROR, "create list fail\n");
			return -1;
		}
		
		recv_stream_handle[i].pListLock_Handle = pClient_handle[i]->pListLock_Handle;
		recv_stream_handle[i].channel = i;
		InitParam initParam;
		initParam.u32AudioSamplerate=8000;
		initParam.ucAudioChannel = 1;
		initParam.u32PCMBitSize = 16;
		initParam.ucAudioCodec = Law_ULaw;
		EasyAACEncoder_Handle handle = Easy_AACEncoder_Init( initParam);
		recv_stream_handle[i].handle = handle;
		recv_stream_handle[i].foutp = fpOut;
		unsigned char *pbG711ABuffer = (unsigned char *)malloc(160 *sizeof(unsigned char));
		unsigned char *pbAACBuffer = (unsigned char*)malloc(640 * sizeof(unsigned char));
		recv_stream_handle[i].acc_data = pbAACBuffer;
		recv_stream_handle[i].g711_data = pbG711ABuffer;
		
		create_stream_recv(&(recv_stream_handle[i]));
	}


	return 0;
}



int creat_file_tmpdir(char *filename)
{
	localtime_t time;
	get_localtime(&time);
	unsigned char cmd[FILE_LEN] = {0};
	Record_CourseInfo_t course_info;
	memset(&course_info, 0, sizeof(Record_CourseInfo_t));
	Device_Info_t device_info;
	memset(&device_info, 0, sizeof(Device_Info_t));
	share_get_deviceInfo(&device_info);
	
	get_global_recordInfo(&course_info);
	
	int ret = strlen(course_info.main_teacher) + strlen(course_info.class_room) + strlen(course_info.course_subject);
	if (ret > 480)//¼ÓÆðÀ´²»ÄÜ³¬¹ýFILE_LEN
	{
		nslog(NS_ERROR, "creat_file_tmpdir param is err!!!\n");
		return -1;
	}
	if (0 == strlen(course_info.main_teacher) && 0 == strlen(course_info.class_room) && 0 == strlen(course_info.course_subject))
	{
		sprintf(filename,"%s%04d_%02d_%02d_%02d_%02d_%02d_tmp",device_info.record_name, time.tm_year, time.tm_mon,\
			time.tm_mday, time.tm_hour, time.tm_min,time.tm_sec);
		sprintf(course_info.file_name, "%s%04d_%02d_%02d_%02d_%02d_%02d",device_info.record_name, time.tm_year, time.tm_mon,\
			time.tm_mday, time.tm_hour, time.tm_min,time.tm_sec);
	}
	else
	{
		sprintf(filename,"%s_%s_%s_%s_%04d%02d%02d_%02d_%02d_%02d_tmp", course_info.main_teacher,course_info.course_subject,course_info.class_room, device_info.record_name,time.tm_year, time.tm_mon,\
			time.tm_mday, time.tm_hour, time.tm_min,time.tm_sec);
		sprintf(course_info.file_name, "%s_%s_%s_%s_%04d%02d%02d_%02d_%02d_%02d",course_info.main_teacher,course_info.course_subject,course_info.class_room,device_info.record_name, time.tm_year, time.tm_mon,\
			time.tm_mday, time.tm_hour, time.tm_min,time.tm_sec);
	}

	sprintf(course_info.start_time, "%04d-%02d-%02d %02d:%02d:%02d", time.tm_year, time.tm_mon,\
			time.tm_mday, time.tm_hour, time.tm_min,time.tm_sec);

	fill_global_recordInfo(course_info);
	sprintf(cmd, "mkdir -p %s/%s", RECORD_PATH, filename);
	nslog(NS_ERROR,"creat the tmp dir= %s %s\n", cmd, course_info.file_name);
	ret = r_system((const int8_t *)(cmd));
	if (-1 == ret)
	{
		nslog(NS_INFO, "mkdir -p is err!!\n");
		return -1;
	}
	
	return 0;
}

int fill_pthread_param(Pthread_Param_t *pthread_param, char *filename, Recv_ClientInfo_t *pClient)
{
	if ((filename == NULL) || (pClient == NULL))
	{
		nslog(NS_ERROR, "fill_pthread_param param is err!!!!\n");
		return -1;
	}
	
	memcpy(pthread_param->filename, filename, FILE_LEN);
	memcpy(&(pthread_param->client_param), pClient, sizeof(Recv_ClientInfo_t));
	return 0;
}

int remove_tmp_dir(char *file_name, char *file_tmp)
{
	if (NULL == file_tmp)
	{
		nslog(NS_ERROR, "remove_tmp_dir param is err!!!!\n");
		return -1;
	}

	char cmd[FILE_LEN] = {0};
	sprintf(cmd, "mv %s/%s %s/%s", RECORD_PATH, file_tmp, RECORD_PATH, file_name);
	int ret = r_system((const int8_t *)(cmd));
	if (-1 == ret)
	{
		nslog(NS_ERROR, "remove_tmp_dir is err!!!\n");
		return -1;
	}
	
	return 0;
}


static int record_store_courseInfo(Record_CourseInfo_t course_info, char *path)
{
	if (NULL == path)
	{
		nslog(NS_ERROR, "record_store_courseInfo param is err!\n");
		return -1;
	}
	
	if (access(path, F_OK) == -1)
	{
		FILE *fp = fopen(path, "w");
		if (fp == NULL)
		{
			nslog(NS_INFO,"record_store_courseInfo fopen xml file failed\n");
			return -1;
		}
		else
		{
			fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
			fprintf(fp, "<root>\n");
			fprintf(fp, "<Course_info>\n");
			fprintf(fp, "<Teacher>%s</Teacher>\n", "ITC");
			fprintf(fp, "<Course_name>%s</Course_name>\n", "ITC");
			fprintf(fp, "<Start_time>%s</Start_time>\n", "2016");
			fprintf(fp, "<Room>%s</Room>\n", "ITC");
			fprintf(fp, "<Notes>%s</Notes>\n", "ITC");
			fprintf(fp, "<Movie_size>%s</Movie_size>\n", "ITC");
			fprintf(fp, "<Res_size>%s</Res_size>\n", "ITC");
			fprintf(fp, "<Time_lenth>%s</Time_lenth>\n", "55");
			fprintf(fp, "</Course_info>\n");
			fprintf(fp, "</root>");
			fclose(fp);
		}
	}

	char movie[32] = {0};
	char res[32] = {0};
	char time[128] = {0};
	sprintf(movie, "%d", course_info.total_size);
	sprintf(res, "%d", course_info.total_size);
	//sprintf(time, "%d", course_info.record_time);
	sprintf(time, "%s", course_info.record_time);
	
	if (0 != TiXmlMgr_SetConfValue(path,	"/root/Course_info/Teacher", course_info.main_teacher)
		||0 != TiXmlMgr_SetConfValue(path, "/root/Course_info/Course_name", course_info.course_subject)
		||0 != TiXmlMgr_SetConfValue(path, "/root/Course_info/Start_time",  course_info.start_time)
		||0 != TiXmlMgr_SetConfValue(path, "/root/Course_info/Notes",  course_info.notes)
		||0 != TiXmlMgr_SetConfValue(path, "/root/Course_info/Movie_size", movie)
		||0 != TiXmlMgr_SetConfValue(path, "/root/Course_info/Res_size",  res)
		||0 != TiXmlMgr_SetConfValue(path, "/root/Course_info/Time_lenth",  time))
	{
		nslog(NS_ERROR,"TiXmlMgr_SetConfValue is err!\n");
		return -1;	
	}
	
	return 0;
}

static int is_dir(const char *path)
{
    struct stat statbuf;
    if (lstat(path, &statbuf) == 0)//lstat·µ»ØÎÄ¼þµÄÐÅÏ¢£¬ÎÄ¼þÐÅÏ¢´æ·ÅÔÚstat½á¹¹ÖÐ
    {
        return S_ISDIR(statbuf.st_mode) != 0;//S_ISDIRºê£¬ÅÐ¶ÏÎÄ¼þÀàÐÍÊÇ·ñÎªÄ¿Â¼
    }
    return -1;
}

static int is_file(const char *path)
{
	struct stat statbuf;
	if (lstat(path, &statbuf) == 0)
	{
		return S_ISREG(statbuf.st_mode) != 0;//ÅÐ¶ÏÎÄ¼þÊÇ·ñÎª³£¹æÎÄ¼þ
	}
	return -1;
}


void get_file_path(const char *path, const char *file_name,  char *file_path)
{
	strcpy(file_path, path);
	if(file_path[strlen(path) - 1] != '/')
	{
		strcat(file_path, "/");
	}
	strcat(file_path, file_name);
}


static int remove_course_abandon(char *path, const char *delim)
{
	if ((NULL == path) || (NULL == delim))
	{
		nslog(NS_ERROR, "remove_course_abandon is err!!!\n");
		return -1;
	}
	
	char subdir_path[FILE_LEN] = {0};
	char sub_file[FILE_LEN] = {0};
	int ret = 0;
	struct dirent *dir_info;
	DIR *dir_fd;

	if((dir_fd = opendir(path)) == NULL)
	{
		nslog(NS_ERROR,"open dir failed! dir: %s", path);
		return -1;
	}
	
	Device_Info_t device_info;
	memset(&device_info, 0, sizeof(Device_Info_t));
	share_get_deviceInfo(&device_info);
	//nslog(NS_ERROR, "\n th4e path delim is %s\n", path, delim);
	
	for (dir_info = readdir(dir_fd); NULL != dir_info; dir_info = readdir(dir_fd))
	{   
		if (strstr(dir_info->d_name, delim) != NULL || (strcmp(delim, device_info.device_name) == 0))
		{
			if (strcmp(dir_info->d_name, "..") == 0 || strcmp(dir_info->d_name, ".") == 0)
			{
				//nslog(NS_INFO, "the subdir_path=%s\n",dir_info->d_name);
				continue;
			}
			
			get_file_path(path, dir_info->d_name, subdir_path);
			//nslog(NS_ERROR, "the subdir_path is .....%s\n", subdir_path);
			if (1 == is_dir(subdir_path))
			{
				DIR *subdir_fd;
				if (NULL == (subdir_fd = opendir(subdir_path)))
				{
					nslog(NS_ERROR, "open dir failed! dir: %s", subdir_path);
					return -1;
				}
				
				struct dirent *subdir_info;
				for (subdir_info = readdir(subdir_fd); NULL != subdir_info; subdir_info = readdir(subdir_fd))
				{
					get_file_path(subdir_path, subdir_info->d_name, sub_file);
					nslog(NS_INFO, "the sub_file is .....%s\n", sub_file);
					if (1 == is_file(sub_file))
					{	
						Record_Status_t record_status = -1;
						record_get_status(&record_status);
						if (RECORD_CLOSE == record_status)
						{
							ret = remove(sub_file);
							if (-1 == ret)
							{
								nslog(NS_ERROR, "cannot remove the sub_file:%s\n", sub_file);
							}
							sleep(3);
							nslog(NS_INFO, " remove the sub_file:%s\n", sub_file);
						}
					}
					
				}
				
				closedir(subdir_fd);
				if (0 == ret)
				{
					ret = remove(subdir_path);
					if (-1 == ret)
					{
						nslog(NS_ERROR, "cannot remove the sub_dir:%s\n", subdir_path);
					}
					nslog(NS_INFO, "remove the sub_file:%s\n", subdir_path);
				}
			}
		}
	}
	closedir(dir_fd);
	return 0;
}

static int remove_old_file()
{	
	
	int ret = -1;

	char *xml_buf = DB_find_FileName("", 2, 0 , 1, 1, 0);
	if (NULL == xml_buf)
	{
		nslog(NS_ERROR, "db have no date!!!\n");
		return -1;
	}
	
	char *xml_bak = xml_buf;
	FILE *fp;
	fp = fopen(OLDFILE_TMP, "w+");
	if (NULL == fp)
	{
		nslog(NS_ERROR, "fopen OLDFILE_TMP is failed!!!\n");
		return -1;
	}

	while (*xml_buf != '\0')
	{
		fwrite(xml_buf, 1, 1, fp);
		xml_buf++;
	}
	
	if (NULL != xml_bak)
	{
		r_free(xml_bak);
		xml_bak = NULL;
	}

	fclose(fp);
	char filename[FILE_LEN] = {0};
	if (TiXmlMgr_GetConfValue(OLDFILE_TMP, "/ResponseMsg/MsgBody/CourseInfo/FileName", filename) != 0)
	{
		nslog(NS_ERROR, "TiXmlMgr_GetConfValue is err!!!\n");
		return -1;
	}
	nslog(NS_INFO, "\n\n the filename is %s\n\n", filename);

	ret = DB_delete_elem(filename);
	if (0 != ret)
	{
		nslog(NS_ERROR, "DB_delete_elem old file is err!!!\n");
		return -1;
	}
	return 0;
}

static void remove_course_old()
{
	int ret = -1;
	Disk_Info_t disk_info;
	memset(&disk_info, 0, sizeof(Disk_Info_t));
	ret = share_getDisk_info(&disk_info, "/dev/sda");
	if (atoi(disk_info.useperson) > SURPLUS_ABANDA)
	{
		nslog(NS_INFO, "the course  remove_course_old\n");
		remove_old_file();
	}

}

void *record_emptyBak_thread()
{
	Device_Info_t device_info;

	memset(&device_info, 0, sizeof(Device_Info_t));
	share_get_deviceInfo(&device_info);
	int fisrt_remove_abandon = 0;
	if (0 == fisrt_remove_abandon)
	{
		remove_course_abandon(RECORD_PATH, "_tmp");
		fisrt_remove_abandon = 1;
	}
	
	while (1)
	{
		Record_Status_t record_status = RECORD_RESUME;
		record_get_status(&record_status);
		if (RECORD_CLOSE == record_status)
		{	
			remove_course_abandon(DB_FILE_BAKDIR, device_info.device_name);
			remove_course_old();
		}
		sleep(3);
	}
	return NULL;
}

int record_creatStart_emptyBak()
{
	pthread_t tid;
	int record_val = -1;
	pthread_attr_t attr_record;
	pthread_attr_init(&attr_record);
	pthread_attr_setscope(&attr_record, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate(&attr_record, PTHREAD_CREATE_JOINABLE);
	record_val = pthread_create(&tid, &attr_record, record_emptyBak_thread, NULL);
	if (record_val)
	{
		nslog(NS_ERROR, "pthread_create is fail\n");
	}
	else
	{
		nslog(NS_DEBUG, "pthread_create is suceess\n");
	}

	pthread_attr_destroy(&attr_record);
	return 0;

}

int check_hardDisk()
{
	int ret = -1;
	Disk_Info_t disk_info;
	memset(&disk_info, 0, sizeof(Disk_Info_t));
	ret = share_getDisk_info(&disk_info, "/dev/sda");
	if (atoi(disk_info.useperson) > SURPLUS)
	{
		nslog(NS_INFO, "the time over RECORD_MAX_SPACE\n");
		record_set_signal(SIGNAL_CLOSE);
		return -1;
	}

	return 0;
}

static int 	switch_time(int time_tmp, char *record_time)
{
	
	int hour = time_tmp / 3600;
	int min = (time_tmp % 3600) /60;
	int sec = (time_tmp - hour * 3600 - min * 60);
	sprintf(record_time, "%02d:%02d:%02d", hour, min, sec);
	return 0;
}

static int get_dir_size(char *dir)
{
	if (NULL == dir)
	{
		nslog(NS_ERROR, "get_dir_size is err!!!\n");
		return -1;
	}

	DIR *d;
	struct dirent *de;
	struct stat buf; 
	int exists; 
	char filename[FILE_LEN] = {0};

	nslog(NS_ERROR, "the dir is %s\n", dir);
	if (NULL == (d = opendir(dir)))
	{
		nslog(NS_ERROR, "open dir failed! dir: %s", dir);
		return -1;
	}
	
	int64_t total_size = 0; 
  	for (de = readdir(d); de != NULL; de = readdir(d))
	{ 
		sprintf(filename, "%s/%s", dir, de->d_name);
		exists = stat(filename, &buf);
		if (exists < 0)
		{ 
  			fprintf(stderr, "Couldn't stat %s\n", filename); 
		}
		else
		{ 
  			total_size += (int64_t)buf.st_size;
		} 
	}

	nslog(NS_ERROR, "the size is %lld\n", total_size);
	return ((total_size / 1024 / 1024)>1)?(total_size / 1024 / 1024):1;
  	closedir(d); 

}

int TestG711ToAAC_standard()
{
	InitParam initParam;
	initParam.u32AudioSamplerate=8000;
	initParam.ucAudioChannel=1;
	initParam.u32PCMBitSize=16;
	//initParam.ucAudioCodec = Law_ALaw;
	initParam.ucAudioCodec = Law_ULaw;
	EasyAACEncoder_Handle handle = Easy_AACEncoder_Init( initParam);
	char* infilename = "./g711u.g711u";  //æ ‡å‡†
	char* outAacname = "./g711u.aac";

	FILE* fpIn = fopen(infilename, "rb");
	if(NULL == fpIn)
	{
		printf("%s:[%d] open %s file failed\n",__FUNCTION__,__LINE__,infilename);
		return -1;
	}

	FILE* fpOut = fopen(outAacname, "wb");
	if(NULL == fpOut)
	{
		printf("%s:[%d] open %s file failed\n",__FUNCTION__,__LINE__,outAacname);
		return -1;
	}

	int gBytesRead = 0;
	int bG711ABufferSize = 160;
	int bAACBufferSize = 4*bG711ABufferSize;//æä¾›è¶³å¤Ÿå¤§çš„ç¼“å†²åŒº
	unsigned char *pbG711ABuffer = (unsigned char *)malloc(bG711ABufferSize *sizeof(unsigned char));
	unsigned char *pbAACBuffer = (unsigned char*)malloc(bAACBufferSize * sizeof(unsigned char));  
	unsigned int out_len = 0;

	printf("dsdsdsds\n");
	unsigned int toatal_len = 0;
	while((gBytesRead = fread(pbG711ABuffer, 1, bG711ABufferSize, fpIn)) >0)
	{    
		if (Easy_AACEncoder_Encode(handle, pbG711ABuffer, gBytesRead, pbAACBuffer, &out_len) > 0)
		{
			fwrite(pbAACBuffer, 1, out_len, fpOut);
			printf("out_len=%d\n", out_len);
			toatal_len+=out_len;
		}
		else
		{
			printf("it is err!!!!!!!!!!!!!!!!!!!\n");
		}
	}
	printf("the toatal_len is %d\n", toatal_len);
	Easy_AACEncoder_Release(handle);

	free(pbG711ABuffer);
	free(pbAACBuffer);
	fclose(fpIn);
	fclose(fpOut);

	return 0;
}



int main(void)
{
	int i = 0;
	int ret = -1;
	pthread_t tid[RECORD_STREAM_MAX];
	int pthread_flag[RECORD_STREAM_MAX] = {0};
	MediaSysInit();
	char filename[FILE_LEN] = {0};
	Pthread_Param_t pthread_param[RECORD_STREAM_MAX];
	
	SIGNAL(SIGPIPE, SIG_IGN);
	
	nslog_conf_info_t info;
	memset(&info, 0, sizeof(nslog_conf_info_t));
	strcpy(info.conf_name, RECORD_NSLOG_CONF);
	strcpy(info.rules_name, RECORD_NSLOG_CNAME);
	strcpy(info.output_type, RECORD_NSLOG_OUT);

	ret = NslogInit(&info);
	if(ret != 0)
	{
		printf("NslogInit  return failed!\n");
		return -1;
	}
	
	//TestG711ToAAC_standard();
	//return 0;
	record_init_global();
	empty_global_recordInfo();

	ret = record_init_communtication();
	if (-1 == ret)
	{
		nslog(NS_ERROR, "record_init_communtication is err!\n");
		return -1;
	}
	
	char* outAacname = "./g711u.g711u";
	FILE* fpOut = fopen(outAacname, "wb");
	if(NULL == fpOut)
	{
		printf("%s:[%d] open %s file failed\n",__FUNCTION__,__LINE__,outAacname);
		return -1;
	}
	
	
	ret = record_creatStart_netSource(fpOut);
	if (0 != ret)
	{
		nslog(NS_ERROR,"record_creatStart_netSource is err!\n");
		return -1;
	}
	
	/*
	while(1)
	{
		sleep(1);
		char ch = getchar();
		if ('q' == ch)
		{
			fclose(fpOut);
			return 0;
		}
	}
	*/
	
	record_creatStart_emptyBak();
	sleep(3);
	
	while (1)
	{
		Record_Status_t record_status = -1;
		record_get_status(&record_status);
		Record_Signal_t record_signal = record_get_signal();
		nslog(NS_INFO,"record_status:%d record_signal:%d\n",record_status,record_signal);
		int switch_flag[RECORD_NET_SOURCE] = {0};
		record_get_switch(switch_flag);
		int flag = 0;
		#if 1
		for (i = 0; i < RECORD_NET_SOURCE; i++)
		{
			if (OPEN == switch_flag[i])
			{
				flag = 1;
				break;
			}
		}

		if (0 == flag)
		{
			sleep(1);
			continue;
		}
		#endif
		if ((SIGNAL_START == record_signal) && (RECORD_CLOSE == record_status))
		{
			ret = check_hardDisk();
			if (ret != 0)
			{	
				usleep(9000);
				continue;
			}
			
			if (creat_file_tmpdir(filename) < 0)//´´½¨ÁÙÊ±ÎÄ¼þ¼Ð
			{
				usleep(9000);
				continue;
			}
			
			g_record_control.record_flag = 1;
			
			for (i = 0; i < RECORD_NET_SOURCE; i++)
			{
				
				if ((OPEN == switch_flag[i]) && (NULL != pClient_handle[i]))
				{
					nslog(NS_INFO, "start.........i = %d\n", i);
					memset(&(pthread_param[i]), 0, sizeof(Pthread_Param_t));
					ret = fill_pthread_param(&(pthread_param[i]), filename, pClient_handle[i]);
					if (0 != ret)
					{
						nslog(NS_ERROR, "pClient_handle[i] is index=%d err!!\n", pClient_handle[i]->index);
						continue;
					}
					
					int record_val = -1;
					pthread_attr_t attr_record;
					pthread_attr_init(&attr_record);
					pthread_attr_setscope(&attr_record, PTHREAD_SCOPE_SYSTEM);
					pthread_attr_setdetachstate(&attr_record, PTHREAD_CREATE_JOINABLE);
					nslog(NS_INFO, "start.........pClient_handle = %s %p\n", pClient_handle[i]->ip, pClient_handle[i]->pListLock_Handle);
					record_val = pthread_create(&tid[i], NULL, record_process_thread, (void *)(&(pthread_param[i])));
					if (record_val)
					{
						nslog(NS_ERROR, "pthread_create is fail\n");
					}
					else
					{
						record_set_status(RECORD_RESUME);
						pthread_flag[i] = 1;
						nslog(NS_DEBUG, "pthread_create is suceess %d\n", i);
					}
			
					pthread_attr_destroy(&attr_record);
					
				}
				else
				{
					nslog(NS_ERROR,"index=%d switch_flag[i]=%d,pClient_handle[i]=%p!!\n",i ,switch_flag[i], pClient_handle[i]);//·µ»Ø´íÎóÂë
				}
			}
		}
		else if ((SIGNAL_START == record_signal) && (RECORD_PAUSE == record_status))
		{
			record_set_status(RECORD_RESUME);
		}
		
		if ((SIGNAL_CLOSE == record_signal) && (RECORD_CLOSE != record_status))
		{
			char path_xml[FILE_LEN] = {0};
			int i =0;
			g_record_control.record_flag = 0;
			record_set_status(RECORD_CLOSE);

			for (i = 0; i < RECORD_STREAM_MAX; i++)
			{
				if (AUDIO == i)
				{
					continue;
				}
				
				if (1 == pthread_flag[i])
				{
					pthread_join(tid[i], NULL);
				}
			}
	
			Record_CourseInfo_t course_info;
			memset(&course_info, 0, sizeof(Record_CourseInfo_t));
			
			get_global_recordInfo(&course_info);
			int time_tmp = 0;
			priv_get_recordTime(&time_tmp);
			switch_time(time_tmp, course_info.record_time);
			
			sleep(1);
			char dir_path[FILE_LEN] = {0};
			remove_tmp_dir(course_info.file_name, filename);

			sprintf(dir_path, "%s/%s", RECORD_PATH, course_info.file_name);
			
			course_info.total_size = get_dir_size(dir_path);
			ret = DB_insert_elem(course_info.file_name, course_info.notes, course_info.total_size, course_info.total_size, course_info.start_time, course_info.record_time,\
				course_info.main_teacher, course_info.course_subject);
			if (ret != 0)
			{
				nslog(NS_ERROR,"DB_insert_elem is err!!\n");
			}
			
			sprintf(path_xml, "%s/%s/%s.xml", RECORD_PATH, course_info.file_name, course_info.file_name);
			ret = record_store_courseInfo(course_info, path_xml);
			if (0 != ret)
			{
				nslog(NS_ERROR, "record_store_size is err!!\n");
			}

			char cmd[FILE_LEN] = {0};
			sprintf(cmd, "rm %s/%s/%s.xml_bak", RECORD_PATH, course_info.file_name, course_info.file_name);
			r_system((const int8_t *)(cmd));
			record_set_time(0);
			empty_global_recordInfo();
		}

		if ((SIGNAL_PAUSE == record_signal) && (RECORD_RESUME == record_status))
		{
			record_set_status(RECORD_PAUSE);
		}
	
		sleep(1);
	}

}


