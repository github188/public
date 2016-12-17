#ifndef	__MEDIA_MSG_H__
#define	__MEDIA_MSG_H__
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "stdint.h"
#include <stdint.h>
#include <errno.h>
#include "nslog.h"

typedef struct parse_datas
{
	int32_t index;//��ʦid
	int32_t data_len;//֡����
	uint8_t *data;
	int32_t flags;//�Ƿ�i֡
	int32_t sindex;//�ڼ�·
	int32_t audio_sindex;
	int32_t data_type;//�������ͣ�����Ƶ
	int32_t height;
	int32_t width;
	int32_t sample_rate;
	uint32_t time_tick;//ʱ���pts
	uint32_t decode_tick;	//dts
	int32_t end_flag;
	int32_t blue_flag;
	int32_t code_rate;
	uint32_t packet_num;
} parse_data_t;



#endif
