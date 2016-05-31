#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include "sqlite3_public.h"

#define DB_FILE_PATH "./test.db"

static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	int i;
	int *p_total = (int *)NotUsed;

	for (i = 0; i < argc; i++)
	{
		printf("[%d]-->%s = %s\n", i, azColName[i], argv[i] ? argv[i] : "NULL");
	}

	if (argv[0] != NULL && NULL != p_total)
	{
		*p_total = atoi(argv[0]);
	}

	return 0;
}

static unsigned int  db_get_currentTime(void)
{
	struct timeval tv;
	struct timezone tz;
	unsigned int  ultime;

	gettimeofday(&tv , &tz);
	ultime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	return (ultime);
}

static int db_print_errMsg(char *cmd, char **err_msg)
{
	printf("cmd=%s  <---> Error=%s\n", cmd, *err_msg);
	sqlite3_free(*err_msg);
	*err_msg = NULL;
	return 0;
}

static int db_exec_selectSQL(sqlite3 *db, char *cmd, char ***azResult, int *nrow, int *ncolumn)
{
	if (NULL == cmd || !db)
	{
		printf("Error find_type=%p s_db=%p\n", cmd, db);
		return -11;
	}

	int db_ret = 0;
	char *zErrMsg = NULL;

	db_ret = sqlite3_get_table(db , cmd , azResult , nrow , ncolumn , &zErrMsg);
	if (db_ret != SQLITE_OK)
	{
		db_print_errMsg(cmd, &zErrMsg);
		return -1;
	}

	if (0 == *nrow && 0 == *ncolumn)
	{
		printf("nrow =%d ncolumn=%d\n", *nrow, *ncolumn);
		return -2;
	}

	return 0;
}


static int db_find_elem(sqlite3 *db, char *sql, char ***reslut, int *row, int *col)
{
	int ret = 0;
	char **pp_reslut = NULL;
	ret = db_exec_selectSQL(db, sql, &pp_reslut, row, col);
	if (ret != 0)
	{
		printf("Failed to Cmd=%s!!\n", sql);
		return -1;
	}
	
	*reslut = pp_reslut;
	return ret;
}

int db_output_struct(DataBase_t *info, char **buff, int pos)
{
	if (pos < 0 || !buff || !info)
	{
		printf("ERROR;pos=%d,buff=%p", pos, buff);
		return -1;
	}

	snprintf(info->FileName, sizeof(info->FileName), "%s", buff[pos++]);
	info->MovieSize = atoll(buff[pos++]);
	snprintf(info->RcdStartTime, sizeof(info->RcdStartTime), "%s", buff[pos++]);
	snprintf(info->Notes, sizeof(info->Notes), "%s", buff[pos++]);
	info->RcdTimeLenght = atoi(buff[pos++]);
	info->DownloadCnt = atoi(buff[pos++]);
	info->FTPupload = atoi(buff[pos++]);
	info->BackupFTPload = atoi(buff[pos++]);
	info->CourseCompleteStatus = atoi(buff[pos++]);
	snprintf(info->CourseTeacher, sizeof(info->CourseTeacher), "%s", buff[pos++]);
	snprintf(info->CourseSubject, sizeof(info->CourseSubject), "%s", buff[pos++]);
	info->Reserve1 = sizeof(buff[pos++]);
	info->Reserve2 = sizeof(buff[pos++]);
	info->Reserve3 = sizeof(buff[pos++]);
	return 0;
}


int db_sqlite_init(sqlite3 **db)
{
	int db_ret = 0;
	char *zErrMsg = 0;
	int have_db = 0;

	if (0 == access(DB_FILE_PATH, 0))
	{
		have_db = 1;
		printf("%s is Exist!!!\n", DB_FILE_PATH);
	}

	/*open database handler*/
	db_ret = sqlite3_open(DB_FILE_PATH, db);
	if (db_ret < 0)
	{
		if (*db)
		{
			printf("Can't open database: %s\n", sqlite3_errmsg(*db));
			sqlite3_close(*db);
		}
		else
		{
			printf("db=%p\n", *db);
		}

		return -1;
	}

	if (0 == have_db)
	{
		char *create_sql = CREATE_DATABASE_SQL;

		//使用sql字符串指定的sql语言 创建database
		db_ret = sqlite3_exec(*db , create_sql , 0 , callback , &zErrMsg);
		printf("\033[32m" "sqlite3_exec open db_ret=%d\n""\033[0m", db_ret);
		if (db_ret < 0)
		{
			printf("sqlite3_exec open failed!!!db_ret=%d\n", db_ret);
			return -1;
		}
	}

	printf("Init DB finished!!!!\n\n");
	return 0;
}


