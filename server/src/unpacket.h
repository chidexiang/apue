/********************************************************************************
 *      Copyright:  (C) 2025 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  packet.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(17/05/25)
 *         Author:  LingYun <iot25@lingyun>
 *      ChangeLog:  1, Release initial version on "17/05/25 18:11:26"
 *                 
 ********************************************************************************/

#ifndef _UNPACKET_H_
#define _UNPACKET_H_

#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "log.h"

#define DEVID_LEN        8
#define TIME_LEN         20
#define CRC16_ITU_T_POLY 0x1021 //CRC-16/ITU-T 的标准多项式
#define TLV_HEADER       0xFDFE //原始头标识
#define TLV_MINSIZE      37

enum
{
	TAG_TEMPERATURE = 1,  //温度标签
	TAG_HUMIDITY,         //湿度标签
	TAG_NOISY,            //噪声标签
};

typedef struct pack_info_s
{
	char       devid[DEVID_LEN+1];
	char       time_str[20];
	float      temper;
}pack_info_t;

typedef int (* pack_proc_t)(pack_info_t *pack, uint8_t *buf, int size);

static uint16_t crc_itu_t(const uint8_t *data, size_t length);
uint16_t to_big_endian(uint16_t num);
int packet_tlv_unpack(pack_info_t *pack_info, uint8_t *pack_buf, int size);

#endif
