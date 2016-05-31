#include <stdio.h>
#include <signal.h>

#include "pthreadInfo_public.h"

int pthread_test_alive(pthread_t tid)
{
	int ret = -1;
	ret = pthread_kill(tid, 0);
	if (0 != ret)
	{
		printf("ID=0x%x is not alive or exit\n",(unsigned int)tid);
		ret = 0;
	}
	else
	{
		printf("ID=0x%x is alive\n",(unsigned int)tid);
		ret = -1;
	}

	return ret;
}

pid_t gettid()
{
	return syscall(SYS_gettid);
}


