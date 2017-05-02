/********************************************************
 * Filename: core/task.c
 * 
 * Author: parkjy, RTOSLab. SNU.
 * 
 * Description: task management.
 ********************************************************/
#include <core/eos.h>

#define READY		1
#define RUNNING		2
#define WAITING		3

/*
 * Queue (list) of tasks that are ready to run.
 */
static _os_node_t *_os_ready_queue[LOWEST_PRIORITY + 1];

/*
 * Pointer to TCB of running task
 */
static eos_tcb_t *_os_current_task;

// implemented functions for queue management
void push_ready_queue(_os_node_t *queue, eos_tcb_t *push_tcb);
void pop_ready_queue(_os_node_t *queue);

// implemented function to visualize ready_queue for debugging purpose
void print_ready_queue(){
	printf("**print_ready_queue called**\n");
	int j=1;
	while(1){
		_os_node_t *queue = _os_ready_queue[j];
		printf(" priority %d\n", j);
		if(j == 1) j = 10;
		else if(j == 10) j = 50;
		else if(j == 50) j = 63;
		else break;
		int i;
		i = 0;
		while(1){
			if(!queue) break;
			printf("  element: %x\n", queue->ptr_data);
			i++;
			queue = queue->next;
		}
	}
}

int32u_t eos_create_task(eos_tcb_t *task, addr_t sblock_start, size_t sblock_size, void (*entry)(void *arg), void *arg, int32u_t priority) {
	PRINT("  eos_create_task, task: 0x%x, priority: %d\n", (int32u_t)task, priority);

	// creating stack space and setting tcb
	task->sp = _os_create_context(sblock_start, sblock_size, entry, arg);
	task->priority = priority;
	task->state = 1; // 1: ready state

	// putting task in _os_ready_queue
	if(!_os_ready_queue[priority]) {_os_ready_queue[priority] = (_os_node_t*) malloc(sizeof(struct _os_node));}
	push_ready_queue(_os_ready_queue[priority], task);
}

int32u_t eos_destroy_task(eos_tcb_t *task) {
}

// pushing node at the end of ready queue
void push_ready_queue(_os_node_t *queue, eos_tcb_t *push_tcb){
//	PRINT("push_ready_queue called, %x\n", push_tcb);

	// searching last node
	while(1){
		if(!queue->next) {break;}
		queue = queue->next;
	}

	// allocating memory for a new node
	queue->next = (_os_node_t*) malloc(sizeof(struct _os_node));
	queue->next->ptr_data = push_tcb;
	queue->next->next = NULL;

	// set bitmap
	_os_set_ready(push_tcb->priority);
	return;	
}

// popping node from the front of ready queue
void pop_ready_queue(_os_node_t *queue){
//	PRINT("pop_ready_queue called, %x\n", queue->next->ptr_data);

	_os_node_t *pop_node;
	pop_node = queue;
	if(!pop_node) {PRINT("error: pop_ready_queue NULL pointer met\n");}
	else if(!pop_node->next) {PRINT("error: pop_ready_queue NULL pointer met2\n");}

	// choosing the first node
	pop_node = queue->next;
	queue->next = pop_node->next;

	// unset bitmap
	_os_unset_ready(((eos_tcb_t*)pop_node->ptr_data)->priority);

	// freeing used node
	free(pop_node);
	return;
}

void eos_schedule() {
//	PRINT("  eos_schedule called\n");
//	print_ready_queue();

	// when no task is running (at initial moment)
	if(!_os_current_task){
//		PRINT("test: current_task: %x, highest_priority: %d\n", _os_ready_queue[_os_get_highest_priority()]->next->ptr_data, _os_get_highest_priority());
		_os_current_task = _os_ready_queue[_os_get_highest_priority()]->next->ptr_data;
		_os_current_task->state = 0;
		pop_ready_queue(_os_ready_queue[_os_get_highest_priority()]);
		_os_restore_context(_os_current_task->sp);
		return;
	}

	// saving context of current task
	addr_t sp_save;
	sp_save = _os_save_context();
	_os_current_task->sp = sp_save;

	// when the scheduler is called just after saving
	if(!sp_save) {return;}

	// choosing next task to run
	eos_tcb_t *_os_next_task;
	_os_next_task = _os_ready_queue[_os_get_highest_priority()]->next->ptr_data;

	// checking state (0: running, 1: ready, 2: waiting)
	if(_os_current_task->state == 0) {_os_current_task->state = 1;}
	_os_next_task->state = 0;

	// removing next task from queue and adding current task to queue
	pop_ready_queue(_os_ready_queue[_os_get_highest_priority()]);
	if(_os_current_task->state == 1) {push_ready_queue(_os_ready_queue[_os_current_task->priority], _os_current_task);}

//	PRINT("current_task %x, next_task %x\n", _os_current_task, _os_next_task);
	_os_current_task = _os_next_task;
	_os_restore_context(_os_current_task->sp);

	return;
}

eos_tcb_t *eos_get_current_task() {
	return _os_current_task;
}

void eos_change_priority(eos_tcb_t *task, int32u_t priority) {
}

int32u_t eos_get_priority(eos_tcb_t *task) {
}

void eos_set_period(eos_tcb_t *task, int32u_t period){
	task->period = period;
	return;
}

int32u_t eos_get_period(eos_tcb_t *task) {
}

int32u_t eos_suspend_task(eos_tcb_t *task) {
}

int32u_t eos_resume_task(eos_tcb_t *task) {
}

void eos_sleep(int32u_t tick) {
//	PRINT("eos_sleep\n");

	// setting alarm and changing into waiting state
	eos_alarm_t new_alarm;
	eos_set_alarm(eos_get_system_timer(), &new_alarm, _os_current_task->period, _os_wakeup_sleeping_task, _os_current_task);
	_os_current_task->state = 2;

	// calling schedule function
	eos_schedule();
	return;
}

void _os_init_task() {
//	PRINT("initializing task module.\n");

	/* init current_task */
	_os_current_task = NULL;

	/* init multi-level ready_queue */
	int32u_t i;
	for (i = 0; i < LOWEST_PRIORITY; i++) {
		_os_ready_queue[i] = NULL;
	}
}

void _os_wait(_os_node_t **wait_queue) {
}

void _os_wakeup_single(_os_node_t **wait_queue, int32u_t queue_type) {
}

void _os_wakeup_all(_os_node_t **wait_queue, int32u_t queue_type) {
}

void _os_wakeup_sleeping_task(void *arg) {
//	PRINT("_os_wakeup_sleeping_task, %x\n", arg);

	// setting as ready state and pushing to ready_queue
	((eos_tcb_t*)arg)->state = 1;
	push_ready_queue(_os_ready_queue[((eos_tcb_t*)arg)->priority], arg);

	eos_schedule();
	return;
}

