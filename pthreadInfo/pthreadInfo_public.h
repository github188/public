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
/*@功能查看线程是否存活
 *@[in]线程ID
 *@return 成功返回0，失败返回非0值
 */

pid_t gettid();




#ifdef __cplusplus
}
#endif

#endif

