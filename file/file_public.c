#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "file_public.h"
int file_write_data(const char *pBuf, int size, const char *fileName, const char *mode)
{
	FILE* filefd;
	int ret = -1;
	
	if (NULL == fileName || NULL == pBuf || NULL == mode)
	{
		printf("file_write_data param is NULL!!!\n");
		return -1;
	}
	
	if (NULL == (filefd = fopen(fileName, mode)))
	{
		printf("file_write_data fopen is err!!!\n");
		perror("fopen");
		return -1;
	}
	
	ret = fwrite(pBuf, 1, size, filefd);
	if (ret != size)
	{
		printf("file_write_data fwrite is err!!!\n");
		perror("fwrite");
		return -1;
	}
	
	fclose(filefd);
	return 0;
}

int file_read_data(char *pBuf, int size, const char *fileName)
{
	FILE* filefd;
	int ret = -1;
	
	if (NULL == fileName || NULL == pBuf || 0 == size)
	{
		printf("file_read_data param is err!!!\n");
		return -1;
	}
	
	if (NULL == (filefd = fopen(fileName, "r")))
	{
		printf("file_read_data fopen is err!!!\n");
		perror("fopen");
		return -1;
	}

	ret = fread(pBuf, 1, size, filefd);
	if (ret != size)
	{
		if (0 != feof(filefd))
		{
			printf("file_read_data fread is end of file!!!\n");
		}
		else
		{
			printf("file_read_data fread is err!!\n");
			perror("fread");
			fclose(filefd);
			return -1;
		}
	}
	fclose(filefd);
	return 0;
}

unsigned long long file_get_len(char *fileName)
{
	FILE* filefd;
	unsigned long long  file_len = 0;
	struct stat FileInfo;
	
	if (NULL == fileName)
	{
		printf("file_get_len param is err!!!\n");
		return -1;
	}
	
	if (NULL == (filefd = fopen(fileName, "r")))
	{
		printf("file_get_len fopen is err!!!\n");
		return -1;
	}
	
	if (stat(fileName, &FileInfo) < 0)
	{
		printf("file_get_len stat is err!!!\n");
		fclose(filefd);
		return -1;
	}
	
	file_len = FileInfo.st_size;
	fclose(filefd);
	return file_len;

}

int file_get_type(char * fileName)
{
	if (NULL == fileName)
	{
		printf("file_get_type param is err!!!\n");
		return -1;
	}

	File_Type_t ret = LINK_FILE;
	struct stat file_info;
	if (stat(fileName, &file_info) < 0)
	{
		printf("file_get_type stat is err!!!\n");
		return -1;
	}
	else
	{
		if (S_ISLNK(file_info.st_mode))
		{
			ret = LINK_FILE;
		}

		if (S_ISSOCK(file_info.st_mode))
		{
			ret = SOCK_FILE;
		}

		if (S_ISFIFO(file_info.st_mode))
		{
			ret = FIFO_FILE;
		}

		if (S_ISBLK(file_info.st_mode))
		{
			ret = BLK_FILE;
		}

		if (S_ISCHR(file_info.st_mode))
		{
			ret = CHR_FILE;
		}

		if (S_ISDIR(file_info.st_mode))
		{
			ret = DIR_FILE;
		}

		if (S_ISREG(file_info.st_mode))
		{
			ret = REG_FILE;
		}
			
	}
	return ret;
}

