#include<future.h>

pid32 remove_from_setq(future_t *f) {

	if(f->setq_front == f->setq_rear) {
		printf("SetQueue Underflow\n");
		return SYSERR;
	}
	else {
		pid32 pid = f->set_queue[f->setq_front];
		f->setq_front = (f->setq_front + 1) % MAX_SIZE;
		//printf("Dequeued SetQ Pid: %d\n", pid);
		return pid;
	}
}

pid32 remove_from_getq(future_t *f) {
	if(f->getq_front == f->getq_rear) {
		printf("GetQueue Underflow\n");
		return SYSERR;
	}
	else {
		pid32 pid = f->get_queue[f->getq_front];
		f->getq_front = (f->getq_front + 1) % MAX_SIZE;
		//printf("Dequeued GetQ Pid: %d\n", pid);
		return pid;
	}
}

bool8 isGetQueueEmpty(future_t *f) {
	return (f->getq_front == f->getq_rear);		
}

bool8 isSetQueueEmpty(future_t *f) {
	return (f->setq_front == f->setq_rear);		
}