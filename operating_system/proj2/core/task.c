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

int32u_t eos_create_task(eos_tcb_t *task, addr_t sblock_start, size_t sblock_size, void (*entry)(void *arg), void *arg, int32u_t priority) {
//	PRINT("  eos_create_task called\n");
//	PRINT("task: 0x%x, priority: %d\n", (int32u_t)task, priority);

	// creating stack space and setting tcb
	task->sp = _os_create_context(sblock_start, sblock_size, entry, arg);
	task->priority = priority;

	// putting task in _os_ready_queue in order
	int i;
	for(i = 0; i < LOWEST_PRIORITY+1; i++){
		if(!_os_ready_queue[i]){
			_os_ready_queue[i] = (_os_node_t*) malloc(sizeof(struct _os_node));
			_os_ready_queue[i]->ptr_data = task;
			break;
		}
	}
//	PRINT("  eos_create_task ended, sp: 0x%x, queue_num: %d\n", (int32u_t)task->sp, i);
}

int32u_t eos_destroy_task(eos_tcb_t *task) {
}

void eos_schedule() {
//	PRINT("  eos_schedule called\n");

	// when no task is running (at initial moment)	
	if(!_os_current_task){
		_os_current_task = _os_ready_queue[1]->ptr_data;
		_os_restore_context(_os_current_task->sp);
		return;		
	}

	// saving context of current task
	addr_t sp_save;
	sp_save = _os_save_context();
	_os_current_task->sp = sp_save;

	// when the scheduler is called just after saving
	if(!sp_save){
		return;
	}

	// choosing next task to run
	eos_tcb_t *_os_next_task;
	if(_os_current_task == _os_ready_queue[1]->ptr_data) _os_next_task = _os_ready_queue[2]->ptr_data;
	else _os_next_task = _os_ready_queue[1]->ptr_data;

	// saving current stack pointer and restoring new context
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
}

int32u_t eos_get_period(eos_tcb_t *task) {
}

int32u_t eos_suspend_task(eos_tcb_t *task) {
}

int32u_t eos_resume_task(eos_tcb_t *task) {
}

void eos_sleep(int32u_t tick) {
}

void _os_init_task() {
	PRINT("initializing task module.\n");

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
}
