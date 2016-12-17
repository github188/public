#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list_public.h"


typedef struct node
{
	void* pData;
	struct node* next;
}Data_Node_t;

typedef struct _HeadNode
{
	void* pData;
	int nSize;
	struct node* next;
}Head_Node_t;

List_Handle_t list_create()
{
	Head_Node_t * pHead = (Head_Node_t *)malloc(sizeof(Head_Node_t));
	if (NULL == pHead)
	{
		return NULL;
	}
	else
	{
		pHead->pData = NULL;
		pHead->nSize = 0;
		pHead->next = NULL;
	}
	
	return pHead;
}

static int list_get_len(Head_Node_t *pHead)
{
	/*
	Data_Node_t * pNode;
	if (pHead == NULL)
	{
		return NULL;
	}
	pNode = pHead->next;
	if (NULL == pNode)
	{
		return NULL;
	}
	for(;pNode->next != NULL; pNode = pNode->next)
	{
		printf("the data is %d\n", *(int *)(pNode->pData));
	}
	*/
	return pHead->nSize;
}

Data_Node_t * find_node_sectail(Head_Node_t *pHead)
{
	Data_Node_t * pNode = NULL;
	Data_Node_t * pNode_tmp = NULL;
	if (pHead == NULL)
	{
		return NULL;
	}
	
	pNode = pHead->next;
	if (NULL == pNode)
	{
		return NULL;
	}
	if (NULL == pNode->next)
	{
		return (Data_Node_t *)-1;
	}
	
	while(1)
	{
		pNode_tmp = pNode->next;
		if (pNode_tmp->next == NULL)
		{
			return pNode;
		}

		pNode = pNode->next;
		//printf("pNode->data:%d %p %p\n", *(int *)pNode->pData, pNode, pNode->next);
	}
	printf("pNode->data:%d %p %p\n", *(int *)pNode->pData, pNode->pData, pNode);
	return pNode;
}


Data_Node_t * find_node_tail(Head_Node_t *pHead)
{
	Data_Node_t * pNode = NULL;
	if (pHead == NULL)
	{
		return NULL;
	}
	
	pNode = pHead->next;
	if (NULL == pNode)
	{
		return NULL;
	}
	
	for(;pNode->next != NULL; pNode = pNode->next)
	{
		printf("pNode->data:%d %p %p\n", *(int *)pNode->pData, pNode, pNode->next);
	}
	printf("pNode->data:%d %p %p\n", *(int *)pNode->pData, pNode->pData, pNode);
	return pNode;
}

int list_Insert_toTail(Head_Node_t *pHead, void *data)
{
 	Data_Node_t * pCurNode = (Data_Node_t *)malloc(sizeof(Data_Node_t));
 	if (NULL == pCurNode)
 	{
 		return -1;
 	}

	if (NULL != pHead->next)
	{
		Data_Node_t * pTailNode = find_node_tail(pHead);
		if (NULL == pTailNode)
		{
			return -1;
		}
		
		pCurNode->pData = data;
		pCurNode->next = NULL;
		pTailNode->next = pCurNode;
		pHead->nSize += 1;
	}
	else
	{
		pCurNode->next = NULL;
		pCurNode->pData = data;
		pHead->next = pCurNode;
		pHead->nSize += 1;
	}
 	return 0;
}

int list_Insert_toHead(Head_Node_t * pHead,void * data)
{
	Data_Node_t * pCurNode = (Data_Node_t *)malloc(sizeof(Data_Node_t));
 	if (NULL == pCurNode)
 	{
 		return -1;
 	}

	pCurNode->pData = data;
 	pCurNode->next = pHead->next;
 	pHead->next = pCurNode;
	pHead->nSize += 1;
 	return 0;
}

pList_userData_t list_get_fromHead(Head_Node_t * pHead)
{
	if (NULL == pHead)
	{
		return NULL;
	}
	
	if (list_get_len(pHead) > 0)
	{
		pList_userData_t data = NULL;
		Data_Node_t * pCurNode = pHead->next;
		data = pCurNode->pData;
		pHead->next = pCurNode->next;
		pCurNode->next = NULL;

		if (NULL != pCurNode)
		{
			free(pCurNode);
			pCurNode = NULL;
		}

		pHead->nSize -= 1;
		return data;
	}
	
 	return NULL;
}

