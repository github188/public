/*************************************************************************
Author: 459336407@qq.com 
date:2016.12.16
 ************************************************************************/

#ifndef __RTP_ENC_H__
#define __RTP_ENC_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __rtp_enc 
{
	uint8_t  pt;
	uint16_t seq;
	uint32_t ssrc;
	uint32_t sample_rate;
}Rtp_Enc_t;


typedef struct rtp_connection
{
	int is_over_tcp;
	int tcp_sockfd; //if is_over_tcp=1. rtsp socket
	int tcp_interleaved[2];//if is_over_tcp=1. [0] is rtp interleaved, [1] is rtcp interleaved
    int udp_sockfd[2]; //if is_over_tcp=0. [0] is rtp socket, [1] is rtcp socket
	uint16_t udp_localport[2]; //if is_over_tcp=0. [0] is rtp local port, [1] is rtcp local port
	uint16_t udp_peerport[2]; //if is_over_tcp=0. [0] is rtp peer port, [1] is rtcp peer port
	struct in_addr peer_addr; //peer ipv4 addr
	int streamq_index;
	uint32_t ssrc;
	uint32_t rtcp_packet_count;
	uint32_t rtcp_octet_count;
	uint64_t rtcp_last_ts;
}Rtp_Connection_t;



#ifdef __cplusplus
}
#endif
#endif

