#ifndef	__PTHREADINFO_PUBLIC_H__
#define	__PTHREADINFO_PUBLIC_H__

/*
author : ydl	   2015.12.30
notes:if you want use these API,and the file size is > 4G,please -D_FILE_OFFSET_BITS=64
*/
#include <pthread.h>
#include <sys/syscall.h> 

#ifdef __cplusplus
extern "C" {
#endif

int pthread_test_alive(pthread_t tid);
/*@���ܲ鿴�߳��Ƿ���
 *@[in]�߳�ID
 *@return �ɹ�����0��ʧ�ܷ��ط�0ֵ
 */

pid_t gettid();




#ifdef __cplusplus
}
#endif

#endif

