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


