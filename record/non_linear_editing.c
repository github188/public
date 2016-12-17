
#include "record_main.h"
//#include "media_msg.h"
#include "dirmd5.h"
#include "Media.h"
#include "nslog.h"
#include "non_linear_editing.h"


#define MAX_RECORD_NUM  		10
#define  RECORD_FPS_30			3//2997
#define  RECORD_FPS_25			2500
#define  AUDIO_SAMPLE_RATE		(44.1)
#define  RECORD_GOP				25
#define  FRAME_BUFFER_MAX_NUM	3
#define  FRAME_MAX_LEN			(1920*1080*2)
#define  VOID_NODE				(-1)
#define AUDIO_PLAY_TIME(x)		((x)*10240/441)
#define  AUDIO_FRMAE_TIME		(23.219)	//44.1 per 1frame
#define VIDEO_PLAY_TIME(x,t)		((x)*(t))
#define AUDIO_HEAD_FRAME		(5)

#define  SOBY_EDITING_CMD		("sobyEditing=1")
#define  SOBY_EDITING_CMD_NO	("sobyEditing=0")




typedef struct NONLINEAREDITING_FRAME_LIST {
	int node;
	int Iframe;
	unsigned int frame_len;
	unsigned char *pFrame_data;
	Add_Frame_T *pFrame_info;
} Frame_List;

//static Frame_List gFrame_list[FRAME_BUFFER_MAX_NUM];

void nonLinearEditing_printf_info(Add_Frame_T *add_frame)
{
	if (!add_frame)
	{
		return;
	}

	int64_t a_std_time = 0, video_time = 0;
	priv_audio_t *audio = &(add_frame->priv_audioData);
	Editing_Data_T *pEditing = &(add_frame->priv_editingData);
	a_std_time = (int64_t)(audio->a_pts);

	if (add_frame->fps != 0)
	{
		video_time = (add_frame->frame_cnt * 1000) / add_frame->fps;
	}

	nslog(NS_INFO, "--------------------index=%d-----------------------------\n", add_frame->index);
	nslog(NS_INFO, "|-Audio:<acount> -< a_drop_count >-<a_add_count>-< V_Pts>, <A_Pts >-|\n");
	nslog(NS_INFO, "|-Audio:<%lld>----<%d>-----------------<%d>--------<%d>------<%d>-|\n", audio->a_count, audio->a_drop_count,
	      audio->a_add_count, add_frame->Vlpts, audio->a_pts);
	nslog(NS_INFO, "|-Video:<frame_cnt>-<V_pts>--<cut_frame>-<add_frame>--<Fps>----<A/V>---<FrameRate>|\n");
	nslog(NS_INFO, "|-Video:--<%d>------<%d>-----<%d>-----------<%d>------<%0.3f>---<%lld>-------<%d>----|\n", add_frame->frame_cnt, add_frame->Vlpts,  pEditing->cut_cnt,
	      pEditing->add_cnt, (float)(add_frame->frame_cnt * 1000.0) / add_frame->Vlpts, video_time,add_frame->fps);

}

void nonLinearEditing_deinit(Add_Frame_T  **add_frame_info)
{
	if(*add_frame_info != NULL)
	{
		//	Add_Frame_T* p=*add_frame_info;
		nslog(NS_INFO, "add_frame_info=%p\n", *add_frame_info);
		/*
		if( add_frame_info->priv_editingData != NULL){
			free(add_frame_info->priv_editingData);
			add_frame_info->priv_editingData=NULL;
		}*/
		free(*add_frame_info);
		*add_frame_info = NULL;
	}
}

