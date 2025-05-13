/********************************************************************************
 *      Copyright:  (C) 2025 iot25<lingyun@email.com>
 *                  All rights reserved.
 *
 *       Filename:  packet.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(13/05/25)
 *         Author:  iot25 <lingyun@email.com>
 *      ChangeLog:  1, Release initial version on "13/05/25 18:05:12"
 *                 
 ********************************************************************************/

#ifndef _PACKET_H_
#define _PACKET_H_

#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "logger.h"
#include "ds18b20.h"

#define DEVID_LEN 8

typedef struct pack_info_s
{
	char       devid[DEVID_LEN+1];
	char       time_str[60];
	float      temper;
}pack_info_t;

typedef int (* pack_proc_t)(pack_info_t *pack, uint8_t *buf, int size);

int get_time(time_t start_time, char *time_str, size_t len);
int get_devid(char *devid, int size, int sn);
int packet_segmented_pack(pack_info_t *pack, uint8_t *pack_buf, int size);
int packet_json_pack(pack_info_t *pack, uint8_t *pack_buf, int size);

#endif
