/********************************************************************************
 *      Copyright:  (C) 2025 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  sqlite.h
 *    Description:  This file is sqlite.h
 *
 *        Version:  1.0.0(03/04/25)
 *         Author:  LingYun <iot25@lingyun>
 *      ChangeLog:  1, Release initial version on "03/04/25 15:55:42"
 *                 
 ********************************************************************************/

#ifndef _SQLITE_H
#define _SQLITE_H

#include <stdio.h>
#include <sqlite3.h>
#include <unistd.h>
#include <string.h>

#include "logger.h"

int init_local_db(char *file, sqlite3 **db);//数据库初始化并创建temp表
int cache_data_local(char *data, sqlite3 *db);//数据存入temp表
int delete_data_local(sqlite3 *db);//删除temp表
int send_data_local(char *buf, sqlite3 *db);//读取数据库中的数据并发送
int send_callback(void *buf, int f_num, char **f_value, char **f_name);//回调函数
int find_data_local(sqlite3 *db);//查询数据库中是否还有数据
int send_1st_data_local(char *buf, sqlite3 *db);//读取数据库的第一条数据并发送
int delete_1st_data_local(sqlite3 *db);//删除temp表的第一条数据
void close_local_db(sqlite3 **db);//关闭数据库

#endif
