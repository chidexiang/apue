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

#define PORT 11008
#define IP "127.0.0.1"
#define MSG_STR "connect success"

int main(int argc, char **argv)
{
	int						confd;
	int						rv = -1;
	struct sockaddr_in		servaddr;
	char 					buf[1024];
	
	//客户端SOCKET连接
	if ( (confd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{
		printf("creat socket failure: %s\n", strerror(errno));
		return -1;
	}

	//服务器端数据更新
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	if (inet_pton(AF_INET, IP, &servaddr.sin_addr) != 1)
	{
		printf("add servaddr ip failure\n");
		return -2;
	}

	//conncet连接
	if (connect(confd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		printf("connect server failure: %S\n", strerror(errno));
		return -3;
	}

	//测试传输数据到服务器端
	if (write(confd, MSG_STR, strlen(MSG_STR)) < 0)
	{
		printf("write to server failure: %s\n", strerror(errno));
		goto cleanup;
	}

	//测试接受服务器端数据
	rv = read(confd, buf, sizeof(buf));
	if (rv < 0)
	{
		printf("read from server failure: %s\n", strerror(errno));
		goto cleanup;
	}
	else if(rv == 0)
	{
		printf("get disconnected\n");
		goto cleanup;
	}
	printf("read %d from server: %s\n", rv, buf);

cleanup:
	close(confd);
	return 0;
}
