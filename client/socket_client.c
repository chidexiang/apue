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
#include "logger.h"
#include "client_socket.h"
#include "clientinput.h"

#define CLIPATH "client.db"
#define DEFAULT_TIME 5
#define SENSOR_ID "ds18b20-1"//此处更改产品序列号

static int   sig_stop = 0;

void sig_sigint(int signum)
{
	if (SIGINT == signum)
	{
		sig_stop = 1;
	}
}

int main(int argc, char **argv)
{
	int                     port = 0;
	int                     second = DEFAULT_TIME;
	char                   *servname = NULL;
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
	char                   *name;
	char                   *time_str;
	sqlite3                *db = NULL;
	char                   *logfile="sock_client.log";
	int                     loglevel=LOG_LEVEL_TRACE;
	int                     logsize=10;

	//捕捉ctrl+c信号
	signal(SIGINT, sig_sigint);

	//创建客户端日志
	if( log_open(logfile, loglevel, logsize, LOG_LOCK_DISABLE) < 0 )
	{
		fprintf(stderr, "Initial log system failed\n");
		return 1;
	}

	//输出到控制台
	if( log_open("console", loglevel, logsize, LOG_LOCK_DISABLE) < 0 )
	{
		fprintf(stderr, "Initial log system failed\n");
		return 1;
	}

	//启动参数域名解析
	client_input(argc, argv, &servip, &port, progname, &second);

	//与服务器建立连接
	rv = socket_connect(&sockfd, &servaddr, servip, &port);
	if (rv < 0)
	{
		log_error("socket connect failure!\n");
		goto cleanup;
	}

	if (init_local_db(CLIPATH, &db) < 0)//初始化数据库
	{
		log_error("init sqlite failure\n");
		goto cleanup;
	}

	while ( ! sig_stop)
	{
		memset(buf, 0, sizeof(buf));

		//获取当前时间
		time(&start_time);

		//采样
		if (difftime(start_time, end_time) >= second)
		{
			if (gettemp(&temp, &name) < 0)//获取到温度值
			{
				log_error("get ds18b20 temperature failure and try again\n");
				continue;
			}
			time_str = ctime(&start_time);
			time_str[strcspn(time_str, "\n")] = '\0';//将获取到的时间后面的换行符删除
			snprintf(buf, sizeof(buf), "%s-%s-%f-%s", SENSOR_ID, name, temp, time_str);
			log_info("get data [%d] data: %s\n", strlen(buf), buf);
			end_time = start_time;
		}

		//判断连接状态
		if (getsockopt(sockfd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len) < 0)
		{
			log_error("getsockopt failure: %s\n", strerror(errno));
			break;
		}
		//连接失败尝试重连
		if (info.tcpi_state != TCP_ESTABLISHED)
		{
			close(sockfd);
			if (socket_connect(&sockfd, &servaddr, servip, &port) < 0)
			{
				log_error("socket_connect failure!\n");
				break;
			}
			//如果采样发送到数据库，等待之后循环依次发送
			if (is_empty(buf, sizeof(buf)) == 0)
			{
				log_error("write to server failure and ready send data to sqlite\n");
				 
				if (cache_data_local(buf, db) < 0)//数据存入数据库
				{
					log_error("cache data to sqlite failure\n");
					break;
				}
			}
			continue;
		}

		//连接成功发送数据
		//如果采样发送到服务器
		if (is_empty(buf, sizeof(buf)) == 0)
		{
			log_info("ready to send server\n");
			if (write(sockfd, buf, strlen(buf)) <= 0)
			{
				log_error("send data to server failure and ready send data to sqlite: %s\n", strerror(errno));
				if (cache_data_local(buf, db) < 0)//数据存入数据库
				{
					log_error("cache data to sqlite failure\n");
					break;
				}
			}
		}

		//检查数据数据库中是否有数据
		if ((rv = find_data_local(db)) == 1)
		{
			if (send_1st_data_local(buf, db) < 0)//读取数据库的第一条数据
			{
				log_error("read from sqlite failure\n");
				continue;
			}
			if (write(sockfd, buf, strlen(buf)) <= 0)
			{
				log_error("send data to server failure\n");
				continue;
			}
			if (delete_1st_data_local(db) < 0)//删除temp表的第一条数据
			{
				log_error("delete data from sqlite failure\n");
				continue;
			}
		}
	}

cleanup:
	log_close();
	close(sockfd);
	if (db != NULL)
	{
		log_info("delect sqlite\n");
		delete_data_local(db);
		close_local_db(&db);
	}
	return 0;
}

