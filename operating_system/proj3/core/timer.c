/********************************************************
 * Filename: core/timer.c
 *
 * Author: wsyoo, RTOSLab. SNU.
 * 
 * Description: 
 ********************************************************/
#include <core/eos.h>

static eos_counter_t system_timer;

// pushing node into alarm_queue in timeout and priority order
void push_alarm_queue(_os_node_t *queue, eos_alarm_t *node){
//	PRINT("push_alarm_queue, node %x, tcb %x, tout %d\n", node, node->arg, node->timeout);

	// searching right position to insert new node
	while(1){
		// end node case
		if(!queue->next){
			queue->next = (_os_node_t*) malloc(sizeof(struct _os_node));
			queue->next->ptr_data = node;
			return;
		}
		else{
			eos_alarm_t *ptr_alarm;
			ptr_alarm = (eos_alarm_t*)queue->next->ptr_data;
			
			// comparing timeout and priority
			if(ptr_alarm->timeout > node->timeout){
				break;
			}
			else if(ptr_alarm->timeout == node->timeout){
				if(((_os_node_t*)ptr_alarm->arg)->priority > ((_os_node_t*)node->arg)->priority){
					break;
				}
			}
		}
		queue = queue->next;
	}

	// inserting a new node
	_os_node_t *new_node;
	new_node = (_os_node_t*) malloc(sizeof(struct _os_node));
	new_node->ptr_data = node;
	new_node->next = queue->next;
	queue->next = new_node;
	return;
}

// popping a desired node from alarm_queue
void pop_alarm_queue(_os_node_t *queue, eos_alarm_t *node){
//	PRINT("pop_alarm_queue\n");

	// empty queue case
	if(!queue) {return;}

	_os_node_t *pop_node;
	pop_node = queue->next;

	// NULL argument case, popping from the front
	if(!node){
		queue->next = pop_node->next;
		free(pop_node);
		return;
	}
	else{
		// searching desired node
		_os_node_t *prev_queue;
		while(1){
			if(!queue->next) {break;}
			prev_queue = queue;
			queue = queue->next;
			if(queue->ptr_data == node){
				// popping
				pop_node = queue;
				prev_queue->next = pop_node->next;
				free(pop_node);
				return;
			}
		}
	}
	return;
}

int8u_t eos_init_counter(eos_counter_t *counter, int32u_t init_value) {
	counter->tick = init_value;
	counter->alarm_queue = NULL;
	return 0;
}

// implemented function to visualize alarm_queue for debuging purpose
void print_alarm_queue(eos_counter_t *counter){
	printf("**print_alarm_queue called**\n");
	_os_node_t *ptr_queue;
	ptr_queue = counter->alarm_queue;
	while(1){
		if(!ptr_queue) break;
		printf("  element: %x\n", ((eos_alarm_t*)ptr_queue->ptr_data)->arg);
		ptr_queue = ptr_queue->next;
	}
		
}

void eos_set_alarm(eos_counter_t* counter, eos_alarm_t* alarm, int32u_t timeout, void (*entry)(void *arg), void *arg) {
//	PRINT("eos_set_alarm, tcb: %x\n", arg);

	pop_alarm_queue(counter->alarm_queue, alarm);
	if(timeout==0 || entry==NULL) {return;}

	// setting alarm entry to push	
	alarm->timeout = timeout + counter->tick;
	alarm->handler = entry;
	alarm->arg = arg;

	// allocating a new node to the alarm_queue
	if(!counter->alarm_queue) {counter->alarm_queue = (_os_node_t*) malloc(sizeof(struct _os_node));}
	push_alarm_queue(counter->alarm_queue, alarm);

//	print_alarm_queue(counter);
	return;
}

eos_counter_t* eos_get_system_timer() {
	return &system_timer;
}

void eos_trigger_counter(eos_counter_t* counter) {
//	PRINT("eos_trigger_counter\n");

	PRINT("tick\n");
	counter->tick++;
//	PRINT("tick %d\n", counter->tick);

	_os_node_t *ptr_queue;
	eos_alarm_t *ptr_alarm;

	// call_back if timeout is met
	while(1){
		ptr_queue = counter->alarm_queue->next;
		if(!ptr_queue) {break;}
		ptr_alarm = ptr_queue->ptr_data;
		if(!ptr_alarm) {break;}
		if(ptr_alarm->timeout == counter->tick){
			pop_alarm_queue(counter->alarm_queue, NULL);
			(ptr_alarm->handler)(ptr_alarm->arg);
		}
		else {break;}
	}
	return;
}

/* Timer interrupt handler */
static void timer_interrupt_handler(int8s_t irqnum, void *arg) {
	/* trigger alarms */
	eos_trigger_counter(&system_timer);
}

void _os_init_timer() {
	eos_init_counter(&system_timer, 0);

	/* register timer interrupt handler */
	eos_set_interrupt_handler(IRQ_INTERVAL_TIMER0, timer_interrupt_handler, NULL);
}
