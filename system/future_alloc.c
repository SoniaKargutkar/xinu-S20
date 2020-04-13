/* future_alloc.c - future allocation */

#include <future.h>
#include <stdio.h>
/*----------------------------------------------------------------------------
 *  future alloc  -  Create a new future and return the future to tthe caller
 *----------------------------------------------------------------------------
 */
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
