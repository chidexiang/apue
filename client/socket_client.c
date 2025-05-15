/*********************************************************************************
 *      Copyright:  (C) 2025 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  socket_client.c
 *ke clean -C src    Description:  This file is socket client
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
#include "packet.h"

#define CLIPATH "client.db"
#define DEFAULT_TIME 5

static int   sig_stop = 0;

void print_usage(char *progname);

void sig_sigint(int signum)
{
	if (SIGINT == signum)
	{
		sig_stop = 1;
	}
}

int main(int argc, char **argv)
{
	socket_ctx_t            socket_ctx;
	int                     second = DEFAULT_TIME;
	int                     rv = -1;
	int                     format = 0;
	char                   *progname = basename(argv[0]);
	FILE                   *fp;
	struct tcp_info         info;
	int                     len = sizeof(info);
	time_t                  start_time, end_time = 0;
	float                   temp;
	char                    buf[128];
	char                   *name;
	sqlite3                *db = NULL;
	char                   *logfile="sock_client.log";
	int                     loglevel=LOG_LEVEL_TRACE;
	int                     logsize=10;
	int                     daemon_id = 1;
	int                     gettemp_flag = 0;
	char                   *servip;
	int                     sn = 1;
	pack_proc_t             pack_proc = packet_segmented_pack;
	pack_info_t             pack;
	struct option           opts[] = 
	{
		{"ipaddr", required_argument, NULL, 'i'},
		{"port", required_argument, NULL, 'p'},
		{"help", no_argument, NULL, 'h'},
		{"time", required_argument, NULL, 't'},
		{"debug", no_argument, NULL, 'd'},
		{"kill", no_argument, NULL, 'k'},
		{"format", required_argument, NULL, 'f'},
		{"sn", required_argument, NULL, 's'},
		{NULL, 0, NULL, 0}
	};
    int                     ch;

	socket_ctx.servip = NULL;
	socket_ctx.port = -1;

	while ( (ch = getopt_long(argc, argv, "i:p:ht:dkf:s:", opts, NULL)) != -1)
	{
		switch(ch)
		{
			case 'i':
				socket_ctx.servip = optarg;
				break;

			case 'p':
				socket_ctx.port = atoi(optarg);
				break;

			case 'h':
				print_usage(progname);
				return 0;
				break;

			case 't':
				second = atoi(optarg);
				break;

			case 'd':
				daemon_id = 0;
				logfile="console";
				loglevel=LOG_LEVEL_DEBUG;
				break;

			case 'k':
				if (kill_daemon(progname) == -1)
				{
					fprintf(stderr, "kill failed\n");
				}
				return 0;

			case 'f':
				format = atoi(optarg);
				if (format == 0)
				{
					pack_proc = packet_segmented_pack;
				}
				else if (format == 1)
				{
					pack_proc = packet_json_pack;
				}
				else if (format == 2)
				{
					pack_proc = packet_tlv_pack;
				}
				else
				{
					fprintf(stderr, "format select failure\n");
					print_usage(progname);
					return 0;
				}
				break;

			case 's':
				sn = atoi(optarg);
				break;

			default:
				break;
		}
	}

	if ( !socket_ctx.servip || !socket_ctx.port)
	{
		print_usage(progname);
		return 0;
	}

	if ( !daemon_id)
	{
		if (daemon(1, 1) == -1)
		{
			fprintf(stderr, "daemon failure\n");
			return 0;
		}
	}

	//捕捉信号
	signal(SIGINT, sig_sigint);

	//创建客户端日志
	if( log_open(logfile, loglevel, logsize, LOG_LOCK_DISABLE) < 0 )
	{
		fprintf(stderr, "Initial log system failed\n");
		return 1;
	}

	if (init_local_db(CLIPATH, &db) < 0)//初始化数据库
	{
		log_error("init sqlite failure\n");
		goto cleanup;
	}

	while ( !sig_stop)
	{
		memset(buf, 0, sizeof(buf));
		gettemp_flag = 0;

		//获取当前时间
		time(&start_time);

		//采样
		if (difftime(start_time, end_time) >= second)
		{
			if (gettemp(&pack.temper, NULL) < 0)//获取到温度值
			{
				log_error("get ds18b20 temperature failure and try again\n");
				continue;
			}

			//获取采样时间字符串
			if (get_time(start_time, pack.time_str, sizeof(pack.time_str)) < 0)
			{
				log_error("get time failure\n");
				continue;
			}

			//生成产品序列号
			if (get_devid(pack.devid, sizeof(pack.devid), sn) < 0)
			{
				log_error("get devid failure\n");
				continue;
			}

			//数据打包
			if (pack_proc(&pack, (uint8_t *)buf, sizeof(buf)) < 0)
			{
				log_error("pack data failure\n");
				continue;
			}

			//log_info("get data [%d] data: %s\n", strlen(buf), buf);
			gettemp_flag = 1;
			end_time = start_time;
		}

		//判断连接状态
		//连接失败尝试重连
		if ( !socket_status(&socket_ctx))
		{
			socket_connect(&socket_ctx);
		}

		if ( !socket_status(&socket_ctx))
		{
			//如果有采样，重连失败则存入数据库
			if ( gettemp_flag )
			{
				if (cache_data_local(buf, db) < 0)//数据存入数据库
				{
					log_error("cache data to sqlite failure\n");
				}
			}
			continue;
		}

		//连接成功发送数据
		//如果采样发送到服务器
		if (gettemp_flag == 1)
		{
			log_info("ready to send server\n");
			if (write(socket_ctx.sockfd, buf, strlen(buf)) <= 0)
			{
				log_error("send data to server failure and ready send data to sqlite: %s\n", strerror(errno));
				if (cache_data_local(buf, db) < 0)//数据存入数据库
				{
					log_error("cache data to sqlite failure\n");
					continue;
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
			if (write(socket_ctx.sockfd, buf, strlen(buf)) <= 0)
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
	close(socket_ctx.sockfd);
	if (db != NULL)
	{
		log_info("delect sqlite\n");
		delete_data_local(db);
		close_local_db(&db);
	}
	return 0;
}

void print_usage(char *progname)
{
	printf("%s usage: \n", progname);

	printf("-i(--ipaddr): sepcify server IP address\n");
	printf("-p(--port)  : sepcify server port.\n");
	printf("-h(--help)  : print this help information.\n");
	printf("-t(--time)  : change the default sampling time.\n");
	printf("-d(--debug) : running in debug mode\n");
	printf("-k(--kill)  : kill daemon process\n");
	printf("-f(--format): choose the format you want to print\n");
	printf("              input \"0\": segmented print\n");
	printf("              input \"1\": json print\n");
	printf("-s(--sn)    : input device id\n");

	return ;
}