pList_userData_t list_get_fromTail(Head_Node_t * pHead)
{
	if (NULL == pHead)
	{
		return NULL;
	}
	
	if (list_get_len(pHead) > 1)
	{
		pList_userData_t data = NULL;
		Data_Node_t * pCurNode = find_node_sectail(pHead);
		Data_Node_t * pNextNode = NULL;

		pNextNode = pCurNode->next;
		pCurNode->next = NULL;
		if (NULL != pNextNode)
		{
			data = pNextNode->pData;
			printf("the data is %p\n",data);
			free(pNextNode);
			pNextNode = NULL;
		}
		
		pHead->nSize -= 1;
 		return data;
	}
	else
	{
		pList_userData_t data = NULL;
		Data_Node_t * pCurNode = pHead->next;
		printf("the pCurNode is:%p\n", pCurNode);
		pHead->next = NULL;
		data = pCurNode->pData;
		if (NULL != pCurNode)
		{
			free(pCurNode);
			pCurNode = NULL;
		}
		pHead->nSize -= 1;
		return data;
	}
	
	return NULL;
}

Data_Node_t * list_search_condNode(Head_Node_t * pHead, void* data)
{
	if (NULL == pHead)
	{
		return NULL;
	}
	
	Data_Node_t * pCurNode = NULL;
	for (pCurNode = pHead->next; pCurNode->pData != data; pCurNode = pCurNode->next);

 	return pCurNode;
}

int list_clear(List_Handle_t pListHandle)
{
	Head_Node_t * pHead =(Head_Node_t *)pListHandle;
	if (NULL == pHead)
	{
		return -1;
	}
	
	Data_Node_t * pNode = pHead->next;
	Data_Node_t * pNodeBak;
	while (NULL != pNode)
	{
		pNodeBak = pNode;
		pNode = pNode->next;
		if (NULL != pNodeBak->pData)
		{
			free(pNodeBak->pData);
			pNodeBak->pData = NULL;
		}
		free(pNodeBak);//销毁节点
		pNodeBak = NULL;
	}

	pHead->nSize = 0;
	return 0;
}


int list_destory(List_Handle_t pListHandle)
{
	Head_Node_t * pHead =(Head_Node_t *)pListHandle;
	if (NULL == pHead)
	{
		return -1;
	}
	
	Data_Node_t *pNode = pHead->next;
	Data_Node_t * pNodeBak;
	while (NULL != pNode)
	{
		pNodeBak = pNode;
		pNode = pNode->next;
		if (NULL != pNodeBak->pData)
		{
			free(pNodeBak->pData);
			pNodeBak->pData = NULL;
		}
		free(pNodeBak);//销毁节点
		pNodeBak = NULL;
	}

	if (NULL != pHead)
	{
		free(pHead);//销毁头
		pHead = NULL;
	}
	
	return 0;
}

List_LockHandle_t* list_lockAndCreate()
{
	List_LockHandle_t* pList_lockHandle = malloc(sizeof(List_LockHandle_t));
	if (NULL == pList_lockHandle)
	{
		printf("list_lockAndCreate malloc is err!!!\n");
		return NULL;
	}
	
	memset(pList_lockHandle, 0, sizeof(List_LockHandle_t));
	pList_lockHandle->list_handle = list_create();
	if (NULL == pList_lockHandle->list_handle)
	{
		printf("list_create is errr!\n");
		return NULL;
	}
	
	pthread_mutex_init(&(pList_lockHandle->lock), NULL);
	return pList_lockHandle;
}

pList_userData_t list_lockAndGet_fromTail(List_LockHandle_t* pHeadHandle)
{
	pList_userData_t pNodeData = NULL;
	if (pHeadHandle == NULL)
	{
		return NULL;
	}
	
	pthread_mutex_lock(&(pHeadHandle->lock));
	pNodeData = list_get_fromTail(pHeadHandle->list_handle);
	pthread_mutex_unlock(&(pHeadHandle->lock));
	return pNodeData;
}

pList_userData_t list_lockAndGet_fromHead(List_LockHandle_t* pHeadHandle)
{
	pList_userData_t pNodeData = NULL;
	if (NULL == pHeadHandle)
	{
		return NULL;
	}
	
	pthread_mutex_lock(&(pHeadHandle->lock));
	pNodeData = list_get_fromHead(pHeadHandle->list_handle);
	pthread_mutex_unlock(&(pHeadHandle->lock));
	return pNodeData;
}

pList_userData_t list_lockAndSet_toTail(List_LockHandle_t* pHeadHandle, pList_userData_t *pData)
{
	pList_userData_t pNodeData = NULL;
	if (NULL == pHeadHandle)
	{
		return NULL;
	}
	
	int ret = -1;
	pthread_mutex_lock(&(pHeadHandle->lock));
	ret = list_Insert_toTail(pHeadHandle->list_handle, pData);
	if (0 != ret)
	{
		printf("list_Insert_toTail is err!!!\n");
	}
	pthread_mutex_unlock(&(pHeadHandle->lock));
	return pNodeData;
}

