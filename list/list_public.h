#ifndef	__LIST_PUBLIC_H__
#define	__LIST_PUBLIC_H__
/*
author : ydl	   2015.12.30
notes:
*/
#ifdef __cplusplus
extern "C" {
#endif

typedef  void* List_Handle_t;//链表操作句柄
typedef void* pList_userData_t;//用户数据
//typedef void* List_CurNode_t;//链表节点

typedef struct _List_LockHandle
{
	List_Handle_t list_handle;
	pthread_mutex_t lock;
}List_LockHandle_t;

List_LockHandle_t* list_lockAndCreate();
pList_userData_t list_lockAndGet_fromTail(List_LockHandle_t* pHeadHandle);
pList_userData_t list_lockAndGet_fromHead(List_LockHandle_t* pHeadHandle);
pList_userData_t list_lockAndSet_toTail(List_LockHandle_t* pHeadHandle, pList_userData_t* pData);
pList_userData_t list_lockAndSet_toHead(List_LockHandle_t* pHeadHandle, pList_userData_t* pData);
int list_lockAndGet_len(List_LockHandle_t* pHeadHandle);
int list_lockAndClear(List_LockHandle_t* pList_handle);
int list_lockAndDestory(List_LockHandle_t* pList_handle);

#ifdef __cplusplus
}
#endif

#endif