Add_Frame_T *nonLinearEditing_init(MuxWriter *pWm, void *func)
{
	Add_Frame_T *add_frame_info = NULL;
	add_frame_info = (Add_Frame_T *) r_malloc(sizeof(Add_Frame_T));

	if (NULL == add_frame_info)
	{
		return NULL;
	}

	memset(add_frame_info, 0, sizeof(Add_Frame_T));
	memset(&(add_frame_info->priv_editingData), 0, sizeof(Editing_Data_T));
	memset(&(add_frame_info->priv_audioData), 0, sizeof(priv_audio_t));
	add_frame_info->priv_audioData.a_ref_time = 0;
	add_frame_info->priv_audioData.a_pts = 0;
	add_frame_info->priv_audioData.a_pts_pause = 0;
	add_frame_info->priv_audioData.a_pts_cnt = AUDIO_HEAD_FRAME;
	add_frame_info->date_len = 0 ;
	add_frame_info->fps = 0;
	add_frame_info->index = 0;
	add_frame_info->Vlpts = 0;
	add_frame_info->tmp_pts = 0;
	add_frame_info->streamID = 0;
	add_frame_info->frame_cnt++;
	add_frame_info->pause_Vpts = 0;
	add_frame_info->I_frame = 0;
	add_frame_info->dts = 0;
	add_frame_info->pWm = pWm;
	add_frame_info->pre_add = 0;
	add_frame_info->pause_flag = 0;
	add_frame_info->frame_total = 0;
	add_frame_info->pWriteFrame_func = func;
	return add_frame_info;
}

static int mp4_video_cut_frame(Add_Frame_T *cut_frame)
{
	if (NULL == cut_frame)
	{
		nslog(NS_WARN, "Warning---cut_frame == NULL\n");
		return 0;
	}

	Editing_Data_T *pEditing = &(cut_frame->priv_editingData);

	if (NULL == pEditing)
	{
		nslog(NS_WARN, "Warning---pEditing == NULL\n");
		return 0;
	}

	unsigned int cut_cnt = (pEditing->flag);

	if (1 == cut_frame->I_frame)
	{
		pEditing->p_frame = 0;
		pEditing->p_flag = 0;
	
		return 0;
	} 

	if (cut_cnt > 3)
	{
		cut_cnt = 3;
	}
	
	if (pEditing->p_frame > 0 && pEditing->p_flag < 3)
	{
		if (pEditing->p_frame > cut_frame->fps - cut_cnt - 1)
		{
			//cut_frame->frame_cnt--;?
			pEditing ->cut_flag = 1;
			pEditing->p_flag++;
			pEditing->cut_cnt += 2; //一次跳2frame
			nonLinearEditing_printf_info(cut_frame);
			nslog(NS_WARN, "index=%d-cut_cnt=%d-!", cut_frame->index, pEditing->cut_cnt);
			return -1;
		}
	}
	
	return 0;
}

static int mp4_video_add_frame(Add_Frame_T *add_frame)
{
	if (NULL == add_frame)
	{
		nslog(NS_WARN, "Warning---add_frame == NULL\n");
		return 0;
	}

	Editing_Data_T *pEditing = &(add_frame->priv_editingData);

	if (NULL == pEditing)
	{
		nslog(NS_WARN, "Warning---pEditing == NULL\n");
		return 0;
	}

	unsigned int cnt = -(pEditing->flag);
	int i = 0 ;

	if (0 == add_frame->I_frame)
	{
		return 0;
	}

	if (pEditing->flag >= 0)
	{
		nslog(NS_INFO, "--------Enter Error flag=%d\n", pEditing->flag);
		return 0;
	}

	for (i = 0; i < cnt; i++)
	{
		//i <2 ,2帧
		if (i > 1)
		{
			nslog(NS_INFO, "---index=%d MAx add  3 Frame cnt =%d\n", add_frame->index,-(pEditing->flag));
			break;
		}

		pEditing->I_space = 1;
		pEditing->flag++;
		//pts 回去的问题?
		add_frame->tmp_pts += 5 * (i);

		add_frame->frame_cnt++;
		add_frame->frame_total++;
		//check pts;
		if (pEditing->prev_pts >= add_frame->tmp_pts)
		{
			nslog(NS_INFO, "---add_frame->priv_pts=%lld,=%d\n", pEditing->prev_pts, add_frame->tmp_pts);
			add_frame->tmp_pts = 5 + pEditing->prev_pts;
		}

		pEditing->prev_pts = add_frame->tmp_pts;
		
		pEditing->add_cnt++;
		nslog(NS_INFO, "---index = %d-add_cnt=%d-\n", add_frame->index, pEditing->add_cnt);
		nonLinearEditing_printf_info(add_frame);

		add_frame->Mp4_packet.bAudio = 0;
		add_frame->Mp4_packet.pts = (int64_t)add_frame->Vlpts;
		add_frame->Mp4_packet.dts = (int64_t)add_frame->Vlpts;
		add_frame->Mp4_packet.bKeyFrame = add_frame->I_frame;
		add_frame->Mp4_packet.pData = add_frame->pdata;
		add_frame->Mp4_packet.iLen = add_frame->date_len;
		add_frame->Mp4_packet.iStreamIdx = add_frame->streamID;
		if (add_frame->pWriteFrame_func(add_frame->pWm, &(add_frame->Mp4_packet)) < 0)
		{
			nslog(NS_WARN, "video MediaWriteFrame is failed:[%s]",  strerror(errno));
			break;
		}

		if (25 == add_frame->fps)
		{
			add_frame->Vlpts += 40;
		}
		else
		{
			add_frame->Vlpts = add_frame->frame_total * 100 / RECORD_FPS_30;//add_frame->tmp_pts;
		}
	}
	
	add_frame->tmp_pts += 5;
	return 0;
}

