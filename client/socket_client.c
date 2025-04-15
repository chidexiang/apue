/*********************************************************************************
 *      Copyright:  (C) 2025 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  socket_client.c
 *    Description:  This file is socket client
 *                 
 *        Version:  1.0.0(30/03/25)
 *         Author:  Guanjiaxu <iot25@lingyun>
 *      ChangeLog:  1, Release initial version on "30/03/25 16:22:48"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <getopt.h>
#include <netdb.h>
#include <libgen.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <pthread.h>
#include <sqlite3.h>
#include <netinet/tcp.h>

#include "ds18b20.h"
#include "sqlite.h"
#include "log.h"
#include "client_socket.h"
#include "clientinput.h"

#define CLIPATH "client.db"

static int   sigint_flag = 0;

void sig_sigint(int signum)
{
	if (SIGINT == signum)
	{
		sigint_flag = 1;
	}
}

int main(int argc, char **argv)
{
	int                     port = 0;
	char                   *servip = NULL;
	int                     sockfd;
	int                     rv = -1;
	struct sockaddr_in      servaddr, *addr;
	char                   *progname = basename(argv[0]);
	FILE                   *fp;
	struct tcp_info         info;
	int                     len = sizeof(info);
	time_t                  start_time, end_time = 0;
	float                   temp;
	char                    buf[128];
	char                   *name = calloc(64, sizeof(char));
	char                   *time_str;
	sqlite3                *db = NULL;

	//捕捉ctrl+c信号
	signal(SIGINT, sig_sigint);

	//创建客户端日志
	fp = fopen("client.txt", "a+");
	if (fp == NULL)
	{
		printf("create log file failure\n");
		return 0;
	}

	//设置日志级别(终端打印)
	log_set_level(LOG_TRACE);

	//设置日志级别(文件打印)
	log_add_fp(fp, LOG_INFO);

	//启动参数域名解析
	client_input(argc, argv, &servip, &port, progname);

	//与服务器建立连接
	rv = setup_socket(&sockfd, &servaddr, servip, &port);
	if (rv < 0)
	{
		log_error("setup_socket failure!");
		goto cleanup;
	}

	init_local_db(CLIPATH, &db);//初始化数据库

	for ( ; ; )
	{
		//获取当前时间
		time(&start_time);

		memset(buf, 0, sizeof(buf));

		//采样
		if (difftime(start_time, end_time) >= g_timeout)
		{
			gettemp(&temp, &name);//获取到温度值
			if (name == NULL)
			{   
				log_error("get ID failure");
				return 0;
			}
			time_str = ctime(&start_time);
			time_str[strcspn(time_str, "\n")] = '\0';//将获取到的时间后面的换行符删除
			snprintf(buf, sizeof(buf), "%s-%f-%s", name, temp, time_str);
			log_info("get data [%d] data: %s", strlen(buf), buf);
			end_time = start_time;
		}

		//判断连接状态
		getsockopt(sockfd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
		//连接失败尝试重连
		if (info.tcpi_state != TCP_ESTABLISHED)
		{
			close(sockfd);
			setup_socket(&sockfd, &servaddr, servip, &port);
			//如果采样发送到数据库
			if (is_empty(buf, sizeof(buf)) == 0)
			{
				log_error("write to server failure");
				log_info("ready to send data to sqlite");
				 
				cache_data_local(buf, db);//数据存入数据库
			}
			continue;
		}

		//连接成功发送数据
		//如果采样发送到服务器
		if (is_empty(buf, sizeof(buf)) == 0)
		{
			log_info("ready to send server");
			write(sockfd, buf, strlen(buf));
		}

		//检查数据数据库中是否有数据
		if ((rv = find_data_local(db)) == 1)
		{
			send_1st_data_local(buf, db);//读取数据库的第一条数据
			write(sockfd, buf, strlen(buf));
			delect_1st_data_local(db);//删除temp表的第一条数据
		}

		if (sigint_flag)
		{
			break;
		}
	}

cleanup:
	close(sockfd);
	free(servip);
	if (db != NULL)
	{
		log_info("delect sqlite");
		delect_data_local(db);
		close_local_db(&db);
	}
	return 0;
}

