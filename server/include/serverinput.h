/********************************************************************************
 *      Copyright:  (C) 2025 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  serverinput.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(05/04/25)
 *         Author:  LingYun <iot25@lingyun>
 *      ChangeLog:  1, Release initial version on "05/04/25 00:02:30"
 *                 
 ********************************************************************************/

#ifndef _SERVERINPUT_H
#define _SERVERINPUT_H

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

void server_input(int argc, char **argv, char **servip_t, int *port, char *progname);
void print_usage(char *progname);

#endif
