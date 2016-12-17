/*************************************************************************
File Name: comm.h
Author: 459336407@qq.com
date: 2016.12.16
 ************************************************************************/

#ifndef __COMM_H__
#define __COMM_H__

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define public_dbg(fmt, ...) do {printf("\033[34m""[DEBUG %s:%d:%s] ""\033[0m" fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);} while(0)
#define public_info(fmt, ...) do {printf("\033[32m""[INFO  %s:%d:%s] ""\033[0m" fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);} while(0)
#define public_warn(fmt, ...) do {printf("\033[33m""[WARN  %s:%d:%s] ""\033[0m" fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);} while(0)
#define public_err(fmt, ...) do {printf("\033[31m""[ERROR %s:%d:%s] ""\033[0m" fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);} while(0)




#endif
