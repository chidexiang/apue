/*********************************************************************************
 *      Copyright:  (C) 2025 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  clientinput.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/04/25)
 *         Author:  LingYun <iot25@lingyun>
 *      ChangeLog:  1, Release initial version on "04/04/25 20:44:20"
 *                 
 ********************************************************************************/

#include "clientinput.h"
#include "log.h"

int g_timeout = 5;

void client_input(int argc, char **argv, char **servip_t, int *port, char *progname)
{
	const char             *hostname;
	char                   *servip;
	struct addrinfo         hints, *res, *p;
	int                     status = 1;
	struct sockaddr_in     *addr;
	struct option           opts[] = 
	{
		{"ipaddr", required_argument, NULL, 'i'},
		{"port", required_argument, NULL, 'p'},
		{"dmname", required_argument, NULL, 'd'},
		{"help", no_argument, NULL, 'h'},
		{"time", required_argument, NULL, 't'},
		{NULL, 0, NULL, 0}
	};
    int                     ch;

	*servip_t = malloc(100);

	while ( (ch = getopt_long(argc, argv, "i:p:d:ht:", opts, NULL)) != -1)
	{
		switch(ch)
		{
			case 'i':
				servip = strcpy(servip, optarg);
				break;
			case 'p':
				*port = atoi(optarg);
				break;
			case 'h':
				print_usage(progname);
				exit(0);
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
					log_error("getaddrinfo error: %s", strerror(errno));
					exit(0);
				}

				//遍历结果链表
				for (p = res; p != NULL; p = p->ai_next)
				{
					addr = (struct sockaddr_in *)p->ai_addr;
					servip = inet_ntoa(addr->sin_addr);
					log_error("IP address: %s", inet_ntoa(addr->sin_addr));
				}

				break;
			default:
				break;
		}
	}
	*servip_t = strdup(servip);
	
	if (! servip || ! port)
	{
		print_usage(progname);
		exit(0);
	}
	return ;
}

void print_usage(char *progname)
{
	log_info("%s usage: ", progname);

	log_info("-i(--ipaddr): sepcify server IP address");
	log_info("-p(--port): sepcify server port.");
	log_info("-h(--help): print this help information.");
	log_info("-d(--dmname): sepcify server domain name.");

	return ;
}
