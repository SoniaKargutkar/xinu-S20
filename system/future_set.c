/* future_set.c - set the value of the future */

#include <future.h>

/*------------------------------------------------------------------------
 *  future set  -  Set a value and changes state from FUTURE_EMPTY 
 *		   or FUTURE_WAITING to FUTURE_VALID
 *------------------------------------------------------------------------
 */
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