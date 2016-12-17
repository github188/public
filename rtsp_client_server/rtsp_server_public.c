#include <stdio.h>
#include "socket_public.h"
#include "comm.h"


int rtsp_do_event (Rtsp_handle_t * rtsp_handle)
{
	if (NULL == rtsp_handle)
	{
		public_err("rtsp_do_event is err!!!\n");
		return -1;
	}
	
	Rtsp_CliConnection_t *client_info = NULL;
	struct timeval tv;
	fd_set rfds;
	fd_set wfds;
	int  maxfd;
	int ret = -1;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);

	FD_SET(rtsp_handle->sockfd, &rfds);

	maxfd = rtsp_handle->sockfd;
				
	for (client_info = rtsp_handle->connections_qhead->tqh_first; client_info; client_info = client_info->demo_entry.tqe_next)
	{
		struct rtsp_session *s = cc->session;
		struct rtp_connection *vrtp = cc->vrtp;
		struct rtp_connection *artp = cc->artp;

		FD_SET(cc->sockfd, &rfds);
		if (cc->sockfd > maxfd)
			maxfd = cc->sockfd;

		if (cc->state != RTSP_CC_STATE_PLAYING)
			continue;

		if (vrtp && streamq_inused(s->vstreamq, vrtp->streamq_index) > 0) {
            //add video rtp sock to wfds
			if (vrtp->is_over_tcp) {
				FD_SET(vrtp->tcp_sockfd, &wfds);
				if (vrtp->tcp_sockfd > maxfd)
					maxfd = vrtp->tcp_sockfd;
			} else {
				FD_SET(vrtp->udp_sockfd[0], &wfds);
				if (vrtp->udp_sockfd[0] > maxfd)
					maxfd = vrtp->udp_sockfd[0];
			}
		}

		if (artp && streamq_inused(s->astreamq, artp->streamq_index) > 0) {
            //add audio rtp sock to wfds
			if (artp->is_over_tcp) {
				FD_SET(artp->tcp_sockfd, &wfds);
				if (artp->tcp_sockfd > maxfd)
					maxfd = artp->tcp_sockfd;
			} else {
				FD_SET(artp->udp_sockfd[0], &wfds);
				if (artp->udp_sockfd[0] > maxfd)
					maxfd = artp->udp_sockfd[0];
			}
		}

		if (vrtp && (!vrtp->is_over_tcp)) {
			//add video rtcp sock to rfds
			FD_SET(vrtp->udp_sockfd[0], &rfds);
			FD_SET(vrtp->udp_sockfd[1], &rfds);
			if (vrtp->udp_sockfd[0] > maxfd)
				maxfd = vrtp->udp_sockfd[0];
			if (vrtp->udp_sockfd[1] > maxfd)
				maxfd = vrtp->udp_sockfd[1];
		}

		if (artp && (!artp->is_over_tcp)) {
			//add audio rtcp sock to rfds
			FD_SET(artp->udp_sockfd[0], &rfds);
			FD_SET(artp->udp_sockfd[1], &rfds);
			if (artp->udp_sockfd[0] > maxfd)
				maxfd = artp->udp_sockfd[0];
			if (artp->udp_sockfd[1] > maxfd)
				maxfd = artp->udp_sockfd[1];
		}
	}

	memset(&tv, 0, sizeof(tv));
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	ret = select(maxfd + 1, &rfds, &wfds, NULL, &tv);
	if (ret < 0) {
		err("select failed : %s\n", strerror(errno));
		return -1;
	}
	if (ret == 0) {
		return 0;
	}

	if (FD_ISSET(d->sockfd, &rfds)) {
		//new client_connection
		rtsp_new_client_connection(d);
	}

	cc = TAILQ_FIRST(&d->connections_qhead); //NOTE do not use TAILQ_FOREACH
	while (cc) {
		struct rtsp_client_connection *cc1 = cc;
		struct rtsp_session *s = cc1->session;
		struct rtp_connection *vrtp = cc1->vrtp;
		struct rtp_connection *artp = cc1->artp;
		cc = TAILQ_NEXT(cc, demo_entry);

		if (FD_ISSET(cc1->sockfd, &rfds)) {
			do {
				rtsp_msg_s reqmsg, resmsg;
				rtsp_msg_init(&reqmsg);
				rtsp_msg_init(&resmsg);

				ret = rtsp_recv_msg(cc1, &reqmsg);
				if (ret == 0)
					break;
				if (ret < 0) {
					rtsp_del_client_connection(cc1);
					cc1 = NULL;
					break;
				}

				if (reqmsg.type == RTSP_MSG_TYPE_INTERLEAVED) {
					//TODO process RTCP over TCP frame
					rtsp_msg_free(&reqmsg);
					continue;
				}

				if (reqmsg.type != RTSP_MSG_TYPE_REQUEST) {
					err("not request frame.\n");
					rtsp_msg_free(&reqmsg);
					continue;
				}

				ret = rtsp_process_request(cc1, &reqmsg, &resmsg);
				if (ret < 0) {
					err("request internal err\n");
				} else {
					rtsp_send_msg(cc1, &resmsg);
				}

				rtsp_msg_free(&reqmsg);
				rtsp_msg_free(&resmsg);
			} while (cc1);

			if (cc1 == NULL)
				continue;
		}

		if (cc1->state != RTSP_CC_STATE_PLAYING)
			continue;

		if (vrtp && streamq_inused(s->vstreamq, vrtp->streamq_index) > 0) {
            //send rtp video packet
			if (vrtp->is_over_tcp && FD_ISSET(vrtp->tcp_sockfd, &wfds)) {
				rtsp_tx_video_packet(cc1);
				//printf("v");fflush(stdout);
			} else if ((!vrtp->is_over_tcp) && FD_ISSET(vrtp->udp_sockfd[0], &wfds)) {
				rtsp_tx_video_packet(cc1);
				//printf("v");fflush(stdout);
			}
		}

		if (artp && streamq_inused(s->astreamq, artp->streamq_index) > 0) {
            //send rtp audio packet
			if (artp->is_over_tcp && FD_ISSET(artp->tcp_sockfd, &wfds)) {
				rtsp_tx_audio_packet(cc1);
				//printf("a");fflush(stdout);
			} else if (0 == artp->is_over_tcp && FD_ISSET(artp->udp_sockfd[0], &wfds)) {
				rtsp_tx_audio_packet(cc1);
				//printf("a");fflush(stdout);
			}
		}

		if (vrtp && (!vrtp->is_over_tcp)) {
			//process video rtcp socket
			if (FD_ISSET(vrtp->udp_sockfd[0], &rfds)) {
				rtsp_recv_rtp_over_udp(cc1, 0);
			}
			if (FD_ISSET(vrtp->udp_sockfd[1], &rfds)) {
				rtsp_recv_rtcp_over_udp(cc1, 0);
			}
		}
		if (artp && (!artp->is_over_tcp)) {
			//process audio rtcp socket
			if (FD_ISSET(artp->udp_sockfd[0], &rfds)) {
				rtsp_recv_rtp_over_udp(cc1, 1);
			}
			if (FD_ISSET(artp->udp_sockfd[1], &rfds)) {
				rtsp_recv_rtcp_over_udp(cc1, 1);
			}
		}
	}

	return 1;
}


