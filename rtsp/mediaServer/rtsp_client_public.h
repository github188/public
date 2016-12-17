#ifndef RTSP_CLIENT_PUBLIC_
#define RTSP_CLIENT_PUBLIC_

#include "rtsp_base.h"
#ifdef __cplusplus
extern "C" {
#endif


rtspClient_Handle_t rtsp_client_start(const char *url, ClientStreamStatus  stateCallback, FrameCallBack frameCall, void* out_param,Rtsp_Control_t * control_param);

#ifdef __cplusplus
}
#endif

#endif
