/*********************************************************************************
 *      Copyright:  (C) 2025 iot25<lingyun@email.com>
 *                  All rights reserved.
 *
 *       Filename:  packet.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(13/05/25)
 *         Author:  iot25 <lingyun@email.com>
 *      ChangeLog:  1, Release initial version on "13/05/25 18:26:06"
 *                 
 ********************************************************************************/

#include "packet.h"

int get_time(time_t start_time, char *time_str, size_t len)
{
	memset(time_str, 0, len);
	struct tm *local_time = localtime(&start_time);

	if ( ! strftime(time_str, len, "%Y-%m-%d %H:%M:%S", local_time))
	{
		return -1;
	}

	return 0;
}

int get_devid(char *devid, int size, int sn)
{
	if( !devid || size < DEVID_LEN )
	{
		return -1;
	}

	memset(devid, 0, size);
	snprintf(devid, size, "rpi#%04d", sn);
	return 0;
}

int packet_segmented_pack(pack_info_t *pack, uint8_t *pack_buf, int size)
{
	char             *buf = (char *)pack_buf;

	if( !pack || !buf || size<=0 )
	{
		printf("buf: %s, size: %d \n", buf, size);
		return -1;
	}
					 
	memset(buf, 0, size);
	snprintf(buf, size, "%s-%.3f-%s", pack->devid, pack->temper, pack->time_str);

	return 0;
}

int packet_json_pack(pack_info_t *pack, uint8_t *pack_buf, int size)
{
	char             *buf = (char *)pack_buf;

	if( !pack || !buf || size<=0 )
	{
		return -1;
	}
					 
	memset(buf, 0, size);
	snprintf(buf, size, "{\"devid\":\"%s\", \"temperature\":\"%.3f\", \"time\":\"%s\"}", pack->devid, pack->temper, pack->time_str);

	return 0;
}

//大小端字节序转换
uint16_t to_big_endian(uint16_t num)
{
	    return (num << 8) | (num >> 8);
}

//CRC-16/ITU-T检验函数
static uint16_t crc_itu_t(const uint8_t *data, size_t length)
{
	uint16_t          crc = 0xFFFF;
	size_t            i, j;
			 
	for (i=0; i<length; i++)
	{
		crc ^= ((uint16_t)data[i] << 8);
									 
		for(j=0; j<8; j++)
		{
			if (crc & 0x8000)
			{
				crc = (crc << 1) ^ CRC16_ITU_T_POLY;
			}
			else
			{
				crc <<= 1;
			}
		}
	}
				 
	return crc;
}

int packet_tlv_pack(pack_info_t *pack_info, uint8_t *pack_buf, int size)
{
	int             ofset = 0;
	uint16_t        crc;

	if( !pack_info || !pack_buf || size<TLV_MINSIZE )
	{
		log_error("Invalid input arguments\n");
		return -1;
	}
	
	//TLV header(2B)
	*(uint16_t *)pack_buf = to_big_endian(TLV_HEADER);
	ofset += 2;

	//TLV tag(1B)
	pack_buf[ofset++] = TAG_TEMPERATURE;

	//skip TLV length(2B)
	ofset += 2;

	//TLV value
	//8B devid
	strncpy((char *)(pack_buf + ofset), pack_info->devid, DEVID_LEN);
	ofset += DEVID_LEN;

	//20B time
	strncpy((char *)(pack_buf + ofset), pack_info->time_str, TIME_LEN);
	ofset += TIME_LEN;

	//2B temper
	pack_buf[ofset++] = (int)pack_info->temper;
	pack_buf[ofset++] = (((int)(pack_info->temper*100))%100);

	//TLV length(2B)
	*(uint16_t *)(pack_buf+3) = (ofset-5);

	//TLV CRC(2B)
	crc = crc_itu_t(pack_buf, ofset);
	*(uint16_t *)(pack_buf+ofset) = crc;

	return 0;
}
