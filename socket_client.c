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

const char 					*CLIPATH = "client.db";

//int init_local_db(void);
//int cache_data_local(char *data);
//int delect_data_local(void);
int send_callback(void *sockfd, int f_num, char **f_value, char **f_name);

void *temp_worker(void *args);
void print_usage(char *progname);
int setup_socket(int *sockfd, struct sockaddr_in *servaddr, char *servip, int *port);
void handle_disconnection(int *sockfd, struct sockaddr_in *servaddr, char *servip, int *port);

int                        g_error = 0;
int                        g_sock_time = 1;
int                        g_timeout = 5;
time_t                     rawtime;

void sig_sigpipe(int signum)
{
	if (SIGPIPE == signum)
	{
		g_error = 1;
	}
}
int main(int argc, char **argv)
{
	int                     port = 0;
	char                   *servip = NULL;
	int                     sockfd;
	int                     rv = -1;
	struct sockaddr_in      servaddr, *addr;
	char                    buf[1024];
	char                   *progname = NULL;
	const char             *hostname;
	struct addrinfo         hints, *res, *p;
	int                     status = 1;
//	float                   temp;
//	time_t                  rawtime;
//	int						timeout = 5;// 定时时长
//	char                   *path = "client.db";
	pthread_attr_t          temp_attr;
	pthread_t               tid;

	struct option           opts[] = {
		{"ipaddr", required_argument, NULL, 'i'},
		{"port", required_argument, NULL, 'p'},
		{"dmname", required_argument, NULL, 'd'},
		{"help", no_argument, NULL, 'h'},
		{"time", required_argument, NULL, 't'},
		{NULL, 0, NULL, 0}
	};
	int                     ch;

	progname = basename(argv[0]);

	signal(SIGPIPE, sig_sigpipe);

	//输入命令行参数
	while ( (ch = getopt_long(argc, argv, "i:p:d:ht:", opts, NULL)) != -1)
	{
		switch(ch)
		{
			case 'i':
				servip = optarg;
				break;
			case 'p':
				port = atoi(optarg);
				break;
			case 'h':
				print_usage(progname);
				break;
			case 't':
				g_timeout = atoi(optarg);
				break;
			case 'd':
				hostname = optarg;

				memset(&hints, 0, sizeof(hints));
				hints.ai_family = AF_INET; //支持IPV4
				hints.ai_socktype = SOCK_STREAM; //TCP

				//DNS解析
				if((status = getaddrinfo(hostname, NULL, &hints, &res)) != 0)
				{
					printf("getaddrinfo error: %s\n", strerror(errno));
					return -1;
				}

				//遍历结果链表
				for (p = res; p != NULL; p = p->ai_next)
				{
					addr = (struct sockaddr_in *)p->ai_addr;
					servip = inet_ntoa(addr->sin_addr);
					printf("IP address: %s\n", inet_ntoa(addr->sin_addr));
				}

				break;
			default:
				break;
		}
	}
	
	if (! servip || ! port)
	{
		print_usage(progname);
		return 0;
	}

	rv = setup_socket(&sockfd, &servaddr, servip, &port);
	if (rv < 0)
	{
		goto cleanup;
	}


	//初始化温度读取线程
	if (pthread_attr_init(&temp_attr))
	{
		printf("pthread_attr_init() failure: %s\n", strerror(errno));
		return -1;
	}

	//将温度读取线程设置为分离状态
	if (pthread_attr_setdetachstate(&temp_attr, PTHREAD_CREATE_DETACHED))
	{
		printf("pthread_attr_setdetachstate() failure: %s\n", strerror(errno));
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
#if 0	
		//测试接受服务器端数据
		memset(buf, 0, sizeof(buf));
		rv = read(sockfd, buf, sizeof(buf));
		if (rv <= 0)
		{
			printf("read from server failure or disconnect: %s\n", strerror(errno));
			handle_disconnection(&sockfd, &servaddr, servip, &port);
			continue;
		}

		printf("read %d from server: %s\n", rv, buf);
#endif

	}

cleanup:
	close(sockfd);
		return 0;
}

