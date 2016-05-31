#ifndef	__FILE_PUBLIC_H__
#define	__FILE_PUBLIC_H__
/*
author : ydl	   2015.12.30
notes:if you want use these API,and the file size is > 4G,please -D_FILE_OFFSET_BITS=64
*/
#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	LINK_FILE = 0,
	REG_FILE,
	DIR_FILE,
	CHR_FILE,
	BLK_FILE,
	SOCK_FILE,
	FIFO_FILE
}File_Type_t;

int file_write_data(const char *pBuf, int size, const char *fileName, const char *mode);
/*@功能写文件
 *@[out]pBuf,内容
 *@[in]fileName文件名
 *@[in]size,大小
 *@[in]mode,例如“w”只读
 *@return 返回写入文件的大小，错误返回-1；
 */

int file_read_data(char *pBuf, int size, const char *fileName);
/*@功能写文件
 *@[out]pBuf,内容
 *@[in]fileName文件名
 *@[in]size,大小
 *@[in]mode,例如“w”只读
 *@return 返回写入文件的大小，错误返回-1；
 */

unsigned long long file_get_len(char *fileName);
/*@功能获取文件的长度
 *@[in]fileName文件名
 *@return 成功返回文件的长度，错误返回-1；
 */

int file_get_type(char *fileName);
/*@功能获取文件的类型
 *@[in]fileName文件名
 *@return 成功返回文件的长度，错误返回-1；
 */



#ifdef __cplusplus
}
#endif

#endif