int rtsp_init_tcpControl(int port)
{	
	struct sockaddr_in inaddr;
	int sockfd
	sockfd = socket(AF_INET, SOCK_STREAM, 0);//tcp
	if (sockfd == INVALID_SOCKET) 
	{
		public_err("create socket failed : %s\n", strerror(errno()));
		sockfd = -1;
		goto EXIT;
	}
	
	int ret = -1;
	memset(&inaddr, 0, sizeof(inaddr));
	inaddr.sin_family = AF_INET;
	inaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	inaddr.sin_port = htons(port);
	ret = bind(sockfd, (struct sockaddr*)&inaddr, sizeof(inaddr));
	if (-1 == ret)
	{
		public_err("bind socket to address failed : %s\n", strerror(errno()));
		close(sockfd);
		sockfd = -1;
		goto EXIT;
	}

	ret = listen(sockfd, 128);//内核中建立队列
	if (-1 == ret)
	{
		public_err("listen socket failed : %s\n", strerror(errno()));
		close(sockfd);
		sockfd = -1;
		goto EXIT;
	}
	
	public_info("rtsp server demo starting on port %d\n", port);
	
EXIT:	
	return sockfd;
}


int main(void)
{	
	int ret = -1;
	Rtsp_handle_t *rtsp_handle = (Rtsp_handle_t *)malloc(sizeof(Rtsp_handle_t));
	if (NULL == rtsp_handle)
	{
		public_err("malloc rtsp_handle is err!!!!\n");
		return -1;
	}

	memset(rtsp_handle, 0, sizeof(Rtsp_handle_t));
	rtsp_handle->sockfd = rtsp_init_tcpControl(554, rtsp_handle);
	if (3 >= rtsp_handle->sockfd)
	{
		public_err("rtsp_init_tcpControl is err!!!\n");
		return -1;
	}

	do
	{
		ret = rtsp_do_event(rtsp_handle);
		if (ret > 0)
			continue;
		if (ret < 0)
			break;
		usleep(20000);
	} while(1);
	
}


