/*********************************************************************************
 *      Copyright:  (C) 2025 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  unpacket.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(17/05/25)
 *         Author:  LingYun <iot25@lingyun>
 *      ChangeLog:  1, Release initial version on "17/05/25 18:25:00"
 *                 
 ********************************************************************************/

#include "unpacket.h"

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

//大小端字节序转换
uint16_t to_big_endian(uint16_t num)
{
        return (num << 8) | (num >> 8); 
}

int packet_tlv_unpack(pack_info_t *pack_info, uint8_t *pack_buf, int size)
{
	if (!pack_info || !pack_buf || size < TLV_MINSIZE) 
	{
		log_error("Invalid input arguments\n");
		return -1;
	}

	// 1. 校验TLV头部
	uint16_t header = *(uint16_t *)pack_buf;
	if (header != to_big_endian(TLV_HEADER)) 
	{
		log_error("Invalid TLV header\n");
		return -1;
	}
			
	// 2. 校验Tag
	uint8_t tag = pack_buf[2];
	if (tag != TAG_TEMPERATURE) 
	{
		log_error("Invalid TLV tag\n");
		return -1;
	}
			
	// 3. 获取TLV长度（大端序）
	uint16_t tlv_len = *(uint16_t *)(pack_buf + 3);
	if (tlv_len != DEVID_LEN + TIME_LEN + 2) 
	{  // 8B devid + 20B time + 2B temper
		log_error("Invalid TLV length\n");
		return -1;
	}
			
	// 4. 解析Value部分
	int offset = 5;  // 跳过Header(2B) + Tag(1B) + Length(2B)
			
	// 设备ID（8字节）
	memcpy(pack_info->devid, pack_buf + offset, DEVID_LEN);
	pack_info->devid[DEVID_LEN] = '\0';  // 确保字符串终止
	offset += DEVID_LEN;
			
	// 时间字符串（20字节）
	memcpy(pack_info->time_str, pack_buf + offset, TIME_LEN);
	pack_info->time_str[TIME_LEN - 1] = '\0';
	offset += TIME_LEN;
			
	// 温度值（2字节：整数部分 + 小数部分）
	uint8_t int_part = pack_buf[offset++];
	uint8_t frac_part = pack_buf[offset++];
	pack_info->temper = int_part + (frac_part / 100.0f);
			
	// 5. 校验CRC（与发送端一致）
	uint16_t crc_received = *(uint16_t *)(pack_buf + offset);
	uint16_t crc_calculated = crc_itu_t(pack_buf, offset);  // offset此时指向CRC字段前
	if (crc_received != crc_calculated) 
	{
		log_error("CRC mismatch\n");
		return -1;
	}
			
	return 0;  // 成功
}
