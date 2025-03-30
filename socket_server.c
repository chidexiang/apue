/*********************************************************************************
 *      Copyright:  (C) 2025 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  socket_server.c
 *    Description:  This file is server
 *                 
 *        Version:  1.0.0(30/03/25)
 *         Author:  LingYun <iot25@lingyun>
 *      ChangeLog:  1, Release initial version on "30/03/25 17:05:49"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 11008

int main(int argc, char **argv)
{
	int						listenfd;
	int						fd;
	struct sockaddr_in		cliaddr;
	socklen_t				len = sizeof(cliaddr);
	struct sockaddr_in		servaddr;
	int						rv = -1;
	int						on = 1;
	char					buf[1024];
	int						i;

	//服务器端socket连接
	if ( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socket failure: %s\n", strerror(errno));
		return -1;
	}

	//允许bind()地址重用
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	//设置服务器端信息
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	//绑定服务器端信息
	if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		printf("bind() failure: %s\n", strerror(errno));
		goto cleanup;
	}

	//服务器端socket转为被动模式
	if (listen(listenfd, 13) < 0)
	{
		printf("listen() failure: %s\n", strerror(errno));
		goto cleanup;
	}

	//与客户端建立连接
	for ( ; ; )
	{
		if ( (fd = accept(listenfd, (struct sockaddr *)&cliaddr, &len)) < 0)
		{
			printf("accept failure: %s\n", strerror(errno));
			goto cleanup;
		}

		//接受客户端发送数据
		memset(buf, 0, sizeof(buf));
		rv = read(fd, buf, sizeof(buf));
		if (rv < 0)
		{
			printf("read from client failure: %s\n", strerror(errno));
			goto cleanup;
		}
		else if (rv == 0)
		{
			printf("get disconnected\n");
			goto cleanup;
		}

		for (i=0;i<rv;i++)
		{
			buf[i] = toupper(buf[i]);
		}

		if( write(fd, buf, rv) < 0)
		{
			printf("write to client failure: %s\n", strerror(errno));
			goto cleanup;
		}
	}

cleanup:
	close(listenfd);
	close(fd);
	return 0;
}