void nonLinearEditing_pause(Add_Frame_T  *add_frame_info)
{
	if(NULL == add_frame_info) {
		nslog(NS_WARN, "Warning---add_frame_info == NULL\n");
		return ;
	}

	Editing_Data_T *pEditing = &(add_frame_info->priv_editingData);

	if(NULL  == add_frame_info || NULL == pEditing) {
		nslog(NS_WARN, "Warning---add_frame_info == NULL\n");
		return ;
	}

	pEditing->start_time = 0;
	add_frame_info->tmp_pts = 0 ;

	if((int32_t)(add_frame_info->priv_audioData.a_pts) > 0) {
		add_frame_info->priv_audioData.a_pts_pause = add_frame_info->priv_audioData.a_pts;
		add_frame_info->priv_audioData.a_pts = 0;
	}

	add_frame_info->pause_Vpts = add_frame_info->Vlpts;
	add_frame_info->priv_audioData.a_ref_time = 0;
}

void nonLinearEditing_set_prm(int index, int VideoStreamIdx, int Iframe, void *data, int data_len, unsigned int Vpts,int fps, Add_Frame_T  *add_frame_info)
{
	if(NULL == add_frame_info) {
		nslog(NS_WARN, "Warning---add_frame_info == NULL\n");
		return ;
	}

	add_frame_info->index = index;
	add_frame_info->streamID = VideoStreamIdx;
	add_frame_info->I_frame = Iframe;
	add_frame_info->pdata = data;
	add_frame_info->date_len = data_len;
	add_frame_info->tmp_pts = Vpts;
	add_frame_info->fps = fps;
}