int db_insert_elem(sqlite3 * db, DataBase_t file_info)
{
	if (db == NULL)
	{
		printf("db_insert_elem is param is err!!\n");
		return -1;
	}

	char cmd[1024] = {0};
	int ret = -1;
	char *zErrMsg = NULL;
	char file_size[64] = {0};
	
	//产生唯一key值
	unsigned int db_key = db_get_currentTime() + rand() % 10;
	sprintf(file_size, "%lld", file_info.MovieSize);
	sprintf(cmd, INSERT_INTODB_SQL, db_key, file_info.FileName, file_size, file_info.RcdStartTime,\
		file_info.Notes, file_info.RcdTimeLenght,file_info.DownloadCnt, file_info.FTPupload, \
		file_info.BackupFTPload, file_info.CourseCompleteStatus, file_info.CourseTeacher, \
		file_info.CourseSubject, file_info.Reserve1, file_info.Reserve2, file_info.Reserve3);
	/*开始事务*/
	ret = sqlite3_exec(db , "begin transaction" , 0 , 0 , &zErrMsg);
	ret = sqlite3_exec(db , cmd , 0 , callback , &zErrMsg);

	if (0 != ret)
	{
		/*异常，进入回滚*/
		ret = sqlite3_exec(db , "rollback transaction" , 0 , 0 , &zErrMsg) ;
		db_print_errMsg(cmd, &zErrMsg);
		printf("Error ,sqlite3_exec failed!! ret=%d\n", ret);
		return -1;
	}

	/*提交事务*/
	ret = sqlite3_exec(db , "commit transaction" , 0 , 0 , &zErrMsg);
	printf("Into DB:%s\n", cmd);
	
	return 0;
}

int db_query_total(sqlite3 *db)
{
	int ret = -1;
	int total = 0;
	char *zErrMsg = NULL;
	char cmd[128] = {0};
	sprintf(cmd, SELECT_GET_TOTAL);
	ret = sqlite3_exec(db, cmd, callback , &total, &zErrMsg);
	if(ret < 0)
	{
		printf("db_query_total is err!!\n");
		return -1;
	}

	return total;
}

int db_get_selectElemNum(sqlite3 *db, char *find)
{
	if (!find || !db)
	{
		printf("Error find_type=%p,%p\n", find, db);
		return -1;
	}

	int total = -1;
	int ret = -1;
	char cmd[512] = {0};
	char **zErrMsg = NULL;
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), SELECT_GET_TOTAL_FILENAME, find);
	ret = sqlite3_exec(db, cmd, callback , &total ,  zErrMsg);
	if (ret < 0)
	{
		return -1;
	}

	return total;
}

int db_select_elem(sqlite3 *db, DB_Select_t select_info, DB_Order_Reslut_t *reslut_info)
{
	if (!select_info.key || !db)
	{
		printf("Error find_type=%p,%p\n", select_info.key, db);
		return -1;
	}

	if (select_info.from >= select_info.to)
	{
		printf("db_select_elem is err from=%d to=%d\n", select_info.from, select_info.to);
		return -1;
	}
	char cmd[512] = {0};
	int total = -1;
	int ret = -1;
	total = db_get_selectElemNum(db, select_info.key);
	if (total <= 0)
	{
		printf("db_get_selectElemNum is err or have no data!!\n");
		return -1;
	}
	
	if (select_info.to > total)
	{
		select_info.to = total;
	}

	if (select_info.from < 0)
	{
		select_info.from = 0;
	}
	

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, SELECT_FILENAME_SQL, select_info.key, select_info.to - select_info.from, select_info.from);

	int row = -1, col = -1;
	char **reslut_tmp = NULL;
	ret = db_find_elem(db, cmd, &reslut_tmp, &row, &col);
	if (ret < 0)
	{
		return -1;
	}
	reslut_info->row = row;
	reslut_info->col = col;
	reslut_info->reslut = reslut_tmp;
	return total;
}

