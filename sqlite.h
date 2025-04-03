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

//使用这个文件必须在主函数文件中修改CLIPATH：
extern const char *CLIPATH;

int init_local_db(void);
int cache_data_local(char *data);
int delect_data_local(void);

#endif
