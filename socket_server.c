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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <libgen.h>

#include "sqlite.h"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

const char                 *CLIPATH = "server.db";

void print_usage(char *progname);
int socket_server_init(char *listen_ip, int port);

int main(int argc, char **argv)
{
	int						listenfd;
	int                     connfd;
	int						fd;
	struct sockaddr_in		cliaddr;
	socklen_t				len = sizeof(cliaddr);
	struct sockaddr_in		servaddr;
	int						rv = -1;
	int						on = 1;
	int                     maxfd = 0;
	int                     port = 0;
	int                     found;
	char                   *progname;
	char					buf[1024];
	int						i,j;
	int                     fds_array[1024];
	int                     ch;
	fd_set                  rdset;

	struct option           opts[] = 
	{
		{"port", required_argument, NULL, 'p'},
		{"help", required_argument, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};

	progname = basename(argv[0]);

	while ((ch = getopt_long(argc, argv, "p:h", opts, NULL)) != -1)
	{
		switch(ch)
		{
			case 'p':
				port = atoi(optarg);
				break;
			case 'h':
				print_usage(progname);
				return 0;
			default:
				break;
		}
	}

	if ( ! port)
	{
		printf("need to input a port\n");
		print_usage(progname);
		return -1;
	}

	if ((listenfd = socket_server_init(NULL, port)) < 0)
	{
		printf("server listen on %d port failure\n", port);
		return -2;
	}

	printf("server start to listen on port %d\n", port);

	for (i=0;i<ARRAY_SIZE(fds_array);i++)
	{
		fds_array[i] = -1;
	}
	fds_array[0] = listenfd;

	for ( ; ; )
	{
		FD_ZERO(&rdset);
		for (i=0;i<ARRAY_SIZE(fds_array);i++)
		{
			if (fds_array[i] < 0)
			{
				continue;
			}

			maxfd = maxfd > fds_array[i] ? maxfd : fds_array[i];
			FD_SET(fds_array[i], &rdset);
		}

		rv = select(maxfd+1, &rdset, NULL, NULL, NULL);
		if (rv < 0)
		{
			printf("select failure: %s\n", strerror(errno));
			break;
		}
		else if (rv == 0)
		{
			printf("timeout\n");
			continue;
		}

		for (i=0;i<rv;i++)
		{

			if (FD_ISSET(listenfd, &rdset))
			{
				if ((connfd = accept(listenfd, (struct sockaddr *)NULL, NULL)) < 0)
				{
					printf("accept new client failure: %s\n", strerror(errno));
					continue;
				}

				found = 0;
				for (i=1;i<ARRAY_SIZE(fds_array);i++)
				{
					if (fds_array[i] < 0)
					{
						printf("accept new client[%d] \n", connfd);
						fds_array[i] = connfd;
						found = 1;
						break;
					}
				}

				if ( ! found)
				{
					printf("failure to add new client because full\n");
					close(connfd);
				}
			}
			else
			{
				for (i=0;i<ARRAY_SIZE(fds_array);i++)
				{
					if (fds_array[i] < 0 || !FD_ISSET(fds_array[i], &rdset))
					{
						continue;
					}

					memset(buf, 0, sizeof(buf));
					if ((rv = read(fds_array[i], buf, sizeof(buf))) <= 0)
					{
						printf("socket[%d] read failure or get disconncet\n", fds_array[i]);
						close(fds_array[i]);
						fds_array[i] = -1;
					}
					printf("socket[%d] read: %s\n", fds_array[i], buf);
					printf("send data to sqlite\n");
					init_local_db();//初始化数据库
					cache_data_local(buf);//数据存入数据库
				}
			}
		}
	}

	close(listenfd);
	return 0;
}

int socket_server_init(char *listen_ip, int port)
{
	struct sockaddr_in       servaddr;
	int                      rv = 0;
	int                      on = 1;
	int                      listenfd;

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socket failure: %s\n", strerror(errno));
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
			printf("set listen ip failure\n");
			rv = -2;
			goto cleanup;
		}
	}

	//绑定服务器端信息
	if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
	    printf("bind() failure: %s\n", strerror(errno));
		rv = -3;
	    goto cleanup;
	}
	
	//服务器端socket转为被动模式
	if (listen(listenfd, 13) < 0)
	{
	    printf("listen() failure: %s\n", strerror(errno));
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

void print_usage(char *progname)
{
	printf("%s usage: \n", progname);

	printf("-p(--port): sepcify server port.\n");
	printf("-h(--help): print this help information.\n");

	return ;
}

