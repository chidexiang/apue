/*********************************************************************************
 *      Copyright:  (C) 2025 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  serverinit.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/04/25)
 *         Author:  LingYun <iot25@lingyun>
 *      ChangeLog:  1, Release initial version on "04/04/25 22:35:12"
 *                 
 ********************************************************************************/

#include "serverinit.h"
#include "log.h"

int socket_server_init(char *listen_ip, int port)
{
	struct sockaddr_in       servaddr;
	int                      rv = 0;
	int                      on = 1;
	int                      listenfd;

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		log_error("socket failure: %s", strerror(errno));
		return -1;
	}

	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	//设置服务器端信息
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);

	if ( ! listen_ip)
	{
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		if (inet_pton(AF_INET, listen_ip, &servaddr.sin_addr) <= 0)
		{
			log_error("set listen ip failure\n");
			rv = -2;
			goto cleanup;
		}
	}

	//绑定服务器端信息
	if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
	    log_error("bind() failure: %s\n", strerror(errno));
		rv = -3;
	    goto cleanup;
	}
	
	//服务器端socket转为被动模式
	if (listen(listenfd, 13) < 0)
	{
	    log_error("listen() failure: %s\n", strerror(errno));
		rv = -4;
	    goto cleanup;
	}

cleanup:
	if (rv < 0)
	{
		close(listenfd);
	}
	else
	{
		rv = listenfd;
	}

	return rv;
}
