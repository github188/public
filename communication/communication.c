#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "communication.h"

int communtication_set_handleStatus(Commutication_Handle_t handle, int status)
{
	if (NULL == handle)
	{
		printf("handle is NULL\n");
		return -1;
	}

	handle->status = status;
	return 0;
}

int communtication_get_handleStatus(Commutication_Handle_t handle)
{
	if (NULL == handle)
	{
		printf("handle is NULL\n");
		return NO_INIT_STATUS;
	}

	return handle->status;
}

int commutication_init_head(Communtication_Head_t *head, int identifier)
{
	memset(head, 0, sizeof(Communtication_Head_t));
	head->check_start[0] = head->check_start[1] = head->check_start[2] = head->check_start[3] = DEFAULT_CHECK_START_CODE;
	head->check_end[0] = head->check_end[1] = head->check_end[2] = head->check_end[3] = DEFAULT_CHECK_END_CODE;
	head->identifier = identifier;
	head->total_len = DEFATULT_COMMUTICATION_TOTAL_LEN;

	return 0;
}

int communtication_check_head(Communtication_Head_t *head)
{
	if((head == NULL) || (head->check_start[0] != DEFAULT_CHECK_START_CODE)
	   || (head->check_start[1] != DEFAULT_CHECK_START_CODE)
	   || (head->check_start[2] != DEFAULT_CHECK_START_CODE)
	   || (head->check_start[3] != DEFAULT_CHECK_START_CODE)
	   || (head->check_end[0] != DEFAULT_CHECK_END_CODE)
	   || (head->check_end[1] != DEFAULT_CHECK_END_CODE)
	   || (head->check_end[2] != DEFAULT_CHECK_END_CODE)
	   || (head->check_end[3] != DEFAULT_CHECK_END_CODE))
	{
		printf("the head is not head\n");
		printf("[%c][%c][%c][%c][%c][%c][%c][%c]\n", head->check_start[0], head->check_start[1], head->check_start[2], head->check_start[3],
		      head->check_end[0], head->check_end[1], head->check_end[2], head->check_end[3]);
		return -1;
	}

	return 0;
}

void communtication_free_head(Commutication_Handle_t *handle)
{
	Commutication_Handle_t temp = *handle;

	if (temp != NULL)
	{
		pthread_mutex_destroy(&(temp->lock));
		free(temp);
		*handle = NULL;
	}

	return;
}


