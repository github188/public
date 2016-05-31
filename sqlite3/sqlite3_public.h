
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
	long long int MovieSize;					//�ļ���С(byte)
	unsigned int  RcdTimeLenght;				//¼��ʱ��(s)
	char FileName[128];							//�ļ��б�
	char RcdStartTime[128];						//¼��ʱ��
	char Notes[128];							//��ע
	unsigned short DownloadCnt;					//���ش���
	unsigned char  FTPupload;					//�ϴ�״̬ 3 -δ�ϴ�,2-�ϴ�ʧ��,1-�ϴ���,0-�ϴ����
	unsigned char  BackupFTPload;				//����FTP�ϴ�״̬ 0 -δ�ϴ�,1-�ϴ���,2-�ϴ�ʧ��,3-�ϴ����
	unsigned int  CourseCompleteStatus;			//¼�����״̬   0-�쳣�μ� 1-�����μ�
	char CourseTeacher[64];						//�μ��ڿ���ʦ
	char CourseSubject[32];						//�γ���
	int Reserve1;								//Ԥ��
	int Reserve2;
	int Reserve3;
}DataBase_t;

typedef struct
{
	int from;//������ʼ��Ŀ
	int to;//���ҽ�����Ŀ
	int mode;//�����������ʽ
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
	char key[128];//���ҵĹؼ���
	int from;//���ҿ�ʼ��Ŀ
	int to;//���ҽ�����Ŀ
	int pageSize;//��ѯ��ҳ��
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

//ע��db_order_elem db_order_elem ����sqlite3_free_table(pp_result);�ͷ��ڴ�
int db_sqlite_init(sqlite3 **db);//��ʼ������db
int db_insert_elem(sqlite3 *db, DataBase_t file_info);//�������ݿ�
int db_query_total(sqlite3 *db);//��ѯ���ݿ��ܵ���Ŀ���Ż�ֵ����Ŀ
int db_get_selectElemNum(sqlite3 *db, char *key);//��ȡ���ݿ��з��ϲ�ѯ��������Ŀ������ֵ������Ŀ
int db_select_elem(sqlite3 *db, DB_Select_t select_info, DB_Order_Reslut_t *reslut_info);//�������ݿ���ָ��Ԫ��
int db_order_elem(sqlite3 * db, DB_Order_t order_info, DB_Order_Reslut_t *reslut_info);//�����ѯ���ݿ�
int db_modify_elem(sqlite3 * db,int mode,const char * file_name,char *modify);//file_nameΪ��Ҫ�޸ĵ��ļ������޸����ݿ�modeΪ�޸��modifyΪ�޸ĺ������


#ifdef __cplusplus
}
#endif

#endif


