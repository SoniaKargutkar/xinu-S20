/* xsh_prodcons.c - xsh_prodcons */

#include <xinu.h>
#include <stdio.h>
#include <prodcons.h>

/*------------------------------------------------------------------------
 * xsh_prodcons - creates two processes that share a global variable n
 *------------------------------------------------------------------------
 */

int n; //Definition of global variable 'n'
/* Now global variable n  will be on Heap so it is accessible for all the processes i.e. consume and produce*/

shellcmd xsh_prodcons(int nargs, char *args[])
{
	//Argument verifications and validations
	int count = 2000; //local variable to hold count

	//check args[1] if present assign value to count
	if(nargs == 2)
	{
		count = *args[1];
	}

	//create the process producer and consumer and put them in ready queue.

	resume( create(producer, 1024, 20, "producer", 1, count));
	resume( create(consumer, 1024, 20, "consumer", 1, count));
	return(0);
}

