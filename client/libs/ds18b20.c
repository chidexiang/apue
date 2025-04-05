/*********************************************************************************
 *      Copyright:  (C) 2025 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  ds18b20.c
 *    Description:  This file is ds18b20
 *                 
 *        Version:  1.0.0(30/03/25)
 *         Author:  LingYun <iot25@lingyun>
 *      ChangeLog:  1, Release initial version on "30/03/25 20:36:56"
 *                 
 ********************************************************************************/

#include "ds18b20.h"
#include "log.h"

int gettemp(float *temp, char **chip_path)
{
	int                  fd;
	int                  rv;
	char                *ptr;
	DIR                 *dirp;
	char                *id_path;
	char                 path[1024];
	char                 buf[1024];
	struct dirent       *readdirp = NULL;

	//打开到一定能查找到的文件目录
	if ( (dirp = opendir(PATH)) == NULL)
	{
		log_error("open dirp failure: %s", strerror(errno));
		return -1;
	}

	//寻找产品文件名
	while ( (readdirp = readdir(dirp)) != NULL)
	{
		if ((strstr(readdirp->d_name, "28-")) != NULL)
		{
			id_path = strstr(readdirp->d_name, "28-");
			goto success;
		}
	}

		log_error("find '28-' failure: %s", strerror(errno));
		closedir(dirp);
		return -2;

success:
	closedir(dirp);
	*chip_path = strdup(id_path);

	//拼接路径
	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "%s%s/w1_slave", PATH, *chip_path);

	//打开温度传感器文件
	if ( (fd = open(path, O_RDONLY)) < 0)
	{
		log_error("open file failure: %s", strerror(errno));
		return -3;
	}

	//读取其中内容
	memset(buf, 0, sizeof(buf));
	if (read(fd, buf, sizeof(buf)) < 0)
	{
		log_error(" read faile failure: %s", strerror(errno));
		return -4;
	}

	//读出温度值
	if((ptr=strstr(buf, "t=")) == NULL)
	{
		log_error("read temp failure: %s", strerror(errno));
		return -5;
	}

	ptr += 2;

	*temp=atof(ptr)/1000;

//	printf("%f\n", *temp);

	close(fd);

	return 0;
}
