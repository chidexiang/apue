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

int socket_connect(socket_ctx_t *socket_ctx)
{
	int                     keepalive = 1;
	int                     keepalive_idle = 7200;//空闲7200s后开始探测
	int                     keepalive_interval = 75;//探测包每隔75秒发送一次
	int                     keepalive_count = 9;//最多发送9个探测包
	struct addrinfo         hints, *res, *p;
	struct in_addr          inaddr;
	int                     status = 1;
	char                    port_str[10];
	struct sockaddr_in      addr;
	int                     len = sizeof(addr);
	int                     rv = 1;

	if ( !socket_ctx)
	{
		return -1;
	}

	close(socket_ctx->sockfd);
	socket_ctx->sockfd = -1;

	//服务器端数据更新
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if (inet_aton(socket_ctx->servip, &inaddr) ) //输入为IP地址
	{
		hints.ai_flags |= AI_NUMERICHOST;
	}

	snprintf(port_str, sizeof(port_str), "%d", socket_ctx->port);

	//DNS解析
	if ((status = getaddrinfo(socket_ctx->servip, port_str, &hints, &res)) != 0)
	{
		log_error("getaddrinfo error: %s\n", strerror(errno));
		rv = -1;
	}

	//遍历结果链表
	for (p = res; p != NULL; p = p->ai_next)
	{
		//客户端SOCKET连接
		if ( (socket_ctx->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
		{
			log_error("creat socket failure: %s\n", strerror(errno));
			rv = -2;
			continue;
		}

		if ( connect(socket_ctx->sockfd, p->ai_addr, p->ai_addrlen) < 0 )
		{
			close(socket_ctx->sockfd);
			socket_ctx->sockfd = -1;
			rv = -3;
			continue;
		}
		else
		{
			log_debug("connect [%d] success\n", socket_ctx->sockfd);
			break;
		}
	}

	freeaddrinfo(res);

	if (rv > 0)
	{
		if (setsockopt(socket_ctx->sockfd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)) < 0)
		{    
			log_error("set keepalive failure\n");
		}                        
		if (setsockopt(socket_ctx->sockfd, IPPROTO_TCP, TCP_KEEPIDLE, &keepalive_idle, sizeof(keepalive_idle)) < 0)
		{    
			log_error("set keepalive_idle failure\n");
		}                             
		if (setsockopt(socket_ctx->sockfd, IPPROTO_TCP, TCP_KEEPINTVL, &keepalive_interval, sizeof(keepalive_interval)) < 0)
		{    
			log_error("set keepalive_interval failure\n");
		}                                 
		if (setsockopt(socket_ctx->sockfd, IPPROTO_TCP, TCP_KEEPCNT, &keepalive_count, sizeof(keepalive_count)) < 0)
		{    
			log_error("set keepalive_count failure\n");
		}
	}

	return rv;
}

/*
void handle_disconnection(struct sockaddr_in *servaddr, socket_ctx_t *socket_ctx)
{
	int             delay;
	int             i;
	int             max = 5;

	close(socket_ctx->sockfd);
	socket_ctx->sockfd = -1;

	i = 0;
	while (1)
	{
		i = i < max ? i : max;
		delay = 1 * pow(2, i);
		log_info("attempting to reconnect in %d second...\n", delay);
		sleep(delay);

		socket_connect(servaddr, socket_ctx);

		log_info("sockfd: %d\n", socket_ctx->sockfd);

		if (socket_ctx->sockfd != -1)
		{
			log_info("Reconnect to server!\n");
			return ;
		}
		i++;
	}
}
*/

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

int socket_status(socket_ctx_t *sock)
{
	struct tcp_info         info;
	int                     len = sizeof(info);
	int                     changed = 0;

	if( !sock )
	{
		return 0;
	}
	 
	if( sock->sockfd < 0 )
	{
		changed = sock->connected ? 1 : 0;
		sock->connected = 0;
		goto out;
	}

	getsockopt(sock->sockfd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
	if (info.tcpi_state == TCP_ESTABLISHED)
	{
		changed = !sock->connected ? 1 : 0;
		sock->connected = 1;
	}
	else
	{
		changed = sock->connected ? 1 : 0;
		sock->connected = 0;
	}

out:
	if( changed )
	{
		log_info("socket status got %s\n", sock->connected?"connected":"disconnected");
	}
	return sock->connected;
}
