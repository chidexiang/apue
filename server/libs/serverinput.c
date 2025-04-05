/*********************************************************************************
 *      Copyright:  (C) 2025 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  serverinput.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/04/25)
 *         Author:  LingYun <iot25@lingyun>
 *      ChangeLog:  1, Release initial version on "04/04/25 23:53:10"
 *                 
 ********************************************************************************/

#include "serverinput.h"
#include "log.h"

void server_input(int argc, char **argv, char **servip_t, int *port, char *progname)
{
	int                     ch;
	char                   *servip;

	struct option           opts[] = 
	{
		{"port", required_argument, NULL, 'p'},
		{"ipaddr", required_argument, NULL, 'i'},
		{"help", required_argument, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};

	*servip_t = malloc(100);

	while ((ch = getopt_long(argc, argv, "p:i:h", opts, NULL)) != -1)
	{
		switch(ch)
		{
			case 'p':
				*port = atoi(optarg);
				break;
			case 'i':
				servip = strcpy(servip, optarg);
				break;
			case 'h':
				print_usage(progname);
				exit(0);
			default:
				break;
		}
	}
	*servip_t = strdup(servip);

	if ( ! port)
	{
		log_error("need to input a port");
		print_usage(progname);
		exit(0);
	}
}

void print_usage(char *progname)
{
	log_info("%s usage: ", progname);

	log_info("-p(--port): sepcify server port.");
	log_info("-i(--ipaddr): sepcify server IP address");
	log_info("-h(--help): print this help information.");

	return ;
}