int nonLinearEditing_process_audio(const unsigned char *pdata, const unsigned int data_len, const int64_t real_pts, const int stream_id, Add_Frame_T *add_frame)
{
	if (NULL == add_frame || !pdata)
	{
		nslog(NS_WARN, "Warning---add_frame == NULL\n");
		return -1;
	}
	
	
	priv_audio_t *audio = &(add_frame->priv_audioData);
	uint64_t a_std_cnt = 0;
	uint64_t time_out = 0;

	if (!audio)
	{
		nslog(NS_WARN, "Warning--index=%d-audio == %p\n", add_frame->index, audio);
		return -1;
	}

	if (audio->a_count >= 0xFFFFFFFE)
	{
		audio->a_count = 0;
		audio->count = 0;
		//nslog(NS_ERROR, "audio->a_count is %lld audio->count\n",audio->a_count );
	}

	if (audio->a_ref_time == 0)
	{
		audio->a_ref_time = real_pts;
		audio->a_last_time = 0;
		audio->a_drop_count = 0;
		audio->a_add_count = 0;
		audio->a_pts = 0;

		audio->a_pts_cnt--;//5--
		audio->a_pts = (int64_t)(audio->a_count * 10240 / 80);//只是用来计算pts

		add_frame->Mp4_packet.bAudio = 1;
		add_frame->Mp4_packet.bKeyFrame = add_frame->I_frame;
		add_frame->Mp4_packet.dts = (int64_t)audio->a_pts;
		add_frame->Mp4_packet.pts = (int64_t)audio->a_pts;
		add_frame->Mp4_packet.iLen = data_len;
		add_frame->Mp4_packet.pData = (uint8_t *)pdata;
		add_frame->Mp4_packet.iStreamIdx = stream_id;

		add_frame->pWriteFrame_func(add_frame->pWm, &(add_frame->Mp4_packet));
		audio->a_count++;
		audio->count++;
		audio->a_add_count++;
		//nonLinearEditing_printf_info(add_frame);

	}

	if (audio->a_pts_cnt > 0 && (real_pts - audio->a_last_time) > 1000)
	{
		//nslog(NS_ERROR, "audio->a_pts_cnt:%lld real_pts - audio->a_last_time:%lld\n", audio->a_pts_cnt, real_pts - audio->a_last_time);
		
		audio->a_pts = (int64_t)(audio->a_count * 10240 / 80);

		add_frame->Mp4_packet.bAudio = 1;
		add_frame->Mp4_packet.bKeyFrame = add_frame->I_frame;
		add_frame->Mp4_packet.dts = (int64_t)audio->a_pts;
		add_frame->Mp4_packet.pts = (int64_t)audio->a_pts;
		add_frame->Mp4_packet.iLen = data_len;
		add_frame->Mp4_packet.pData = (uint8_t *)pdata;
		add_frame->Mp4_packet.iStreamIdx = stream_id;
		add_frame->pWriteFrame_func(add_frame->pWm, &(add_frame->Mp4_packet));
		//nonLinearEditing_printf_info(add_frame);
		audio->a_last_time = real_pts;
		audio->a_pts_cnt--;
		audio->a_count++;
		audio->count++;
		audio->a_add_count++;
	}
	
	//a_std_cnt = (uint64_t)real_pts * 441 / 10240;//应该获取到的音频数//把timeout 变成0 real_pts-audio->a_ref_time   a_count=0
	time_out = real_pts - audio->a_ref_time;
	/*
	if (audio->count % 40 == 0)
	{
		nslog(NS_ERROR, "real_pts=%d audio->a_ref_time=%d\n", real_pts, audio->a_ref_time);
	}*/
	
	a_std_cnt = time_out * 80 / 10240;
	if (0 == time_out)
	{
		nslog(NS_ERROR, "the a_std_cnt =%lld audio->count=%lld   index=%d\n", a_std_cnt, audio->count, add_frame->index);
	}

	if (a_std_cnt  > (audio->count + 1))
	{
		
		if ((real_pts - audio->a_last_time) > 3000)//音频补帧
		{
			nslog(NS_INFO,"Add-->a_std_cnt=%lld  a_interval_time=%lld  audio->a_count+1=%lld  \n",a_std_cnt,real_pts,( audio->a_count+1));
			audio->a_last_time = real_pts;
			
			audio->a_pts = (uint64_t)(audio->a_count * 10240 / 80);
			add_frame->Mp4_packet.bAudio = 1;
			add_frame->Mp4_packet.bKeyFrame = add_frame->I_frame;
			add_frame->Mp4_packet.dts = (int64_t)audio->a_pts;
			add_frame->Mp4_packet.pts = (int64_t)audio->a_pts;
			add_frame->Mp4_packet.iLen = data_len;
			add_frame->Mp4_packet.pData = (uint8_t *)pdata;
			add_frame->Mp4_packet.iStreamIdx = stream_id;
			add_frame->pWriteFrame_func(add_frame->pWm, &(add_frame->Mp4_packet));
			audio->a_count++;
			audio->count++;
			audio->a_add_count++;

			audio->a_pts = (uint64_t)(audio->a_count * 10240 / 80);
			add_frame->Mp4_packet.bAudio = 1;
			add_frame->Mp4_packet.bKeyFrame = add_frame->I_frame;
			add_frame->Mp4_packet.dts = (int64_t)audio->a_pts;
			add_frame->Mp4_packet.pts = (int64_t)audio->a_pts;
			add_frame->Mp4_packet.iLen = data_len;
			add_frame->Mp4_packet.pData = (uint8_t *) pdata;
			add_frame->Mp4_packet.iStreamIdx = stream_id;
			add_frame->pWriteFrame_func(add_frame->pWm, &(add_frame->Mp4_packet));
			audio->a_count++;
			audio->count++;
			audio->a_add_count++;
			//nonLinearEditing_printf_info(add_frame);
			return 1;
		}
	} 
	else if(a_std_cnt  < (audio->count + 1)  && (int32_t)((audio->count + 1) - a_std_cnt) > 4 + AUDIO_HEAD_FRAME)
	{
		nslog(NS_INFO,"Cut-->a_std_cnt=%d  a_interval_time=%d  audio->a_count+1=%d \n",a_std_cnt,real_pts,( audio->a_count+1));
		if ((real_pts - audio->a_last_time) > 3000)
		{
			audio->a_last_time = 0;
			audio->a_drop_count++;
			//nonLinearEditing_printf_info(add_frame);
			return 0;
		}
	}

	audio->a_pts = (int64_t)(audio->a_count * 10240 / 80);
	add_frame->Mp4_packet.bAudio = 1;
	add_frame->Mp4_packet.bKeyFrame = add_frame->I_frame;
	add_frame->Mp4_packet.dts = (int64_t)audio->a_pts;
	add_frame->Mp4_packet.pts = (int64_t)audio->a_pts;
	add_frame->Mp4_packet.iLen = data_len;
	add_frame->Mp4_packet.pData = (uint8_t *) pdata;
	add_frame->Mp4_packet.iStreamIdx = stream_id;
	add_frame->pWriteFrame_func(add_frame->pWm, &(add_frame->Mp4_packet));
	audio->a_count++;
	audio->count++;
	return 0;
}

