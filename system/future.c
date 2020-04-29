#include <future.h>
#include <stdio.h>

future_t* future_alloc(
	  future_mode_t future_flag, uint size, uint nelems		/* Flag to be set for this future	*/
	)
{
	intmask mask;
	mask = disable();
	
	/* create a new future */
	future_t *fptr;
		
	/*if((fptr = (future_t *)getmem(((size) + sizeof(future_t)))) == SYSERR)
	{
		printf("Future creation failed");
		return (future_t *)SYSERR;
	}*/

	if((fptr = (future_t *)getmem(((size * nelems) + sizeof(future_t)))) == SYSERR)
	{
		return SYSERR;
	}
	
	/* assign the flag */
	fptr->mode = future_flag;

	/* assign the furure state*/
	fptr->state = FUTURE_EMPTY;

	if(future_flag == FUTURE_SHARED)
	{
		// fptr->data = sizeof(future_t) + (char *)fptr;
		fptr->setq_front, fptr->setq_rear, fptr->getq_front, fptr->getq_rear = 0;
		fptr->size = size;
	}
	/* initialize the front & rear pointers */
	else if(future_flag == FUTURE_QUEUE)
	{
		fptr->data = sizeof(future_t) + (char *)fptr;
		//kprintf("\nSize of future_t: %d, fptr: %d, fptr->data: %d", sizeof(future_t), &fptr, &fptr->data);
		// fptr->data = (future_t *)getmem(sizeof(future_t)*(nelems));
		fptr->head = 0;
		fptr->tail = 0;
		fptr->count = 0;
		fptr->max_elems = nelems;
		fptr->setq_front, fptr->setq_rear, fptr->getq_front, fptr->getq_rear = 0;
		fptr->size = size;
	}
	restore(mask);
	return fptr;
}

syscall future_free(
	  future_t *f		/* Future to be freed	*/
	)
{
	intmask mask;
	mask = disable();

	/* free the future */	
	if(freemem(f,sizeof(future_t)) == SYSERR){
		//restore(mask);
		return SYSERR;
	}

	restore(mask);
	return OK;

} 

syscall future_get(
	  future_t* f,
	  char* value
	)
{
	intmask mask;
	mask = disable();

	/*------------------------------------------------------------------------
	 *  FUTURE_SHARED
	 *------------------------------------------------------------------------
	 */
	if(f->mode == FUTURE_SHARED) {
		if(f->state == FUTURE_READY){
			*value = f->data;
			f->state = FUTURE_EMPTY;
		}
		else if(f->state == FUTURE_WAITING){
			/* enqueue the current process in get_queue  */
			insert_in_getq(f, currpid);
			suspend(currpid);

			/* resume operations */
			*value = f->data;
			f->state = FUTURE_EMPTY;
		}
		else if(f->state == FUTURE_EMPTY){
			/* enqueue the current process in get_queue  */
			f->pid = currpid;
			f->state = FUTURE_WAITING;
			insert_in_getq(f, currpid);
			suspend(currpid);

			/* resume operations */
			*value = f->data;
			f->state = FUTURE_EMPTY;
		}

		/* If Queue is empty, send message */		
		//if(isGetQueueEmpty(f)) {
		//	send(main_pid, "Done!");
		//}
	} //end-if

	/*------------------------------------------------------------------------
	 *  FUTURE_QUEUE
	 *------------------------------------------------------------------------
	 */
	else if(f->mode == FUTURE_QUEUE) {
		if(f->count == 0) {
			insert_in_getq(f, currpid);
			f->state = FUTURE_WAITING;
			suspend(currpid);

			// after resumption
			// *value = f->data[f->head];
			char* headelemptr = f->data + (f->head * f->size);
			// kprintf("\n******in get time %d,\n", f->data[f->head]);
			// memcpy(value, f->data[f->head], sizeof(f->data[f->head]));
			memcpy(value, headelemptr, f->size);
			f->head = (f->head + 1) % f->max_elems;
			f->count = f->count - 1;
			f->state = FUTURE_EMPTY;
		}
		else{
			// *value = f->data;
			char* headelemptr = f->data + (f->head * f->size);
			// memcpy(value, f->data[f->head], sizeof(f->data[f->head]));
			memcpy(value, headelemptr, f->size);
			f->head = (f->head + 1) % f->max_elems;
			f->count = f->count - 1;
			f->state = FUTURE_EMPTY;
			// kprintf("\n inget->time %d\n", *value);
			if(!isSetQueueEmpty(f))
				resume(remove_from_setq(f));
		}
	} //end-if

	else {

		/*------------------------------------------------------------------------
		 *  OTHERS
		 *------------------------------------------------------------------------
		 */
		if(f->state == FUTURE_READY){
			*value = f->data;
			f->state = FUTURE_EMPTY;
		}
		else if(f->state == FUTURE_WAITING){
			//restore(mask);
			return SYSERR;
		}
		else if(f->state == FUTURE_EMPTY){ 
			/*block waiting for it to set and store pid */
			f->pid = currpid;
			f->state = FUTURE_WAITING;

			/* busy wait on the future */
			while(1) {
				if(f->state == FUTURE_READY) {
					/* read future value & break from loop */
					*value = f->data;
					f->state = FUTURE_EMPTY;
					break;
				}
				else {
					/* reschedule */
					resched();
				}
			}
		}
	} //end-else

	restore(mask);
	return OK;
}

