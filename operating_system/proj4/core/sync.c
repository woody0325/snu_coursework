/********************************************************
 * Filename: core/sync.c
 * 
 * Author: wsyoo, RTOSLab. SNU.
 * 
 * Description: semaphore, condition variable management.
 ********************************************************/
#include <core/eos.h>

// made for debugging purpose only
void print_sem_queue(_os_node_t *queue){
	printf("**print_sem_queue called\n");

	while(queue->next){
		printf("**%x\n", queue->next->ptr_data);
		queue = queue->next;
	}
}

void eos_init_semaphore(eos_semaphore_t *sem, int32u_t initial_count, int8u_t queue_type) {
	/* initialization */
	// set intialized values
	sem->count = initial_count;
	sem->queue_type = queue_type;
	sem->timeout = -1;

	// allocating first node of waiting queue
	sem->wait_queue = (_os_node_t*) malloc(sizeof(struct _os_node));
	sem->wait_queue->next = NULL;
}

// used to put a task in waiting queue
void push_sem_queue(_os_node_t *queue, eos_tcb_t *task){
//	PRINT("   push_sem_queue\n");

	// searching for the last node
	while(1){
		if(!queue->next) {break;}
		queue = queue->next;
	}

	// allocating a new node at the tail
	queue->next = (_os_node_t*) malloc(sizeof(struct _os_node));
	queue->next->ptr_data = (void*)task;
	queue->next->next = NULL;

	return;
}

// used for get a task from waiting queue
eos_tcb_t *pop_sem_queue(_os_node_t *queue, int8u_t queue_type){
//	PRINT("   pop_sem_queue\n");

	_os_node_t *pop_node;

	// searching for a node for disallocation
	if(queue_type == 0){ // FIFO queue
		pop_node = queue->next;
		queue->next = pop_node->next;
	}
	else if(queue_type == 1){ // priority queue
		int32u_t highest_priority = 64;
		int32u_t temp_priority;
		_os_node_t *prev_node;

		// searching for a node with highest priority
		while(queue->next){
			temp_priority = ((eos_tcb_t*)queue->next->ptr_data)->priority;
			if(temp_priority < highest_priority){
				highest_priority = temp_priority;
				prev_node = queue;
				pop_node = queue->next;
			}
		}
		prev_node->next = pop_node->next;
	}

	// disallocating a selected node
	eos_tcb_t *task;
	task = (eos_tcb_t*)pop_node->ptr_data;
	free(pop_node);

	return task;
}

int32u_t eos_acquire_semaphore(eos_semaphore_t *sem, int32s_t timeout) {
//	PRINT("  eos_acquire_semaphore (%x)\n", eos_get_current_task());

	// when there is resource available
	if(sem->count > 0){
		sem->count--;
		return 1;
	}

	eos_tcb_t *current_task;
	current_task = eos_get_current_task();

	if(timeout == -1){ // when timeout == -1, return immediately
		return 0;
	}
	else if(timeout == 0){ // when timeout == 0, wait for other task
		// push current task into waiting queue
		push_sem_queue(sem->wait_queue, current_task);
//		print_sem_queue(sem->wait_queue);
		current_task->state = 2;

		// save current context
		addr_t sp_save;
		sp_save = _os_save_context();
		current_task->sp = sp_save;
		if(!sp_save) {return 1;}
		
		// call eos_schedule function to execute other task
		eos_schedule();
		return 1;
	}
	else if(timeout > 0){ //when timeout >= 1, wait for timeout value
		// when thee is resource available
		if(sem->count > 0){
			sem->count--;
			sem->timeout = -1;
			return 1;
		}

		// decrease timeout value by 1 for every clock tick
		if(sem->timeout == 0){ // when timeout is met
			sem->timeout = -1;
			return 0;
		}
		else if(sem->timeout == -1){ // initial timeout setting
			sem->timeout = timeout;
		}
		else{
			sem->timeout = sem->timeout - 1;
		}

		// set alarm with period 1
		eos_alarm_t new_alarm;
		eos_set_alarm(eos_get_system_timer(), &new_alarm, 1, eos_acquire_semaphore, current_task);
		current_task->state = 2;

		eos_schedule();		
	}
}
		
void eos_release_semaphore(eos_semaphore_t *sem) {
//	PRINT("  eos_release_semaphore (%x)\n", eos_get_current_task());

	// when waiting queue is empty
	if(!sem->wait_queue->next){
		sem->count++;
		return;
	}

	// get a task from waiting queue and call eos_resume_task function
	eos_tcb_t *next_task;
	next_task = pop_sem_queue(sem->wait_queue, sem->queue_type);
//	print_sem_queue(sem->wait_queue);
	eos_resume_task(next_task);

	return;
}

void eos_init_condition(eos_condition_t *cond, int32u_t queue_type) {
	/* initialization */
	cond->wait_queue = NULL;
	cond->queue_type = queue_type;
}

void eos_wait_condition(eos_condition_t *cond, eos_semaphore_t *mutex) {
	/* release acquired semaphore */
	eos_release_semaphore(mutex);
	/* wait on condition's wait_queue */
	_os_wait(&cond->wait_queue);
	/* acquire semaphore before return */
	eos_acquire_semaphore(mutex, 0);
}

void eos_notify_condition(eos_condition_t *cond) {
	/* select a task that is waiting on this wait_queue */
	_os_wakeup_single(&cond->wait_queue, cond->queue_type);
}
