/********************************************************************************
 *      Copyright:  (C) 2025 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  clientinput.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(04/04/25)
 *         Author:  LingYun <iot25@lingyun>
 *      ChangeLog:  1, Release initial version on "04/04/25 20:54:59"
 *                 
 ********************************************************************************/

#ifndef _CLIENTINPUT_H
#define _CLIENTINPUT_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <getopt.h>
#include <netdb.h>
#include <libgen.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <pthread.h>
#include <sqlite3.h>
#include <netinet/tcp.h>

#include "logger.h"
#include "client_socket.h"

void client_input(int argc, char **argv, socket_ctx_t *socket_ctx, char *progname, int *second);
void print_usage(char *progname);
int kill_daemon(const char *process_name);
pid_t find_daemon_pid(const char *process_name);
int time_print(time_t start_time, char *time_str, size_t len);

#endif
