#include<future.h>

void insert_in_setq(future_t *f, pid32 pid) {

	if((f->setq_rear+1) % MAX_SIZE == f->setq_front) {
		printf("SetQueue Overflow\n");
		return SYSERR;
	}
	else {
		f->set_queue[f->setq_rear] = pid;
		f->setq_rear = (f->setq_rear + 1) % MAX_SIZE;
		//printf("Queued in SetQ Pid: %d\n", pid);
	}
}

void insert_in_getq(future_t *f, pid32 pid) {

	if((f->getq_rear+1) % MAX_SIZE == f->getq_front) {
		printf("Get Queue Overflow\n");
		return SYSERR;
	}
	else {

		f->get_queue[f->getq_rear] = pid;
		f->getq_rear = (f->getq_rear + 1) % MAX_SIZE;
		//printf("Queued in GetQ Pid: %d\n", pid);
	}
}