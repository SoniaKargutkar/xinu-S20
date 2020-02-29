/* future_alloc.c - future allocation */

#include <future.h>
/*----------------------------------------------------------------------------
 *  future alloc  -  Create a new future and return the future to tthe caller
 *----------------------------------------------------------------------------
 */
future_t* future_alloc(
	  future_mode_t future_flag, uint size, int n		/* Flag to be set for this future	*/
	)
{
	intmask mask;
	mask = disable();

	/* create a new future */
	future_t *fptr = (future_t *)getmem(size + sizeof(future_t));
	
	
	/* initialize the front & rear pointers */
	fptr->setq_front = fptr->setq_rear = fptr->getq_front = fptr->getq_rear = 0;

	if(fptr == SYSERR){
		restore(mask);
		return NULL;
	}

	/* assign the flag */
	fptr->mode = future_flag;

	/* assign the furure state*/
	fptr->state = FUTURE_EMPTY;

	restore(mask);
	return fptr;
}
