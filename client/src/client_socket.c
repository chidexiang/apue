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

int socket_connect(int *sockfd, struct sockaddr_in *servaddr, char *servip, int *port)
{
	int                     keepalive = 1;
	int                     keepalive_idle = 3;//空闲3s后开始探测
	int                     keepalive_interval = 1;//探测包每隔1秒发送一次
	int                     keepalive_count = 1;//最多发送1个探测包
	const char             *hostname;
	struct addrinfo         hints, *res, *p;
	struct sockaddr_in     *addr;
	int                     status = 1;
	char                    port_str[6];

	*sockfd = -1;

	//客户端SOCKET连接
	if ( (*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{
		log_error("creat socket failure: %s\n", strerror(errno));
		return -1;
	}

	//服务器端数据更新
	memset(servaddr, 0, sizeof(*servaddr));
	servaddr->sin_family = AF_INET;
	servaddr->sin_port = htons(*port);
	if (inet_pton(AF_INET, servip, &servaddr->sin_addr) != 1)//void *
	{
		hostname = servip;
		snprintf(port_str, sizeof(port_str), "%d", *port);

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET; //支持IPV4
		hints.ai_socktype = SOCK_STREAM; //TCP

		//DNS解析
		if((status = getaddrinfo(hostname, port_str, &hints, &res)) != 0)
		{
			log_error("getaddrinfo error: %s\n", strerror(errno));
			exit(0);
		}

		//遍历结果链表
		for (p = res; p != NULL; p = p->ai_next)
		{
			if (connect(*sockfd, p->ai_addr, p->ai_addrlen) < 0)
			{
				close(*sockfd);
				*sockfd = -1;
				continue;
			}
			else
			{
				log_debug("connect [%d] success\n", *sockfd);
				freeaddrinfo(res); 
				break;
			}
			return -3;
		}
	}
	else
	{
		//conncet连接
		if (connect(*sockfd, (struct sockaddr *)servaddr, sizeof(*servaddr)) < 0)
		{
			close(*sockfd);
			*sockfd = -1;
			return -3;
		}
		log_debug("connect [%d] success\n", *sockfd);
	}

	if (*sockfd)
	{
		if (setsockopt(*sockfd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)) < 0)
		{    
			log_error("set keepalive failure\n");
		}                        
		if (setsockopt(*sockfd, IPPROTO_TCP, TCP_KEEPIDLE, &keepalive_idle, sizeof(keepalive_idle)) < 0)
		{    
			log_error("set keepalive_idle failure\n");
		}                             
		if (setsockopt(*sockfd, IPPROTO_TCP, TCP_KEEPINTVL, &keepalive_interval, sizeof(keepalive_interval)) < 0)
		{    
			log_error("set keepalive_interval failure\n");
		}                                 
		if (setsockopt(*sockfd, IPPROTO_TCP, TCP_KEEPCNT, &keepalive_count, sizeof(keepalive_count)) < 0)
		{    
			log_error("set keepalive_count failure\n");
		}
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
		log_info("attempting to reconnect in %d second...\n", delay);
		sleep(delay);

		setup_socket(sockfd, servaddr, servip, port);

		log_info("sockfd: %d\n", *sockfd);

		if (*sockfd != -1)
		{
			log_info("Reconnect to server!\n");
			return ;
		}
		i++;
	}
}

int is_empty(char arr[], int size) 
{
	int           i = 0;

	for (i = 0;i < size; i++) 
	{
		if (arr[i] != 0) 
		{ // 假设默认值为 0
			return 0;   // 发现非零元素，数组非空
		}
	}
		    return 1; // 所有元素均为 0，视为“空”
}

