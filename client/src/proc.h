/********************************************************************************
 *      Copyright:  (C) 2025 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  proc.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(16/05/25)
 *         Author:  LingYun <iot25@lingyun>
 *      ChangeLog:  1, Release initial version on "16/05/25 20:11:16"
 *                 
 ********************************************************************************/

#ifndef _PROC_H_
#define _PROC_H_

#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
 
#include "logger.h"

void daemonize(int nochdir, int noclose);
int check_set_program_running(int daemon, char *pidfile);
int check_running(char *pidfile);
pid_t get_process_pid(char *pidfile);
int record_pid(char *pidfile);


#endif