int db_order_elem(sqlite3 * db, DB_Order_t order_info, DB_Order_Reslut_t *reslut_info)
{
	int ret = 0;
	char **azResult = NULL;
	char *zErrMsg = NULL;
	char cmd[512] = {0};
	int total = 0;
	int max_row = 0, max_column = 0;

	if (!db)
	{
		printf("s_db=%p Error!!!\n", db);
		return -1;
	}
	
	// 获取总数
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, SELECT_GET_TOTAL);
	ret = sqlite3_exec(db, cmd, callback , &total ,  &zErrMsg);
	printf("order DB:%s,ret=%d total=%d\n", cmd, ret, total);
	if (ret < 0)
	{
		return -1;
	}

	if (order_info.from < 0)
	{
		order_info.from = 0;
	}

	if (order_info.to > total || 0 == order_info.to)
	{
		printf("Error,to_row=%d max_row=%dError!!\n", order_info.to, total);
		order_info.to = total;
	}
	
	// 查询命令
	memset(cmd, 0, sizeof(cmd));

	if (0 == order_info.mode)
	{
		sprintf(cmd, ORDER_STARTTIME_SQL_DESC,order_info.msg, order_info.to - order_info.from, order_info.from);
	}
	else if(1 == order_info.mode)
	{
		sprintf(cmd, ORDER_STARTTIME_SQL_ASC,order_info.msg, order_info.to - order_info.from, order_info.from);
	}
	else if(2 == order_info.mode)
	{
		sprintf(cmd, ORDER_FILESIZE_SQL_DESC,order_info.msg, order_info.to - order_info.from, order_info.from);
	}
	else if(3 == order_info.mode)
	{
		sprintf(cmd, ORDER_FILESIZE_SQL_ASC, order_info.msg, order_info.to - order_info.from, order_info.from);
	}
	else
	{
		sprintf(cmd, ORDER_STARTTIME_SQL_DESC,order_info.msg, order_info.to - order_info.from, order_info.from);
	}
	
	ret = db_exec_selectSQL(db, cmd, &azResult, &max_row, &max_column);
	printf("order DB:%s\n", cmd);
	if (ret < 0)
	{
		printf("db_exec_selectSQL is err!!!\n");
		return -1;
	}

	printf("max_row=%d--max_column=%d %d\n", max_row, max_column, order_info.to);
	reslut_info->row = max_row;
	reslut_info->col = max_column;
	/*校验检索查询的结果*/
	if (max_row <= 0)
	{
		printf("Error,max_row=%dError!!\n", max_row);
		return -1;
	}

	reslut_info->reslut = azResult;
	return total;
}

int db_modify_elem(sqlite3 *db, int mode, const char *file_name, char *modify)
{
	if (mode > 7 && mode < 1)
	{
		printf("mode=%d Error!!!\n", mode);
		return -1;
	}

	if (!db)
	{
		printf("db=%p Error!!!\n", db);
		return -1;
	}

	int ret = 0;
	char *zErrMsg = NULL;
	char cmd[512] = {0};

	if (DB_MODIDY_FILE_NAME == mode)
	{
		sprintf(cmd, MODIFY_FileName_SQL, modify, file_name);
	} 
	else if(DB_MODIFY_NOTES== mode)
	{
		sprintf(cmd, MODIFY_Notes_SQL, modify, file_name);
	}
	else if(DB_MODIDY_COURSE_TEACHER == mode)
	{
		sprintf(cmd, MODIFY_CourseTeacher_SQL, modify, file_name);
	} 

	ret = sqlite3_exec(db , cmd , 0 , callback , &zErrMsg);
	if (0 != ret)
	{
		db_print_errMsg(cmd, &zErrMsg);
		printf("Error ,sqlite3_exec failed!! ret=%d\n", ret);
		return -1;
	}
	return 0;

}
