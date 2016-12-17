#ifndef RTSP_BASE_H_
#define RTSP_BASE_H_
#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
	RTSPCLIENT_START = 0,
	RTSPCLIENT_PAUSE,
	RTSPCLIENT_STOP,
	RTSPCLIENT_FINISH,
}Rtsp_Status_t;

typedef enum
{
	VIDEO_TYPE = 0,
	AUDIO_TYPE,

}Frame_Type_t;

typedef struct fream_Info
{
	int frameSize;
	int width;
	int height;
	int fps;
	unsigned char *data;
	int iFrame;
	Frame_Type_t type;
	void *param;//���û���ʼ�����Լ�����ȥ�ı�ʶ
}Fream_Info_t;

typedef struct
{
	int turnAudio;
}Rtsp_Control_t;//��Ҫ������չ


typedef struct Rtsp_Client_State_Info
{
	Rtsp_Status_t status;
	void *param;//���û��Լ�����ȥ��ʶ
}Rtsp_ClientStream_State_t;



typedef void * rtspClient_Handle_t;
typedef int (*ClientStreamStatus)(Rtsp_ClientStream_State_t * param);
typedef int (*RtspClientStreamStatus)(Rtsp_ClientStream_State_t * param, void* rtspdata);
typedef int (*FrameCallBack)(Fream_Info_t* frame);


typedef struct rtsp_ourregister
{
	ClientStreamStatus  stateCallback;
	RtspClientStreamStatus  rtspStateCallback;
	FrameCallBack frameCall;
	Rtsp_Control_t control_parm;
	//void* rtsp_param;
	void* param;
}Rtsp_OutRegister_t;

#ifdef __cplusplus
}
#endif

#endif
