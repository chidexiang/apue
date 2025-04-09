/********************************************************************************
 *      Copyright:  (C) 2025 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  client_socket.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(04/04/25)
 *         Author:  LingYun <iot25@lingyun>
 *      ChangeLog:  1, Release initial version on "04/04/25 20:30:31"
 *                 
 ********************************************************************************/

#ifndef _CLIENT_SOCKET_H
#define _CLIENT_SOCKET_H

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

extern int      g_sock_time;

int setup_socket(int *sockfd, struct sockaddr_in *servaddr, char *servip, int *port);
void handle_disconnection(int *sockfd, struct sockaddr_in *servaddr, char *servip, int *port);

#endif