pList_userData_t list_lockAndSet_toHead(List_LockHandle_t* pHeadHandle, pList_userData_t* pData)
{
	pList_userData_t pNodeData = NULL;
	if (NULL == pHeadHandle)
	{
		return NULL;
	}
	int ret = -1;
	pthread_mutex_lock(&(pHeadHandle->lock));
	ret = list_Insert_toHead(pHeadHandle->list_handle, pData);
	if (0 != ret)
	{
		printf("list_Insert_toHead is err!!!\n");
	}
	pthread_mutex_unlock(&(pHeadHandle->lock));
	return pNodeData;
}

int list_lockAndGet_len(List_LockHandle_t* pHeadHandle)
{
	int size = 0;
	if (NULL == pHeadHandle)
	{
		return -1;
	}
	pthread_mutex_lock(&(pHeadHandle->lock));
	size = list_get_len(pHeadHandle->list_handle);
	pthread_mutex_unlock(&(pHeadHandle->lock));
	return size;
}

int list_lockAndDestory(List_LockHandle_t* pList_handle)
{
	if (NULL == pList_handle)
	{
		return -1;
	}
	pthread_mutex_lock(&(pList_handle->lock));
	list_destory(pList_handle->list_handle);
	pthread_mutex_unlock(&(pList_handle->lock));
	pthread_mutex_destroy(&(pList_handle->lock));
	if (NULL != pList_handle)
	{
		free(pList_handle);
		pList_handle = NULL;
	}
	return 0;
}

int list_lockAndClear(List_LockHandle_t * pList_handle)
{
	int ret = 0;
	if (NULL == pList_handle)
	{
		return -1;
	}
	
	pthread_mutex_lock(&(pList_handle->lock));
	ret = list_clear(pList_handle->list_handle);
	pthread_mutex_unlock(&(pList_handle->lock));
	return ret;
}


/*****************************************************************************/
static int list_wait_signal(List_LockHandle_t* pHeadHandle, int time)
{
	struct timespec abstime = {2, 0};
	struct timeval now;
	int ret = 0;
	if(pHeadHandle == NULL)
	{
		return -1;
	}
	if(time > 0)
	{
		gettimeofday(&now, NULL);
		abstime.tv_sec = now.tv_sec + time / 1000;
		abstime.tv_nsec = (now.tv_usec * 1000) + (time % 1000) * 1000 * 1000;
		ret = pthread_cond_timedwait(&(pHeadHandle->condl), &(pHeadHandle->lock), &abstime);
	}
	else
	{
		ret= pthread_cond_wait(&(pHeadHandle->condl), &(pHeadHandle->lock));
	}

	return ret;
}

 List_NodeData_t list_lockAndPop_backSignal(List_LockHandle_t* pHeadHandle, int time)
{
	List_NodeData_t pNodeData = NULL;
	int size = 0;
	if(pHeadHandle == NULL)
	{
		return NULL;
	}
	pthread_mutex_lock(&(pHeadHandle->lock));
	if((size = list_size(pHeadHandle->pList_Handle)) <= 0)
	{
		list_wait_signal(pHeadHandle,  time);
	}

	pNodeData = list_pop_back(pHeadHandle->pList_Handle);
	pthread_mutex_unlock(&(pHeadHandle->lock));
	return pNodeData;
}

List_NodeData_t list_lockAndPop_frontSignal(List_LockHandle_t* pHeadHandle, int time)
{
	List_NodeData_t pNodeData = NULL;
	int size = 0;
	if(pHeadHandle == NULL)
	{
		return NULL;
	}
	pthread_mutex_lock(&(pHeadHandle->lock));
	if((size = list_size(pHeadHandle->pList_Handle)) <= 0)
	{
		list_wait_signal(pHeadHandle,  time);
	}
	pNodeData = list_pop_front(pHeadHandle->pList_Handle);
	pthread_mutex_unlock(&(pHeadHandle->lock));
	return pNodeData;
}

List_CurNode_t list_lockAndPush_backSignal(List_LockHandle_t* pHeadHandle, void* pData)
{
	List_CurNode_t pNodeData = NULL;
	if(pHeadHandle == NULL)
	{
		return NULL;
	}
	pthread_mutex_lock(&(pHeadHandle->lock));
	pNodeData = list_push_back(pHeadHandle->pList_Handle, pData);
	pthread_cond_signal(&(pHeadHandle->condl));
	pthread_mutex_unlock(&(pHeadHandle->lock));
	return pNodeData;
}

List_CurNode_t list_lockAndPush_frontSignal(List_LockHandle_t* pHeadHandle, void* pData)
{
	List_CurNode_t pNodeData = NULL;
	if(pHeadHandle == NULL)
	{
		return NULL;
	}
	pthread_mutex_lock(&(pHeadHandle->lock));
	pNodeData = list_push_front(pHeadHandle->pList_Handle, pData);
	pthread_cond_signal(&(pHeadHandle->condl));
	pthread_mutex_unlock(&(pHeadHandle->lock));
	return pNodeData;
}


