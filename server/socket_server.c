/*********************************************************************************
 *      Copyright:  (C) 2025 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  socket_server.c
 *    Description:  This file is server
 *                 
 *        Version:  1.0.0(30/03/25)
 *         Author:  LingYun <iot25@lingyun>
 *      ChangeLog:  1, Release initial version on "30/03/25 17:05:49"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <libgen.h>

#include "sqlite.h"
#include "serverinit.h"
#include "serverinput.h"
#include "log.h"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

const char                 *CLIPATH = "server.db";

int main(int argc, char **argv)
{
	int						listenfd;
	int                     connfd;
	int						rv = -1;
	int                     maxfd = 0;
	int                     port = 0;
	int                     found;
	char                   *progname = basename(argv[0]);
	char                   *servip = NULL;
	char					buf[51];
	int						i;
	int                     fds_array[1024];
	fd_set                  rdset;
	FILE                    *fp;
	fp = fopen("server.txt", "a+");
	if(fp == NULL)
	{
		printf("creat log file failure\n");
		return 0;
	}

	//设置日志级别(在终端打印)
	log_set_level(LOG_TRACE);

	//设置日志级别(在文件中打印)
	log_add_fp(fp, LOG_INFO);

	//启动参数域名解析
	server_input(argc, argv, &servip, &port, progname);

	//建立socket连接
	if ((listenfd = socket_server_init(NULL, port)) < 0)
	{
		log_error("server listen on %d port failure", port);
		return -2;
	}

	log_info("server start to listen on port %d", port);

	//启动select多路复用
	for (i=0;i<ARRAY_SIZE(fds_array);i++)
	{
		fds_array[i] = -1;
	}
	fds_array[0] = listenfd;

	for ( ; ; )
	{
		FD_ZERO(&rdset);
		for (i=0;i<ARRAY_SIZE(fds_array);i++)
		{
			if (fds_array[i] < 0)
			{
				continue;
			}

			maxfd = maxfd > fds_array[i] ? maxfd : fds_array[i];
			FD_SET(fds_array[i], &rdset);
		}

		rv = select(maxfd+1, &rdset, NULL, NULL, NULL);
		if (rv < 0)
		{
			log_error("select failure: %s", strerror(errno));
			break;
		}
		else if (rv == 0)
		{
			log_error("timeout");
			continue;
		}

		for (i=0;i<rv;i++)
		{

			if (FD_ISSET(listenfd, &rdset))
			{
				if ((connfd = accept(listenfd, (struct sockaddr *)NULL, NULL)) < 0)
				{
					log_error("accept new client failure: %s", strerror(errno));
					continue;
				}

				found = 0;
				for (i=1;i<ARRAY_SIZE(fds_array);i++)
				{
					if (fds_array[i] < 0)
					{
						log_info("accept new client[%d]", connfd);
						fds_array[i] = connfd;
						found = 1;
						break;
					}
				}

				if ( ! found)
				{
					log_error("failure to add new client because full");
					close(connfd);
				}
			}
			else
			{
				for (i=0;i<ARRAY_SIZE(fds_array);i++)
				{
					if (fds_array[i] < 0 || !FD_ISSET(fds_array[i], &rdset))
					{
						continue;
					}

					memset(buf, 0, sizeof(buf));
					if ((rv = read(fds_array[i], buf, sizeof(buf) - 1)) <= 0)
					{
						log_error("socket[%d] read failure or get disconncet", fds_array[i]);
						close(fds_array[i]);
						fds_array[i] = -1;
						continue;
					}
					buf[rv] = '\0';
					log_info("socket[%d] read: %s", fds_array[i], buf);
					log_info("send data to sqlite");
					init_local_db();//初始化数据库
					cache_data_local(buf);//数据存入数据库
				}
			}
		}
	}

	close(listenfd);
	return 0;
}
