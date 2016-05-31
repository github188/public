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
/*@����д�ļ�
 *@[out]pBuf,����
 *@[in]fileName�ļ���
 *@[in]size,��С
 *@[in]mode,���硰w��ֻ��
 *@return ����д���ļ��Ĵ�С�����󷵻�-1��
 */

int file_read_data(char *pBuf, int size, const char *fileName);
/*@����д�ļ�
 *@[out]pBuf,����
 *@[in]fileName�ļ���
 *@[in]size,��С
 *@[in]mode,���硰w��ֻ��
 *@return ����д���ļ��Ĵ�С�����󷵻�-1��
 */

unsigned long long file_get_len(char *fileName);
/*@���ܻ�ȡ�ļ��ĳ���
 *@[in]fileName�ļ���
 *@return �ɹ������ļ��ĳ��ȣ����󷵻�-1��
 */

int file_get_type(char *fileName);
/*@���ܻ�ȡ�ļ�������
 *@[in]fileName�ļ���
 *@return �ɹ������ļ��ĳ��ȣ����󷵻�-1��
 */



#ifdef __cplusplus
}
#endif

#endif

