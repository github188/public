
#ifndef	__SQLITE3_PUBLIC_H__
#define	__SQLITE3_PUBLIC_H__
/*
author : ydl	   2016.01.18
notes:
*/
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "sqlite3.h"

#define CREATE_DATABASE_SQL			("CREATE TABLE FileManage(\
										ID       		int PRIMARY KEY,  \
										FileName 		varchar(128)	,              \
										MovieSize 		int 	,\
										RcdStartTime  	varchar(128) 	,              \
										Notes    		varchar(128) 	,          			\
										RcdTimeLenght 	int		,\
										DownloadCnt		int		,\
										FTPupload		int		,\
										BackupFTPload	int		,\
										CourseCompleteStatus	int		,\
										CourseTeacher	varchar(64) 	,\
										CourseSubject	varchar(32) 	,\
										Reserve1		int		,\
										Reserve2		int		,\
										Reserve3		int		\
);")

#define INSERT_INTODB_SQL			("INSERT INTO 'FileManage' VALUES(%d,'%s','%s','%s','%s',%d,%d,%d,%d,%d,'%s','%s',%d,%d,%d);")
#define SELECT_GET_TOTAL_FILENAME	("SELECT COUNT(*) FROM FileManage WHERE FileName LIKE '%%%s%%'")
#define SELECT_FILENAME_SQL			("SELECT * FROM FileManage WHERE FileName LIKE '%%%s%%' limit %d offset %d")
#define SELECT_GET_TOTAL			("SELECT COUNT(*) FROM FileManage WHERE 1 = 1")
#define ORDER_STARTTIME_SQL_DESC 	("select  * from FileManage WHERE FileName LIKE '%%%s%%' order by RcdStartTime desc limit %d offset %d")
#define ORDER_STARTTIME_SQL_ASC 	("select * from FileManage WHERE FileName LIKE '%%%s%%' order by RcdStartTime asc limit %d offset %d")
#define ORDER_FILESIZE_SQL_DESC		("select * from FileManage WHERE FileName LIKE '%%%s%%' order by MovieSize desc limit %d offset %d")
#define ORDER_FILESIZE_SQL_ASC 		("select * from FileManage WHERE FileName LIKE '%%%s%%' order by MovieSize asc limit %d offset %d")

#define MODIFY_FileName_SQL			("update FileManage set FileName = '%s' where FileName ='%s' ")
#define MODIFY_Notes_SQL			("update FileManage set Notes = '%s' where  FileName='%s' ")
#define MODIFY_CourseTeacher_SQL	("update FileManage set CourseTeacher = '%s' where FileName  ='%s' ")


typedef struct DataBase_Struct{
	long long int MovieSize;					//文件大小(byte)
	unsigned int  RcdTimeLenght;				//录制时长(s)
	char FileName[128];							//文件列表
	char RcdStartTime[128];						//录制时间
	char Notes[128];							//备注
	unsigned short DownloadCnt;					//下载次数
	unsigned char  FTPupload;					//上传状态 3 -未上传,2-上传失败,1-上传中,0-上传完成
	unsigned char  BackupFTPload;				//备份FTP上传状态 0 -未上传,1-上传中,2-上传失败,3-上传完成
	unsigned int  CourseCompleteStatus;			//录制完成状态   0-异常课件 1-正常课件
	char CourseTeacher[64];						//课件授课老师
	char CourseSubject[32];						//课程名
	int Reserve1;								//预留
	int Reserve2;
	int Reserve3;
}DataBase_t;

typedef struct
{
	int from;//查找起始条目
	int to;//查找结束条目
	int mode;//结果返回排序方式
	char *msg;
}DB_Order_t;

typedef struct
{
	char **reslut;
	int row;
	int col;
}DB_Order_Reslut_t;

typedef struct
{
	char key[128];//查找的关键词
	int from;//查找开始条目
	int to;//查找结束条目
	int pageSize;//查询的页码
}DB_Select_t;

enum DB_MODYFY_ITEM
{
	DB_MODIDY_FILE_NAME 		= 1,
	DB_MODIFY_NOTES  			= 2,	
	DB_MODIDY_COURSE_TEACHER	= 3,
	DB_MODIFY_COURSE_SUBJECT  	= 4,
	DB_MODIDY_FTP_UPLOAD 		= 5,
	DB_MODIFY_BACKUP_FTPLOAD 	= 6,
	DB_MODIDY_DOWNLOAD_CNT 		= 7	
};

//注意db_order_elem db_order_elem 调用sqlite3_free_table(pp_result);释放内存
int db_sqlite_init(sqlite3 **db);//初始化创建db
int db_insert_elem(sqlite3 *db, DataBase_t file_info);//插入数据库
int db_query_total(sqlite3 *db);//查询数据库总的条目，放回值即条目
int db_get_selectElemNum(sqlite3 *db, char *key);//获取数据库中符合查询条件的条目，返回值就是数目
int db_select_elem(sqlite3 *db, DB_Select_t select_info, DB_Order_Reslut_t *reslut_info);//查找数据库中指定元素
int db_order_elem(sqlite3 * db, DB_Order_t order_info, DB_Order_Reslut_t *reslut_info);//排序查询数据库
int db_modify_elem(sqlite3 * db,int mode,const char * file_name,char *modify);//file_name为需要修改的文件名，修改数据库mode为修改项，modify为修改后的内容


#ifdef __cplusplus
}
#endif

#endif


