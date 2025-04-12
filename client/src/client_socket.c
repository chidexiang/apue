/*********************************************************************************
 *      Copyright:  (C) 2025 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  client_socket.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/04/25)
 *         Author:  LingYun <iot25@lingyun>
 *      ChangeLog:  1, Release initial version on "04/04/25 19:55:46"
 *                 
 ********************************************************************************/

#include "client_socket.h"
#include "log.h"
#include "sqlite.h"

int g_sock_time = 1;

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
		log_error("creat socket failure: %s", strerror(errno));
		return -1;
	}

	//服务器端数据更新
	memset(servaddr, 0, sizeof(*servaddr));
	servaddr->sin_family = AF_INET;
	servaddr->sin_port = htons(*port);
	if (inet_pton(AF_INET, servip, &servaddr->sin_addr) != 1)//void *
	{
		log_error("add servaddr ip failure");
		return -2;
	}

	//conncet连接
	if (connect(*sockfd, (struct sockaddr *)servaddr, sizeof(*servaddr)) < 0)
	{
		log_error("connect server failure: %s", strerror(errno));
		close(*sockfd);
		*sockfd = -1;
		return -3;
	}

	if (setsockopt(*sockfd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)) < 0)
	{    
		log_error("set keepalive failure");
	}                        
	if (setsockopt(*sockfd, IPPROTO_TCP, TCP_KEEPIDLE, &keepalive_idle, sizeof(keepalive_idle)) < 0)
	{    
		log_error("set keepalive_idle failure");
	}                             
	if (setsockopt(*sockfd, IPPROTO_TCP, TCP_KEEPINTVL, &keepalive_interval, sizeof(keepalive_interval)) < 0)
	{    
		log_error("set keepalive_interval failure");
	}                                 
	if (setsockopt(*sockfd, IPPROTO_TCP, TCP_KEEPCNT, &keepalive_count, sizeof(keepalive_count)) < 0)
	{    
		log_error("set keepalive_count failure");
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
		log_info("attempting to reconnect in %d second...", delay);
		sleep(delay);

		setup_socket(sockfd, servaddr, servip, port);

		log_info("sockfd: %d", *sockfd);

		if (*sockfd != -1)
		{
			log_info("Reconnect to server!");
			return ;
		}
		i++;
	}
}
