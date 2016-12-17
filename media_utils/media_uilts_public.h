/*************************************************************************
Author: 459336407@qq.com
date:2016.12.16
************************************************************************/

#ifndef _MEDIA_UTILS_H__
#define _MEDIA_UTILS_H__

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct codec_data_h264 
{
	uint8_t sps[64]; //no nal leader code 001
	uint8_t pps[64];
	uint32_t sps_len;
	uint32_t pps_len;
}Code_DataH264_t;

typedef struct codec_data_h265
{
	uint8_t vps[64];
	uint8_t sps[64];
	uint8_t pps[64];
	uint32_t vps_len;
	uint32_t sps_len;
	uint32_t pps_len;
}Code_DataH265_t;

struct codec_data_g726
{
	uint32_t bit_rate;
}Code_DataG726_t;

struct codec_data_aac
{
	uint8_t audio_specific_config[64];
	uint32_t audio_specific_config_len;
	uint32_t sample_rate;
	uint32_t channels;
}Codec_DataAac_t;


#ifdef __cplusplus
}
#endif
#endif

