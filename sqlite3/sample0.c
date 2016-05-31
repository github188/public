#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sqlite3_public.h"
#define MAX_CHECK_NUM (10)

int dbl_output_struct(DataBase_t *info, char **buff, int pos)
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

int main(void)
{
	int ret = -1;
	sqlite3 *db = NULL;
	ret = db_sqlite_init(&db);
	if (0 != ret)
	{
		printf("db_sqlite_init is failed!!!!\n");
	}
	
	int i = -1;
	#if 0
	for (i = 0; i < 20; i++)
	{
		DataBase_t file_info;
		memset(&file_info, 0, sizeof(DataBase_t));
		sprintf(file_info.FileName, "t%d", i);
		memcpy(file_info.RcdStartTime, "2015", 4);
		file_info.CourseCompleteStatus = 1;
		file_info.MovieSize = 1024;
		file_info.RcdTimeLenght = 10;
		file_info.BackupFTPload = 1;
		file_info.DownloadCnt = 0;
		memcpy(file_info.CourseSubject, "tt", 1);
		memcpy(file_info.CourseTeacher, "ming", 3);
		ret = db_insert_elem(db, file_info);
		if (0 != ret)
		{
			printf("db_insert_elem is err!!!\n");
		}
	}
	#endif

	#if 0
	int total = 0;
	total = db_query_total(db);
	if (-1 == total)
	{
		printf("db_query_total is err!!!\n");
	}

	printf("the toatal = %d\n", total);
	DB_Order_t order_info;
	memset(&order_info, 0, sizeof(DB_Order_t));
	order_info.from = 0;
	order_info.to = 11;
	order_info.mode = 0;
	order_info.msg = "";
	DB_Order_Reslut_t reslut_info;
	memset(&reslut_info, 0, sizeof(DB_Order_Reslut_t));
	db_order_elem(db, order_info, &reslut_info);

	int pos = 0;
	DataBase_t info;
	for (i = 1; i < reslut_info.row; i++)
	{
		pos = (i) * reslut_info.col + 1;
		db_output_struct(&info, reslut_info.reslut, pos);
		printf("info=====%s %s %d\n", info.FileName, info.CourseTeacher, info.BackupFTPload);
	}
	#endif

	#if 0
	//DB_Find_t	result;
	//memset(&result, 0, sizeof(result));

	//result.from = 0;
	//result.to = 5;
	//result.pageSize = 1;

	
	DB_Select_t select_info;
	strcpy(select_info.key,"t3");
	select_info.from = 0;
	select_info.to = 8;
	select_info.pageSize = 2;
	DB_Order_Reslut_t reslut_info;
	memset(&reslut_info, 0, sizeof(DB_Order_Reslut_t));
	db_select_elem(db, select_info, &reslut_info);
	#endif
	/*
	DataBase_t info1;
	int pos = -1;
	for (i = 1 ; i < reslut_info.row + 1; i++)
	{
		pos = (i) * (reslut_info.col) + 1;
		dbl_output_struct(&info1, reslut_info.reslut, pos);
		printf("~~~~~~~~%s %s %lld\n", info1.FileName, info1.CourseTeacher, info1.MovieSize);
	}
	*/

	db_modify_elem(db, 1, "t3", "ttt");
	/*
	char **pp_result = NULL;
	int from = 0, to = 0;
	int mode = 0;
	int col = 0;
	int i = 0;
	DataBase_t info;
	int pos = -1;
	while (total > 0)
	{
		if (total > MAX_CHECK_NUM)
		{
			to = 10;
		}
		else
		{
			to = total;
		}
		
		ret = db_order_elem(db,"", mode, &from, &to, &col, &pp_result);
		for (i += 1; i < to + 1; i++)
		{
			memset(&info, 0, sizeof(DataBase_t));
			pos = (i) * col + 1;
			db_output_struct(&info, pp_result, pos);
		}

		total = total - to;
		from = to;
		printf("%s %s %d\n", info.CourseTeacher, info.FileName, info.DownloadCnt);
	}

	if (pp_result)
	{
		sqlite3_free_table(pp_result);
		pp_result = NULL;
	}
	
	/*
	char find[128] = "yyyyy";
	DB_Find_t reslut;
	memset(&reslut, 0, sizeof(DB_Find_t));
	
	
	reslut.from = 0
	reslut.to = 10;
	reslut.pageSize = 1;
	
	ret = db_select_elem(db, find, &reslut);
	if (0 != ret)
	{
		printf("it is err!!!\n");
	}
	*/
	//printf("the total is :%d\n", ret);
	return 0;
}
