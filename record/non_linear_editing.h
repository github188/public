#ifndef  NON_LINEAR_EDITING_H
#define NON_LINEAR_EDITING_H

int (*pWriteFrame_func)(MuxWriter * pMuxWriter,uint8_t * pBuf,int iBufLen,int64_t pts,int iStreamIdx,int bKeyFrame,int bAudio);

typedef struct _Editing_Data{
	int  flag;
	unsigned int start_time;
	unsigned long long prev_pts;
	unsigned int p_frame;
	unsigned int p_flag;
	unsigned int I_space;
	unsigned int prev_pauseVlpts;
	unsigned int cut_flag;//cut_flag == 1表示当前开始丢帧	
	unsigned int cut_cnt;
	unsigned int add_cnt;

}Editing_Data_T;

typedef struct _Audio_s{
	uint64_t a_ref_time; //first audio frame pts
	uint64_t a_last_time;
	int32_t a_interval_time;
	int32_t a_drop_count;
	int32_t a_add_count;
	uint64_t a_count;
	uint32_t a_pts_pause;	
	uint32_t a_pts;
	int32_t a_pts_cnt;
	uint32_t a_prev_pts;
	uint64_t count;//用于计数连续录制帧数
}priv_audio_t;


typedef struct _Sadd_Frame_T{
	int index;
	int streamID;
	int stream_type;// 1 -->HD  2-->SD
	unsigned int fps;
	unsigned int I_frame;
	uint32_t tmp_pts;
	uint32_t pause_Vpts;
	uint32_t dts;
	uint32_t Vlpts;
	unsigned char* pdata;
	unsigned int  date_len;	
	uint32_t frame_cnt;
	uint32_t frame_total;
	priv_audio_t priv_audioData;
	Editing_Data_T priv_editingData;
	MediaPacket Mp4_packet;
	MuxWriter* pWm;
	int pre_add;
	int pause_flag;
	int (*pWriteFrame_func)(MuxWriter * pMuxWriter,MediaPacket* mp4_packet);
}Add_Frame_T;

Add_Frame_T* nonLinearEditing_init(MuxWriter * pWm,void* func);
void nonLinearEditing_pause(Add_Frame_T*  add_frame_info);
void nonLinearEditing_deinit(Add_Frame_T**  add_frame_info);
void nonLinearEditing_set_prm(int sindex,int VideoStreamIdx,int Iframe,void* data,int data_len,unsigned int Vpts,int fps,Add_Frame_T*  add_frame_info);
int nonLinearEditing_process(Add_Frame_T * add_frame);
int nonLinearEditing_process_audio(const unsigned char* pdata,const unsigned int data_len,const int64_t real_pts,const int stream_id,Add_Frame_T *add_frame);

void nonLinearEditing_printf_info(Add_Frame_T *add_frame);

#endif
