#ifndef	__STREAM_PUBLIC_H__
#define	__STREAM_PUBLIC_H__

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <ctype.h>

#include "socket_public.h"
/*
author : ydl	   2016.01.11
*/

/*audio and video frame header */
#define DEFAULT_CHECK_START_CODE '$'
#define DEFAULT_CHECK_END_CODE   '#'
#define IP_LEN (16)
#define	RH_MAX_STREAMS		(16)
#define	RH_MAX_CLIENTS	(6)


typedef struct RH_FRAMEHEAD_ {
	unsigned char  check_start[4]; /*,标示符*/
	unsigned int ID;							//=mmioFOURCC('4','D','S','P');
	unsigned int nChannel;
	unsigned int nPTimeTick;						//time
	unsigned int nDTimeTick;						//time
	unsigned int nFrameLength;					//length of frame
	unsigned int nDataCodec;					//encode type
	unsigned int s_blue;						// 1 blue， 0 no
	
	union {
		unsigned int nFrameRate;				//video:framerate
		unsigned int nSamplerate;					//aduio:samplerate (default:44100)
	};
	union {
		unsigned int nWidth;						//video:width
		unsigned int nAudioChannel; 				//audio:channel (default:2)
	};
	union {
		unsigned int nHight;						//video:height
		unsigned int nSamplebit;					//audio:samplebit (default:16)
	};
	union {
		unsigned int nColors;						//video:colors
		unsigned int nBandwidth;				//audio:bandwidth (default:64000)
	};
	union {
		unsigned int nIframe;					//video: I frame
		unsigned int nReserve;					//audio:  reserve
	};

	unsigned int nPacketNumber; 					//packages serial number
	unsigned int nOthers;							//reserve
	unsigned char  check_end[4]; /*,标示符*/
} Stream_Head_t;

typedef struct StreamServer__ {
	int	sindex;
	char ip[IP_LEN];
	int port;
} Stream_Server_t;

typedef struct tcp_stream_recv_cond_ {
	char	ip[IP_LEN]; //接收端ip地址
	short	port;        //接收端端口
	int		timeout ;// timeout -1 不需要设置
	void		*arg;
	void * (*getEmptyBuf)(void *arg, void **ppEmptyBufInfo, int data_len);
	int (*putStreaminfo)(Stream_Head_t *fh, void *pEmptyBufInfo, void *arg, void *data);
	int (*exception_msg)(int msg,void* prm);
} Tcp_Stream_Recv_t;

void stream_init_headType(Stream_Head_t *head);
void *Stream_init_client(Tcp_Stream_Recv_t * inParm);
int stream_init_server(Stream_Server_t * arg);
void stream_send_dataToClient(int sindex, int nLen, unsigned char *pData, Stream_Head_t *fh);


#ifdef __cplusplus
}
#endif

#endif