syscall future_set(
	  future_t* f,		/* Future to be set	*/
	  char* value		/* the value to be set into the future */
	)
{
	int i;
	intmask mask;
	mask = disable();
	// kprintf("\nF->mode: %s", (future_t *)f->mode);
	// kprintf("\nHere in set future_queue");

	/*------------------------------------------------------------------------
	 *  FUTURE_SHARED
	 *------------------------------------------------------------------------
	 */
	if(f->mode == FUTURE_SHARED) {
		if(f->state == FUTURE_READY){
			restore(mask);		
			return SYSERR;
		}
		else if(f->state == FUTURE_WAITING) {
			/* set the future value and state */
			f->data = *value;
			f->state = FUTURE_READY;

			/* dequeue all processes waiting */
			while(!isGetQueueEmpty(f)) {
				resume(remove_from_getq(f));
			}
		}
		else if(f->state == FUTURE_EMPTY){ 
			/* set the future value and state */
			f->data = *value;
			f->state = FUTURE_READY;
		}
	} //end-if

	/*------------------------------------------------------------------------
	 *  FUTURE_QUEUE
	 *------------------------------------------------------------------------
	 */
	else if(f->mode == FUTURE_QUEUE) {
		//if(isGetQueueEmpty(f)) {
        
        if(f->count == f->max_elems-1)
		{
			// kprintf("\nCOunt: %d, max_elems: %d", f->count, f->max_elems-1);
            restore(mask);
			insert_in_setq(f, currpid);
            f->state = FUTURE_WAITING;
			suspend(currpid);

			// after resumption
			// f->data[f->head] = *value;
			char* tailelemptr = f->data + (f->tail * f->size);
            // memcpy(f->data[f->tail], value, sizeof(value));
			memcpy(tailelemptr, value, f->size);
			// kprintf("\n inset->time %d\n", f->data[f->tail]);
            f->tail = (f->tail + 1) % f->max_elems;
            f->count = f->count + 1;
            f->state = FUTURE_READY;
		}
		else{
			// kprintf("\nStart COunt: %d, max_elems: %d", f->count, f->max_elems-1);
			// f->data[f->head] = *value;
			char* tailelemptr = f->data + (f->tail * f->size);
            // memcpy(f->data[f->tail], value, sizeof(value));
			memcpy(tailelemptr, value, f->size);
            f->tail = (f->tail + 1) % f->max_elems;
            f->count = f->count + 1;
            f->state = FUTURE_READY;
			// kprintf("\nEnd COunt: %d, max_elems: %d", f->count, f->max_elems-1);

            if(!isGetQueueEmpty(f))
			    resume(remove_from_getq(f));
		}
	} //end-if

	/*------------------------------------------------------------------------
	 *  OTHERS
	 *------------------------------------------------------------------------
	 */
	else {
		if(f->state == FUTURE_READY){
			//restore(mask);		
			return SYSERR;
		}
		else if(f->state == FUTURE_WAITING){
			/* set the future value and state */
			f->data = *value;		
			f->state = FUTURE_READY;				
		}
		else if(f->state == FUTURE_EMPTY){
			/* set the future value and state */
			f->data = *value;
			f->state = FUTURE_READY;
		}
	} //end-else


	restore(mask);
	return OK;

} 

