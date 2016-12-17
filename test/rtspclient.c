#include <stdio.h>
#include "rtsp_client_public.h"
typedef struct CAM_INFO_
{
	char ip[16];
	int port;
	char channel[48];
}Cam_Info_t;

int status(Rtsp_ClientStream_State_t * param)
{
	printf("hehheheheh\n");
	
}

FILE *fp = NULL;


int frame_func(Fream_Info_t* frame)
{
	if (frame->type == 0)
	{
		fwrite(frame->data, frame->frameSize, 1, fp);
		//printf("frame->height=%d frame->width=%d frame->frameSize=%d\n", frame->height, frame->width,  frame->frameSize);
	}
	return 0;
}


int main(void)
{
	
	int chanel = 0;
	
	char url1[128] = "rtsp://192.168.28.87:554";
	fp = fopen("./hehe.h264", "w");
	Rtsp_Control_t param;
	param.turnAudio = 1;
	rtsp_client_start(url1, status, frame_func, (void *)&chanel, &param);
	
	//while(1)
	{
		sleep(2);
		char ch = getchar();
		if (ch == 'q')
		{
			fclose(fp);
		}
	}
	return 0;
}