void *temp_worker(void *args)
{
	float           temp;
	char            buf[128];
	int            *sockfd;
	char           *name = calloc(64, sizeof(char));
	struct tcp_info info;
	int             len = sizeof(info);

	sockfd = (int *)args;

	for ( ; ; )
	{
		gettemp(&temp, &name);//获取到温度值
		if (name == NULL)
		{
			printf("get ID failure\n");
			return 0;
		}
//		printf("%s\n", name);
		memset(buf, 0, sizeof(buf));                      
		time(&rawtime);//获取当前时间
		snprintf(buf, sizeof(buf), "%s-%f-%s", name, temp, ctime(&rawtime));
		printf("ready to send: %s\n", buf);

		getsockopt(*sockfd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);

	//	if ( (shutdown(*sockfd, SHUT_RD) < 0) || (write(*sockfd, buf, strlen(buf)) < 0) || ( g_error == 1))
		if(info.tcpi_state == TCP_ESTABLISHED)
		{
			write(*sockfd, buf, strlen(buf));
		}
		else
		{
			printf("write to server failure\n");
			printf("ready to send data to sqlite\n");

			g_sock_time = 0;

			init_local_db();//初始化数据库
			cache_data_local(buf);//数据存入数据库
		}

		sleep(g_timeout - 1);
	}

	free(name);
}

int setup_socket(int *sockfd, struct sockaddr_in *servaddr, char *servip, int *port)
{
	int             keepalive = 1;
	int             keepalive_idle = 3;//空闲3s后开始探测
	int             keepalive_interval = 1;//探测包每隔1秒发送一次
	int             keepalive_count = 1;//最多发送1个探测包

	*sockfd = -1;

	//客户端SOCKET连接
	if ( (*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{
		printf("creat socket failure: %s\n", strerror(errno));
		return -1;
	}

#if 0
	if (setsockopt(*sockfd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)) < 0)
	{
		printf("set keepalive failure\n");
	}
	if (setsockopt(*sockfd, IPPROTO_TCP, TCP_KEEPIDLE, &keepalive_idle, sizeof(keepalive_idle)) < 0)
	{
		printf("set keepalive_idle failure\n");
	}
	if (setsockopt(*sockfd, IPPROTO_TCP, TCP_KEEPINTVL, &keepalive_interval, sizeof(keepalive_interval)) < 0)
	{
		printf("set keepalive_interval failure\n");
	}
	if (setsockopt(*sockfd, IPPROTO_TCP, TCP_KEEPCNT, &keepalive_count, sizeof(keepalive_count)) < 0)
	{
		printf("set keepalive_count failure\n");
	}
#endif

	//服务器端数据更新
	memset(servaddr, 0, sizeof(*servaddr));
	servaddr->sin_family = AF_INET;
	servaddr->sin_port = htons(*port);
	if (inet_pton(AF_INET, servip, &servaddr->sin_addr) != 1)//void *
	{
		printf("add servaddr ip failure\n");
		return -2;
	}

	//conncet连接
	if (connect(*sockfd, (struct sockaddr *)servaddr, sizeof(*servaddr)) < 0)
	{
		printf("connect server failure: %s\n", strerror(errno));
		close(*sockfd);
		*sockfd = -1;
		return -3;
	}

	if (setsockopt(*sockfd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)) < 0)
	{    
		printf("set keepalive failure\n");
	}                        
	if (setsockopt(*sockfd, IPPROTO_TCP, TCP_KEEPIDLE, &keepalive_idle, sizeof(keepalive_idle)) < 0)
	{    
		printf("set keepalive_idle failure\n");
	}                             
	if (setsockopt(*sockfd, IPPROTO_TCP, TCP_KEEPINTVL, &keepalive_interval, sizeof(keepalive_interval)) < 0)
	{    
		printf("set keepalive_interval failure\n");
	}                                 
	if (setsockopt(*sockfd, IPPROTO_TCP, TCP_KEEPCNT, &keepalive_count, sizeof(keepalive_count)) < 0)
	{    
		printf("set keepalive_count failure\n");
	}

	return 1;
}

