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

const char 					*CLIPATH = "client.db";

void *temp_worker(void *args);

time_t                     rawtime;

void sig_sigint(int signum)
{
	if (SIGINT == signum)
	{
		log_info("delect sqlite");
		delect_data_local();
		exit(0);
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
	pthread_attr_t          temp_attr;
	pthread_t               tid;
	FILE                   *fp;

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
		goto cleanup;
	}

	//初始化温度读取线程
	if (pthread_attr_init(&temp_attr))
	{
		log_error("pthread_attr_init() failure: %s", strerror(errno));
		return -1;
	}

	//将温度读取线程设置为分离状态
	if (pthread_attr_setdetachstate(&temp_attr, PTHREAD_CREATE_DETACHED))
	{
		log_error("pthread_attr_setdetachstate() failure: %s", strerror(errno));
		return -1;
	}

	//创建温度读取并传入服务器端线程
	pthread_create(&tid, &temp_attr, temp_worker, &sockfd);

	pthread_attr_destroy(&temp_attr);
	
	for ( ; ; )
	{
		//时刻判断数据写入过程是否出错，一旦出错进行重连
		if ( ! g_sock_time)
		{
		    handle_disconnection(&sockfd, &servaddr, servip, &port);
		}

	}

cleanup:
	close(sockfd);
	free(servip);
		return 0;
}

void *temp_worker(void *args)
{
	float           temp;
	char            buf[128];
	int            *sockfd;
	char           *name = calloc(64, sizeof(char));
	char           *time_str;
	struct tcp_info info;
	int             len = sizeof(info);

	sockfd = (int *)args;

	for ( ; ; )
	{
		gettemp(&temp, &name);//获取到温度值
		if (name == NULL)
		{
			log_error("get ID failure");
			return 0;
		}
		memset(buf, 0, sizeof(buf));                      
		time(&rawtime);//获取当前时间
		time_str = ctime(&rawtime);
		time_str[strcspn(time_str, "\n")] = '\0';//将获取到的时间后面的换行符删除
		snprintf(buf, sizeof(buf), "%s-%f-%s", name, temp, time_str);
		log_info("ready to send [%d] data: %s", strlen(buf), buf);

		getsockopt(*sockfd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);

		if(info.tcpi_state == TCP_ESTABLISHED)
		{
			write(*sockfd, buf, strlen(buf));
		}
		else
		{
			log_error("write to server failure");
			log_info("ready to send data to sqlite");

			g_sock_time = 0;

			init_local_db();//初始化数据库
			cache_data_local(buf);//数据存入数据库
		}

		sleep(g_timeout - 1);
	}
}
