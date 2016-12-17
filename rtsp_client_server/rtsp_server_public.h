/*************************************************************************
File Name: rtsp_server_public.h
Author: 459336407@qq.com
date: 2016.12.16
 ************************************************************************/

#ifndef __RTSP_SERVER_PUBLIC_H__
#define __RTSP_SERVER_PUBLIC_H__
/*
 * a simple RTSP server demo
 * RTP over UDP/TCP H264/G711a 
 * */

#include "stdint.h"
#include "rtp_enc.h"

#ifdef __cplusplus
extern "C" {
#endif

struct rtsp_handle_ ;
struct rtsp_clicon_queue_head;
struct rtsp_session_queue_head;
struct rtsp_client_connection;
struct rtsp_session_;

typedef struct rtsp_session_q
{								
	struct rtsp_session_ *tqe_next;		/* next element */		
	struct rtsp_session_ **tqe_prev;	/* address of previous next element */
}Rtsp_Session_Pq_t;

typedef struct stream_queue_
{
	int pktsiz;
	int nbpkts;
	int head;
	int tail;
	int *pktlen;
	char *buf;
}Stream_Queue_t;

typedef struct rtsp_session_
{
	char path[64];
	int  vcodec_id;
	int  acodec_id;

	union 
	{
		Code_DataH264_t h264;
		Code_DataH265_t h265;
	}vcodec_data;

	union
	{
		Code_DataG726_t g726;
		Codec_DataAac_t aac;
	} acodec_data;

	Rtp_Enc_t vrtpe;
	Rtp_Enc_t artpe;
	Stream_Queue_t *vstreamq;
	Stream_Queue_t *astreamq;

	uint64_t video_ntptime_of_zero_ts;
	uint64_t audio_ntptime_of_zero_ts;
	
	struct rtsp_handle_ *rtsp_handle;
	struct rtsp_clicon_queue_head connections_qhead;
	Rtsp_Session_Pq_t pq_entry;
}Rtsp_Session_t;

typedef struct rtsp_client_connection
{
	int state;	//session state
	#define RTSP_CC_STATE_INIT		0
	#define RTSP_CC_STATE_READY		1
	#define RTSP_CC_STATE_PLAYING	2
	#define RTSP_CC_STATE_RECORDING	3

	int sockfd;		//rtsp client socket
	struct in_addr peer_addr; //peer ipv4 addr
	unsigned long session_id;	//session id

	char reqbuf[1024];
	int  reqlen;

	Rtp_Connection_t *vrtp;
	Rtp_Connection_t *artp;

	struct rtsp_handle_ *demo;
	struct rtsp_session_queue_head *session;
	struct rtsp_client_connection demo_entry; 
	struct rtsp_client_connection session_entry;
}Rtsp_CliConnection_t;

typedef struct rtsp_session_queue_head
{					
	Rtsp_Session_t *tqh_first;		/* first element */		
	Rtsp_Session_t **tqh_last;	/* addr of last next element */
}Rtsp_Session_Head_t;

typedef struct rtsp_clicon_queue_head
{					
	Rtsp_Session_t *tqh_first;		/* first element */		
	Rtsp_Session_t **tqh_last;	/* addr of last next element */
}Rtsp_CliCon_head_t;

typedef struct rtsp_handle_ 
{
	int sockfd;	//rtsp server socket 0:invalid
	Rtsp_Session_Head_t sessions_qhead;
	Rtsp_CliCon_head_t connections_qhead;
}Rtsp_handle_t;



typedef void * rtsp_socket_handle;


#ifdef __cplusplus
}
#endif
#endif

