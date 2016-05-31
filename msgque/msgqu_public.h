/**********************************************************
 * Filename:    MessageQueue.h
 * 
 *
 * Description:消息队列的封装
 *              
 *
 * Author:ydl
 *         
 *  
 *
 * date: 2016.01.18 
 *********************************************************/


#ifndef __MESSAGEQUEUE_H_
#define __MESSAGEQUEUE_H_

#include <sys/ipc.h>
/*
 * 自定义消息结构体
 *
 * 
 */
#define MSGSIZE 1024
typedef struct mymesg{
	long mtype;
	char data[MSGSIZE];
}Msg_info_t; 


/*!
 * @brief  创建IPC键
 * 
 * @param[in] 
 * 
 * @return IPC键值
 *
 */
key_t create_key(const char *path);


/*!
 * @brief 创建消息队列标识符
 * 
 * @param[in] 
 * 
 * @return 消息队列标识符
 */
int msg_get(key_t key);


/*!
 * @brief 对消息队列执行多种操作，常用的是把cmd
 * 置为IPC_RMID来删除该消息队列
 * @param[in] 
 * 
 * @return
 */
void msg_ctl(int msgid, int cmd, struct msqid_ds *buf);


/*!
 * @brief 非阻塞发送消息
 * 
 * @param[in] 
 * 
 * @return 
 */
void msg_send(int msgid, const Msg_info_t *msg);


/*!
 * @brief 阻塞接收消息
 * 
 * @param[in] 
 * 
 * @return 
 */
void msg_recv(int msgid, Msg_info_t *msg, long type);


#endif /* __MESSAGEQUEUE_H_ */

