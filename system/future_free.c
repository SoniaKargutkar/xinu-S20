/* future_free.c - future free */

#include <future.h>

/*------------------------------------------------------------------------
 *  future free  -  Free the allocated future
 *------------------------------------------------------------------------
 */
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