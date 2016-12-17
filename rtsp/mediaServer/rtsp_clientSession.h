#ifndef _RTSP_CLIENTSESSION_
#define _RTSP_CLIENTSESSION_

#include "rtsp_base.h"
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

typedef struct
{
	TaskScheduler* scheduler;
	RTSPClient* rtspClient;
	UsageEnvironment* usage_env;
	Rtsp_OutRegister_t registerInfo;
	Rtsp_Status_t status;
	pthread_t errmessege_tid;
	pthread_t loop_tid;
	int isreconect;
	char url[256];
	int nUse;
	int port;
	char m_quit;

}Rtsp_Client_Info_t;



void openURL(UsageEnvironment& env, char const* progName, char const* rtspURL, Rtsp_OutRegister_t* registerInfo);

#endif
