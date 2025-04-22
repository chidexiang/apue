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
#include "logger.h"

//数据库初始化
int init_local_db(char *file, sqlite3 **db)
{
	char               *err_msg = NULL;
	int                 rv = -1; 
	char               *sq;

	//创建一个数据库
	if ( (rv = sqlite3_open(file, db)) != SQLITE_OK)
	{   
		log_debug("error to open sqlite: %s\n", sqlite3_errmsg(*db));
		return -1; 
	}   
						
	//创建一个存放数据的表
	sq = sqlite3_mprintf("create table if not exists temp(data char)");
	if ( (rv = sqlite3_exec(*db, sq, NULL, NULL, &err_msg)) != SQLITE_OK)
	{   
		log_debug("error to create table: %s\n", err_msg);
		sqlite3_free(err_msg);
		rv = -2; 
		goto init_clean;
	}   
						
init_clean:
	sqlite3_free(sq);
						
	if (rv < 0)
	{   
		return rv; 
	}   
						
	return 0;
}

//数据存入本地数据库
int cache_data_local(char *data, sqlite3 *db)
{
	char                *err_msg = NULL;
	int                  rv = -1;
	char                *sq;

	//向表中写入数据
	sq = sqlite3_mprintf("insert into temp values(%Q)", data);
	if ( (rv = sqlite3_exec(db, sq, NULL, NULL, &err_msg)) != SQLITE_OK)
	{
		log_debug("error to insert into temp: %s\n", err_msg);
		sqlite3_free(err_msg);
		rv = -2;
		goto cache_clean;
	}
						
cache_clean:
	sqlite3_free(sq);
						
	if (rv < 0)
	{
		return rv;
	}
						
	return 0;
}

//删除本地数据库数据
int delete_data_local(sqlite3 *db)
{
	char                 *err_msg;
	int                   rv = -1;
	char                 *sq;

	//删除表
	sq = sqlite3_mprintf("drop table if exists temp");
	if ( (rv = sqlite3_exec(db, sq, NULL, NULL, &err_msg)) != SQLITE_OK)
	{
		log_debug("error to delect temp: %s\n", err_msg);
		sqlite3_free(err_msg);
		rv = -2;
		goto delect_clean;
	}
						
delect_clean:
	sqlite3_free(sq);
						
	if (rv < 0)
	{
		return rv;
	}
						
	return 0;
}

//删除本地数据库第一条数据
int delect_1st_data_local(sqlite3 *db)
{
	char                 *err_msg;
	int                   rv = -1;
	char                 *sq;

	//删除表
	sq = sqlite3_mprintf("delete from temp where rowid = (select rowid from temp limit 1)");
	if ( (rv = sqlite3_exec(db, sq, NULL, NULL, &err_msg)) != SQLITE_OK)
	{
		log_debug("error to delect first temp: %s\n", err_msg);
		sqlite3_free(err_msg);
		rv = -2;
		goto delect_clean;
	}
						
delect_clean:
	sqlite3_free(sq);
						
	if (rv < 0)
	{
		return rv;
	}
						
	return 0;
}

//读取数据库的全部数据
int send_data_local(char *buf, sqlite3 *db)
{
	char           *err_msg;
	int             rv = -1;
		
	if ((rv = sqlite3_exec(db, "select * from temp", send_callback, buf, &err_msg)) != SQLITE_OK)
	{
		log_debug("error to read sqlite: %s\n", err_msg);
		sqlite3_free(err_msg);
		rv = -2;
		goto send_clean;
	}

send_clean:

	if (rv < 0)
	{
		return rv;
	}

	return 0;
}

int send_callback(void *buf, int f_num, char **f_value, char **f_name)
{
	char          date[512];

	memset(date, 0, sizeof(date));
	snprintf(date, sizeof(date), "%s", f_value[0]);

	strncpy((char *)buf, date, strlen(date));

	return 0;
}

//读取数据库的第一条数据
int send_1st_data_local(char *buf, sqlite3 *db)
{
	char           *err_msg;
	int             rv = -1;
		
	if ((rv = sqlite3_exec(db, "select * from temp limit 1", send_callback, buf, &err_msg)) != SQLITE_OK)
	{
		log_debug("error to read sqlite: %s\n", err_msg);
		sqlite3_free(err_msg);
		rv = -2;
		goto send_clean;
	}

send_clean:

	if (rv < 0)
	{
		return rv;
	}

	return 0;
}

//查询是否还有数据
int find_data_local(sqlite3 *db)
{
	sqlite3_stmt   *stmt;
	const char     *sql = "select * from temp limit 1;";
	int             rv = -1;
	
	//准备数据库执行代码
	rv = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	if (rv == SQLITE_OK) 
	{
		//查询temp中是否有数据
		if (sqlite3_step(stmt) == SQLITE_ROW)
		{
			rv = 1;
			goto find_clean;
		}
		else
		{
			rv = 0;
			goto find_clean;
		}
	}
	else
	{
		rv = -2;
		log_debug("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
		goto find_clean;
	}
find_clean:
	sqlite3_finalize(stmt);

    return rv;
}

void close_local_db(sqlite3 **db)
{
	sqlite3_close(*db);
	*db = NULL;
	return ;
}
