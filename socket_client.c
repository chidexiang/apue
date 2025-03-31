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

#include "ds18b20.h"

int init_local_db(char *path);
int cache_data_local(char *path, char *data);
int delect_data_local(char *path);

void print_usage(char *progname);
int setup_socket(int *sockfd, struct sockaddr_in *servaddr, char *servip, int *port);
void handle_disconnection(int *sockfd, struct sockaddr_in *servaddr, char *servip, int *port);

int                        g_error = 0;
int                        g_sqlite = 0;
float                      g_temp;
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
	int						timeout = 5;// 定时时长
	char                   *path = "client.db";

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
				timeout = atoi(optarg);
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

	//每一定时间上传数据
	for( ; ;)
	{
		//测试数据到服务器端
		rv = gettemp(&temp);
		memset(buf, 0, sizeof(buf));
		time(&rawtime);
		snprintf(buf, sizeof(buf), "ds18b20-%f-%s", temp, ctime(&rawtime));
		if ( (write(sockfd, buf, strlen(buf)) < 0) || ( g_error == 1))
		{
			printf("write to server failure: %s\n", strerror(errno));
			init_local_db(path);//初始化数据库
			cache_data_local(path, buf);//此处分多线程！！！！！！！！！！！！！！！！！！！
		    handle_disconnection(&sockfd, &servaddr, servip, &port);
			continue;
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

		sleep(timeout-2);
	}

cleanup:
	close(sockfd);
		return 0;
}

int setup_socket(int *sockfd, struct sockaddr_in *servaddr, char *servip, int *port)
{
	*sockfd = -1;

	//客户端SOCKET连接
	if ( (*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{
		printf("creat socket failure: %s\n", strerror(errno));
		return -1;
	}

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

	return 1;
}

void handle_disconnection(int *sockfd, struct sockaddr_in *servaddr, char *servip, int *port)
{
	int             delay;
	int             i;
	int             max = 5;

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
			g_sqlite = 1;
			return ;
		}
		i++;
	}
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

//数据库初始化
int init_local_db(char *path)
{
	sqlite3            *db;
	char               *err_msg = NULL;
	int	                rv = -1;
	char               *sq;

	//创建一个数据库
	if ( (rv = sqlite3_open(path, &db)) != SQLITE_OK)
	{
		printf("error to open sqlite: %s\n", sqlite3_errmsg(db));
		return -1;
	}

	//创建一个存放数据的表
	sq = sqlite3_mprintf("creat table temp(data char)");
	if ( (rv = sqlite3_exec(db, sq, NULL, NULL, err_msg)) != SQLITE_OK)
	{
		printf("error to creat table: %s\n", err_msg);
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
int cache_data_local(char *path, char *data)
{
	sqlite3             *db;
	char                *err_msg = NULL;
	int                  rv = -1;
	char                *sq;

	//打开数据库
	if ( (rv = sqlite3_open(path, &db)) != SQLITE_OK)
	{
		printf("error to open sqlite: %s\n", sqlite3_errmsg(db));
		return -1;
	}

	//向表中写入数据
	sq = sqlite3_mprintf("insert into temp values(%Q)", data);
	if ( (rv = sqlite3_exec(db, sq, NULL, NULL, err_msg)) != SQLITE_OK)
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
int delect_data_local(char *path)
{
	sqlite3              *db;
	char                 *err_msg;
	int                   rv = -1;
	char                 *sq;

	//打开数据库
	if ( (rv = sqlite3_open(path, &db)) != SQLITE_OK)
	{
		printf("error to open sqlite: %s\n", sqlite3_errmsg(db));
		return -1;
	}

	//删除表
	sq = sqlite3_mprintf("drop table if exists temp");
	if ( (rv = sqlite3_exec(db, sq, NULL, NULL, err_msg)) != SQLITE_OK)
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


