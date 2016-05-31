/**********************************************************
 * Filename:    MessageQueue.h
 * 
 *
 * Description:��Ϣ���еķ�װ
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
 * �Զ�����Ϣ�ṹ��
 *
 * 
 */
#define MSGSIZE 1024
typedef struct mymesg{
	long mtype;
	char data[MSGSIZE];
}Msg_info_t; 


/*!
 * @brief  ����IPC��
 * 
 * @param[in] 
 * 
 * @return IPC��ֵ
 *
 */
key_t create_key(const char *path);


/*!
 * @brief ������Ϣ���б�ʶ��
 * 
 * @param[in] 
 * 
 * @return ��Ϣ���б�ʶ��
 */
int msg_get(key_t key);


/*!
 * @brief ����Ϣ����ִ�ж��ֲ��������õ��ǰ�cmd
 * ��ΪIPC_RMID��ɾ������Ϣ����
 * @param[in] 
 * 
 * @return
 */
void msg_ctl(int msgid, int cmd, struct msqid_ds *buf);


/*!
 * @brief ������������Ϣ
 * 
 * @param[in] 
 * 
 * @return 
 */
void msg_send(int msgid, const Msg_info_t *msg);


/*!
 * @brief ����������Ϣ
 * 
 * @param[in] 
 * 
 * @return 
 */
void msg_recv(int msgid, Msg_info_t *msg, long type);


#endif /* __MESSAGEQUEUE_H_ */