void handle_disconnection(int *sockfd, struct sockaddr_in *servaddr, char *servip, int *port)
{
	int             delay;
	int             i;
	int             max = 5;
	sqlite3        *db;
	char           *err_msg;
	char           *sq;

	close(*sockfd);
	*sockfd = -1;

	i = 0;
	while (1)
	{
		i = i < max ? i : max;
		delay = 1 * pow(2, i);
		printf("attempting to reconnect in %d second...\n", delay);
		sleep(delay);

		setup_socket(sockfd, servaddr, servip, port);

		printf("sockfd: %d\n", *sockfd);

		if (*sockfd != -1)
		{
			printf("Reconnect to server!\n");
			g_error = 0;

			printf("ready to send from server\n");
			if (sqlite3_open(CLIPATH, &db) != SQLITE_OK)
			{
				printf("error to open sqlite: %s\n", sqlite3_errmsg(db));
				continue;
			}
			if (sqlite3_exec(db, "select * from temp", send_callback, sockfd, &err_msg) != SQLITE_OK)
			{
				printf("error to read sqlite: %s\n", err_msg);
				sqlite3_free(err_msg);
			}
			sqlite3_close(db);

			printf("delect sqlite later\n");
			delect_data_local();
			g_sock_time = 1;
			return ;
		}
		i++;
	}
}

int send_callback(void *sockfd, int f_num, char **f_value, char **f_name)
{
	char          buf[1024];
	snprintf(buf, sizeof(buf), "%s\n", f_value[0]);

	write(*(int *)sockfd, buf, strlen(buf));
	return 0;
}

void print_usage(char *progname)
{
	printf("%s usage: \n", progname);

	printf("-i(--ipaddr): sepcify server IP address\n");
	printf("-p(--port): sepcify server port.\n");
	printf("-h(--help): print this help information.\n");
	printf("-d(--dmname): sepcify server domain name.\n");

	return ;
}

#if 0
//数据库初始化
int init_local_db(void)
{
	sqlite3            *db;
	char               *err_msg = NULL;
	int	                rv = -1;
	char               *sq;

	//创建一个数据库
	if ( (rv = sqlite3_open(CLIPATH, &db)) != SQLITE_OK)
	{
		printf("error to open sqlite: %s\n", sqlite3_errmsg(db));
		return -1;
	}

	//创建一个存放数据的表
	sq = sqlite3_mprintf("create table  if not exists temp(data char)");
	if ( (rv = sqlite3_exec(db, sq, NULL, NULL, &err_msg)) != SQLITE_OK)
	{
		printf("error to create table: %s\n", err_msg);
		sqlite3_free(err_msg);
		rv = -2;
		goto init_clean;
	}

init_clean:
	sqlite3_free(sq);
	sqlite3_close(db);

	if (rv < 0)
	{
		return rv;
	}

	return 0;
}

//数据存入本地数据库
int cache_data_local(char *data)
{
	sqlite3             *db;
	char                *err_msg = NULL;
	int                  rv = -1;
	char                *sq;

	//打开数据库
	if ( (rv = sqlite3_open(CLIPATH, &db)) != SQLITE_OK)
	{
		printf("error to open sqlite: %s\n", sqlite3_errmsg(db));
		return -1;
	}

	//向表中写入数据
	sq = sqlite3_mprintf("insert into temp values(%Q)", data);
	if ( (rv = sqlite3_exec(db, sq, NULL, NULL, &err_msg)) != SQLITE_OK)
	{
		printf("error to insert into temp: %s\n", err_msg);
		sqlite3_free(err_msg);
		rv = -2;
		goto cache_clean;
	}

cache_clean:
	sqlite3_free(sq);
	sqlite3_close(db);

	if (rv < 0)
	{
		return rv;
	}

	return 0;
}

//删除本地数据库数据
int delect_data_local(void)
{
	sqlite3              *db;
	char                 *err_msg;
	int                   rv = -1;
	char                 *sq;

	//打开数据库
	if ( (rv = sqlite3_open(CLIPATH, &db)) != SQLITE_OK)
	{
		printf("error to open sqlite: %s\n", sqlite3_errmsg(db));
		return -1;
	}

	//删除表
	sq = sqlite3_mprintf("drop table if exists temp");
	if ( (rv = sqlite3_exec(db, sq, NULL, NULL, &err_msg)) != SQLITE_OK)
	{
		printf("error to delect temp: %s\n", err_msg);
		sqlite3_free(err_msg);
		rv = -2;
		goto delect_clean;
	}

delect_clean:
	sqlite3_free(sq);
	sqlite3_close(db);

	if (rv < 0)
	{
		return rv;
	}

	return 0;
}
#endif

