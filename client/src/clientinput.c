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

void client_input(int argc, char **argv, socket_ctx_t *socket_ctx, char *progname, int *second)
{
	char                   *servip;
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

	while ( (ch = getopt_long(argc, argv, "i:p:d:ht:", opts, NULL)) != -1)
	{
		switch(ch)
		{
			case 'i':
				socket_ctx->servip = optarg;
				break;
			case 'p':
				socket_ctx->port = atoi(optarg);
				break;
			case 'h':
				print_usage(progname);
				exit(0);
				break;
			case 't':
				*second = atoi(optarg);
				break;
			default:
				break;
		}
	}

	if (! socket_ctx->servip || ! socket_ctx->port)
	{
		print_usage(progname);
		exit(0);
	}
	return ;
}

/*
void print_usage(char *progname)
{
	log_info("%s usage: \n", progname);

	log_info("-i(--ipaddr): sepcify server IP address\n");
	log_info("-p(--port): sepcify server port.\n");
	log_info("-h(--help): print this help information.\n");
	log_info("-d(--dmname): sepcify server domain name.\n");

	return ;
}
*/

int kill_daemon(const char *process_name) 
{
	pid_t pid = find_daemon_pid(process_name);
	if (pid == -1) 
	{
		fprintf(stderr, "Process '%s' not found\n", process_name);
		return -1;
	}
		                                 
	if (kill(pid, SIGINT) == -1)
	{
		fprintf(stderr, "kill SIGTERM failed\n");
		return -1;
	}
}

pid_t find_daemon_pid(const char *process_name) 
{
	char command[256];
	FILE *fp;
	pid_t pid = -1;

	snprintf(command, sizeof(command), "pgrep -x %s", process_name);
	fp = popen(command, "r");
	if (fp) 
	{
		if (fscanf(fp, "%d", &pid) != 1)
		{
			pid = -1;  // 未找到
		}
		pclose(fp);
	}
	else
	{
		pid = -1;
	}

	return pid;
}

int get_time(time_t start_time, char *time_str, size_t len)
{
	struct tm *local_time = localtime(&start_time);

	if ( ! strftime(time_str, len, "%Y-%m-%d %H:%M:%S", local_time))
	{
		return -1;
	}
	
	return 0;
}
