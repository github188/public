#include "rtsp_client_public.h"
#include "pthread.h"
#include "rtsp_clientSession.h"

static void * evenLoopThread(void *argv)
{
	Rtsp_Client_Info_t* pClientInfo = (Rtsp_Client_Info_t*)argv;
	if(pClientInfo == NULL)
	{
		printf("usage_env == NULL is NULL\n");
		return NULL;
	}

	pClientInfo->usage_env->taskScheduler().doEventLoop(&(pClientInfo->m_quit));

	pthread_detach(pthread_self());
	pthread_exit(0);
}





int rtsp_client_connect(Rtsp_Client_Info_t *pClientInfo)
{
	//RTSPClient* rtspClient = NULL;
	if (NULL == pClientInfo)
	{

		printf("reconnect is fail\n");
		return -1;
	}

	/*
	if(pClientInfo->rtspClient != NULL && pClientInfo->status != RTSPCLIENT_FINISH)
	{
		return 0;
	}
	//连接完成，等待其动作全部完成，避免外部showdown后内部又showdown
	usleep(100000);
	if(pClientInfo->rtspClient != NULL)
	{
		shutdownStream(pClientInfo->rtspClient,0);
		pClientInfo->rtspClient = NULL;
	}
	
	rtspClient =  openURL(*(pClientInfo->usage_env), NULL, pClientInfo->url,  &(pClientInfo->registerInfo));
	if(rtspClient == NULL)
	{
		printf("pClientInfo->rtspClient is NULL\n");
		return -1;
	}
	*/
	return 0;
}


rtspClient_Handle_t rtsp_client_start(const char *url, ClientStreamStatus  stateCallback, FrameCallBack frameCall, void* out_param,Rtsp_Control_t * control_param)
{
	int error = 0;
	Rtsp_OutRegister_t registerInfo = {0};
	if (url == NULL || stateCallback== NULL || frameCall == NULL || control_param == NULL)
	{
		error = 1;
		printf("rtsp_client_start param is NULL url:%p stateCallback:%p frameCall:%p\n", url, stateCallback, frameCall);
		return NULL;
	}


	Rtsp_Client_Info_t* pClientInfo = (Rtsp_Client_Info_t*)malloc(sizeof(Rtsp_Client_Info_t));
	if (pClientInfo == NULL)
	{
		error = 1;
		printf("pClientInfo is NULL\n");
		goto EXIT_INIT;
	}
	memset(pClientInfo, 0, sizeof(Rtsp_Client_Info_t));

	registerInfo.frameCall = frameCall;
	registerInfo.param = out_param;
	registerInfo.stateCallback = stateCallback;
	//registerInfo.rtspStateCallback = rtspStateCallback;
	//registerInfo.rtsp_param = pClientInfo;
	memcpy(&(registerInfo.control_parm), control_param, sizeof(Rtsp_Control_t));
	memcpy(&(pClientInfo->registerInfo), &registerInfo, sizeof(Rtsp_OutRegister_t));
	strcpy(pClientInfo->url, url);

	pClientInfo->scheduler = BasicTaskScheduler::createNew();
	if(pClientInfo->scheduler == NULL)
	{
		error = 1;
		printf("pClientInfo->scheduler is NULL\n");
		goto EXIT_INIT;
	}

	pClientInfo->usage_env = BasicUsageEnvironment::createNew(*(pClientInfo->scheduler));
	if(pClientInfo->usage_env == NULL)
	{
		error = 1;
		printf("pClientInfo->usage_env is NULL\n");
		goto EXIT_INIT;
	}

	
	openURL(*(pClientInfo->usage_env), NULL, pClientInfo->url,  &(pClientInfo->registerInfo));

	error = pthread_create(&pClientInfo->loop_tid, NULL, evenLoopThread, pClientInfo);
	if(error != 0)
	{
		printf("pthread_create(&tid, NULL, doEvenLoopThread, pClientInfo); is fail\n");
		error = 1;
	}
#if 0
	// Run loop
	pthread_t tid;
	error = pthread_create(&tid, NULL, doClientConnect, pClientInfo);
	if(error != 0)
	{
		printf("pthread_create(&tid, NULL, doEvenLoopThread, pClientInfo); is fail\n");
		error = 1;
	}

	

	error = pthread_create(&pClientInfo->loop_tid, NULL, doClientEvenLoopThread, pClientInfo);
	if(error != 0)
	{
		printf("pthread_create(&tid, NULL, doEvenLoopThread, pClientInfo); is fail\n");
		error = 1;
	}
	error = pthread_create(&(pClientInfo->errmessege_tid), NULL, doClientPrintError, pClientInfo);
	if(error != 0)
	{
		printf("pthread_create(&tid, NULL, doEvenLoopThread, pClientInfo); is fail\n");
		error = 1;
	}
#endif


EXIT_INIT:
	if(error == 1)
	{
		if(pClientInfo->usage_env)
		{
			pClientInfo->usage_env->reclaim();
			pClientInfo->usage_env = NULL;
		}
		
		if(pClientInfo->scheduler)
		{
			delete pClientInfo->scheduler;
			pClientInfo->scheduler = NULL;
		}
	
		if(pClientInfo)
		{
			free(pClientInfo);
			pClientInfo = NULL;
		}
	
		return NULL;
	}
	
	return pClientInfo;
}