int nonLinearEditing_process(Add_Frame_T *add_frame)
{
	if (NULL == add_frame)
	{
		nslog(NS_WARN, "Warning---add_frame == NULL\n");
		return 0;
	}


	Editing_Data_T *pEditing = &(add_frame->priv_editingData);
	if (NULL == pEditing)
	{
		nslog(NS_ERROR, "pEditing=%p!\n", pEditing);
		return 0;
	}

	uint64_t std_frame_cnt = 0;
	uint32_t timeout = 0;
	int ret = 0;

	if (0 == pEditing->start_time)
	{
		add_frame->frame_cnt = 0;
		pEditing->start_time = add_frame->tmp_pts;
		nonLinearEditing_printf_info(add_frame);
	}

	add_frame->frame_cnt++;//用于统计帧的个数
	add_frame->frame_total++;//用于30fps
	if (add_frame->date_len == 0)
	{
		nslog(NS_ERROR, "index=%d  FPS=%d--->ERROR!\n", add_frame->index,add_frame->fps);
		return 0;
	}
	/*
	if (pEditing->prev_pauseVlpts != add_frame->pause_Vpts)
	{
		add_frame->Vlpts = add_frame->pause_Vpts;
		pEditing->prev_pauseVlpts = add_frame->pause_Vpts;
		nslog(NS_INFO, "prev_pauseVlpts=%d--pause_Vpts=%d------>Warining!\n", pEditing->prev_pauseVlpts, add_frame->pause_Vpts);
	}
	*/

	timeout = add_frame->tmp_pts - pEditing->start_time;//第一次是0
	
	if (1 == add_frame->I_frame)
	{
		pEditing->p_frame = 0;
		pEditing ->cut_flag = 0;//I帧不能抛
	}
	
	pEditing->p_frame++;
	
	if (1 == pEditing->cut_flag)
	{
		ret = -1;
		//add_frame->frame_cnt --;?//应该不能减
		
		nslog(NS_WARN, "need cut p frame,add_frame->frame_cnt=%d\n", add_frame->frame_cnt);
	}
	else if ((timeout > 60000) && (add_frame->pause_flag == 0))//前一分钟不进这个逻辑
	{
		if (25 == add_frame->fps)
		{
			std_frame_cnt = (timeout) / (40);
		} 
		else if (30 == add_frame->fps)
		{
			std_frame_cnt = ((uint64_t)(RECORD_FPS_30 * timeout)) / (100);
		}

		if (add_frame->frame_cnt  > std_frame_cnt + 1)
		{
			pEditing->flag = add_frame->frame_cnt - std_frame_cnt - 1;
		}
		else if(add_frame->frame_cnt  < std_frame_cnt - 1)
		{
			pEditing->flag = add_frame->frame_cnt - std_frame_cnt + 1 ;
		}
		
		if (pEditing->flag < -4)
		{	
			nslog(NS_ERROR, "index=%d we need add pEditing->flag:%d %d %d %lld\n",add_frame->index, -pEditing->flag, add_frame->pre_add, add_frame->frame_cnt, std_frame_cnt);
			ret = mp4_video_add_frame(add_frame);
			if ((add_frame->pre_add < (-pEditing->flag)) && ((add_frame->index == 0) || (add_frame->index == 1)))//上次需要加的帧数小于本次实际需要加的帧数
			{
				nslog(NS_ERROR, " modify pts %d %d\n", add_frame->pre_add, -pEditing->flag);
				add_frame->Vlpts += 5 * 40;
				add_frame->frame_cnt += 5;
			}
			
			add_frame->pre_add = -(pEditing->flag);
		}
		else if(pEditing->flag > 4)
		{
			nslog(NS_ERROR, "index=%d we need cut pEditing->flag:%d std_frame_cnt=%d add_frame->frame=%d\n", add_frame->index,-pEditing->flag, add_frame->frame_cnt);
			ret = mp4_video_cut_frame(add_frame);
		}

	}

	if (0 == ret)
	{
		//check pts;
		if (pEditing->prev_pts >= add_frame->tmp_pts)
		{
			//nslog(NS_INFO, "---add_frame->priv_pts=%lld,=%d\n", pEditing->prev_pts, add_frame->tmp_pts);
			add_frame->tmp_pts = 5 + pEditing->prev_pts;
		}

		pEditing->prev_pts = add_frame->tmp_pts;
		
		add_frame->Mp4_packet.bAudio = 0;
		add_frame->Mp4_packet.pts = (int64_t)add_frame->Vlpts;
		add_frame->Mp4_packet.dts = (int64_t)add_frame->Vlpts;
		add_frame->Mp4_packet.bKeyFrame = add_frame->I_frame;
		add_frame->Mp4_packet.pData = add_frame->pdata;
		add_frame->Mp4_packet.iLen = add_frame->date_len;
		add_frame->Mp4_packet.iStreamIdx = add_frame->streamID;

		if (add_frame->pWriteFrame_func(add_frame->pWm, &(add_frame->Mp4_packet)) < 0)
		{
			nslog(NS_WARN, "-%d--video MediaWriteFrame is failed:[%lld--%d]total=%d\n", add_frame->index, add_frame->Mp4_packet.dts, add_frame->Vlpts, add_frame->frame_total);
		}

		if (25 == add_frame->fps)
		{
			add_frame->Vlpts += 40;
		}
		else
		{
			add_frame->Vlpts = (add_frame->frame_total) * 100 / RECORD_FPS_30;//add_frame->tmp_pts;
		}

		return 0;
	}

	return ret;
}

