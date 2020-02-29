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
		if(isGetQueueEmpty(f)) {
			insert_in_setq(f, currpid);
			suspend(currpid);
			
			// after resumption
			f->data = *value;
			signal(mutex);
		}
		else {
			f->data = *value;
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