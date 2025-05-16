/*********************************************************************************
 *      Copyright:  (C) 2025 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  proc.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(16/05/25)
 *         Author:  LingYun <iot25@lingyun>
 *      ChangeLog:  1, Release initial version on "16/05/25 20:13:12"
 *                 
 ********************************************************************************/

#include "proc.h"

//类似daemon()函数的写法
void daemonize(int nochdir, int noclose)
{
	int rv, fd;
	int i;
			 
	/*   already a daemon */
	if (1 == getppid())
	{
		return ;
	}
				 
	/*   fork error */
	rv = fork();
	if (rv < 0)
	{
		exit(1);
	}
						 
	/*   parent process exit */
	if (rv > 0)
	{
		exit(0);
	}
							 
	/*   obtain a new process session group */
	setsid();
								 
	if (!noclose)
	{
		/*   close all descriptors */
		for (i = getdtablesize(); i >= 0; --i)
		{
			//if (i != g_logPtr->fd)
			close(i);
		}
																					                         
		/*   Redirect Standard input [0] to /dev/null */
		fd = open("/dev/null", O_RDWR);
																					                                          
		/*  Redirect Standard output [1] to /dev/null */
		dup(fd);
																					                                                           
		/*  Redirect Standard error [2] to /dev/null */
		dup(fd);
	}

	umask(0);

	if (!nochdir)
	{
		chdir("/");
	}

	return ;
}

int check_set_program_running(int daemon_id, char *pidfile)
{
	if ( !pidfile)
	{
		log_error("pidfile input failure\n");
		return -1;
	}

	if (check_running(pidfile))
	{
		log_error("process exist\n");
		return -2;
	}

	if (daemon_id)
	{
		daemon(1, 1);
		log_info("Program running as daemon [PID:%d].\n", getpid());

		if (record_pid(pidfile) < 0)
		{
			log_error("Record PID to file failure\n");
			return -3;
		}
	}
	else
	{
		if (record_pid(pidfile) < 0)
		{
			log_error("Record PID to file failure\n");
			return -4;
		}
	}

	return 0;
}

int check_running(char *pidfile)
{
	int           rv = -1;
	struct stat   file;
	pid_t         pid = -1;

	//将pidfile路径下的文件信息存入stat结构体中，成功返回0，失败返回-1
	rv = stat(pidfile, &file);
	if (rv == 0)
	{
		pid = get_process_pid(pidfile);

		if (pid > 0)
		{
			if (kill(pid, 0) == 0)
			{
				log_error("program running\n");
				return 1;
			}
			else
			{
				remove(pidfile);
				return 0;
			}
		}
		else if (pid == 0)
		{
			remove(pidfile);
			return 0;
		}
		else
		{
			log_error("Read record file failure, maybe program still running.\n");
			return 1;
		}
	}
	else
	{
		return 0;
	}
}

pid_t get_process_pid(char *pidfile)
{
	FILE      *fp;
	char       pid_str[16];
	pid_t      pid;

	if ((fp = fopen(pidfile, "r")) != NULL)
	{
		if (fgets(pid_str, sizeof(pid_str), fp) == NULL)
		{
			log_error("get pid failure\n");
			fclose(fp);
			return -1;
		}

		pid_str[strcspn(pid_str, "\n")] = '\0';

		pid = atoi(pid_str);

		fclose(fp);
	}
	else
	{
		log_error("open pidfile failure\n");
		return -1;
	}

	return pid;
}

int record_pid(char *pidfile)
{
	struct stat      file;
	int              fd = -1;
	int              mode = S_IROTH | S_IXOTH | S_IRGRP | S_IXGRP | S_IRWXU;
	char             ipc_dir[64] = { 0 };
	char             pid[16];
				 
	strncpy(ipc_dir, pidfile, 64);
					 
	//将路径中的文件名去掉
	dirname(ipc_dir);
						 
	//路径不存在则创建
	if (stat(ipc_dir, &file) < 0)
	{
		if (mkdir(ipc_dir, mode) < 0)
		{
			log_error("cannot create %s: %s\n", ipc_dir, strerror(errno));
			return -1;
		}
		
		//忽略返回值，防止umask影响权限
		(void)chmod(ipc_dir, mode);
	}
							 
	//创建pid文件
	mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	if ((fd = open(pidfile, O_RDWR | O_CREAT | O_TRUNC, mode)) >= 0)
	{
		snprintf(pid, sizeof(pid), "%u\n", (unsigned)getpid());
		if (write(fd, pid, strlen(pid)) <= 0)
		{
			log_error("failed to write PID\n");
			close(fd);
			return -2;
		}
		close(fd);
																				 
		log_debug("Record PID<%u> to file %s.\n", getpid(), pidfile);
	}
	else
	{
		log_error("cannot create %s: %s\n", pidfile, strerror(errno));
		return -3;
	}
										 
	return 0;
}
