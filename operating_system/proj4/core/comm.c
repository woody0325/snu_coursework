/********************************************************
 * Filename: core/comm.c
 *  
 * Author: jtlim, RTOSLab. SNU.
 * 
 * Description: message queue management. 
 ********************************************************/
#include <core/eos.h>

void eos_init_mqueue(eos_mqueue_t *mq, void *queue_start, int16u_t queue_size, int8u_t msg_size, int8u_t queue_type) {
	// initialize each element of mq structure
	mq->queue_start = queue_start;
	mq->queue_size = queue_size;
	mq->msg_size = msg_size;
	mq->queue_type = queue_type;

	mq->front = queue_start;
	mq->rear = queue_start;

	// initialize putsem and getsem semaphores
	eos_init_semaphore(&(mq->putsem), queue_size, queue_type);
	eos_init_semaphore(&(mq->getsem), 0, queue_type);
}

int8u_t eos_send_message(eos_mqueue_t *mq, void *message, int32s_t timeout) {
//	PRINT("  eos_send_message\n");

	// acquire putsem by calling function, and error handling
	if(!eos_acquire_semaphore(&(mq->putsem), timeout)){
		PRINT("Error: eos_send_message, acquire putsem failed, %x\n", eos_get_current_task());
		return;
	}

	// copy message into message queue
	memcpy(mq->rear, message, sizeof(message));

	// control rear values in circular queue manner
	if(mq->rear == (mq->queue_start) + 4*(mq->queue_size - 1)){
		mq->rear = mq->queue_start;
	}
	else{
		mq->rear = mq->rear + 4;
	}

	// release getsem by calling function
	eos_release_semaphore(&(mq->getsem));

	return;
}

int8u_t eos_receive_message(eos_mqueue_t *mq, void *message, int32s_t timeout) {
//	PRINT("  eos_receive_message\n");

	// acquire getsem by calling function, and error handling
	if(!eos_acquire_semaphore(&(mq->getsem), timeout)){
		PRINT("Error: eos_receive_message, acquire getsem failed, %x\n", eos_get_current_task());
		return;
	}

	// copy message from message queue
	memcpy(message, mq->front, sizeof(message));

	// control front values in circular queue manner
	if(mq->front == (mq->queue_start) + 4*(mq->queue_size - 1)){
		mq->front = mq->queue_start;
	}
	else{
		mq->front = mq->front + 4;
	}

	// release putsem by calling function
	eos_release_semaphore(&(mq->putsem));

	return;		
}
