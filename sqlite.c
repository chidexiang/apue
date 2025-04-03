/*********************************************************************************
 *      Copyright:  (C) 2025 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  sqlite.c
 *    Description:  This file is sqlite
 *                 
 *        Version:  1.0.0(03/04/25)
 *         Author:  LingYun <iot25@lingyun>
 *      ChangeLog:  1, Release initial version on "03/04/25 15:34:37"
 *                 
 ********************************************************************************/

#include "sqlite.h"

//数据库初始化
int init_local_db(void)
{
	sqlite3            *db;
	char               *err_msg = NULL;
	int                 rv = -1; 
	char               *sq;

	//创建一个数据库
	if ( (rv = sqlite3_open(CLIPATH, &db)) != SQLITE_OK)
	{   
		printf("error to open sqlite: %s\n", sqlite3_errmsg(db));
		return -1; 
	}   
						
	//创建一个存放数据的表
	sq = sqlite3_mprintf("create table  if not exists temp(data char)");
	if ( (rv = sqlite3_exec(db, sq, NULL, NULL, &err_msg)) != SQLITE_OK)
	{   
		printf("error to create table: %s\n", err_msg);
		sqlite3_free(err_msg);
		rv = -2; 
		goto init_clean;
	}   
						
init_clean:
	sqlite3_free(sq);
	sqlite3_close(db);
						
	if (rv < 0)
	{   
		return rv; 
	}   
						
	return 0;
}

//数据存入本地数据库
int cache_data_local(char *data)
{
	sqlite3             *db;
	char                *err_msg = NULL;
	int                  rv = -1;
	char                *sq;

	//打开数据库
	if ( (rv = sqlite3_open(CLIPATH, &db)) != SQLITE_OK)
	{
		printf("error to open sqlite: %s\n", sqlite3_errmsg(db));
		return -1;
	}
						
	//向表中写入数据
	sq = sqlite3_mprintf("insert into temp values(%Q)", data);
	if ( (rv = sqlite3_exec(db, sq, NULL, NULL, &err_msg)) != SQLITE_OK)
	{
		printf("error to insert into temp: %s\n", err_msg);
		sqlite3_free(err_msg);
		rv = -2;
		goto cache_clean;
	}
						
cache_clean:
	sqlite3_free(sq);
	sqlite3_close(db);
						
	if (rv < 0)
	{
		return rv;
	}
						
	return 0;
}

//删除本地数据库数据
int delect_data_local(void)
{
	sqlite3              *db;
	char                 *err_msg;
	int                   rv = -1;
	char                 *sq;

	//打开数据库
	if ( (rv = sqlite3_open(CLIPATH, &db)) != SQLITE_OK)
	{
		printf("error to open sqlite: %s\n", sqlite3_errmsg(db));
		return -1;
	}
						
	//删除表
	sq = sqlite3_mprintf("drop table if exists temp");
	if ( (rv = sqlite3_exec(db, sq, NULL, NULL, &err_msg)) != SQLITE_OK)
	{
		printf("error to delect temp: %s\n", err_msg);
		sqlite3_free(err_msg);
		rv = -2;
		goto delect_clean;
	}
						
delect_clean:
	sqlite3_free(sq);
	sqlite3_close(db);
						
	if (rv < 0)
	{
		return rv;
	}
						
	return 0;
}
