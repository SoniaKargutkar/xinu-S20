/* produce.c - produce */

#include <xinu.h>
#include <prodcons.h>

/*-----------------------------------------------------------------
*  produce - Increment global variable n for count times and print
*------------------------------------------------------------------
*/

void producer(int count) {

	int32 i = 0;
	for(i=0; i<count; i++)
	{
		n =i;
		printf("produced: %d \n",n); 
	}

}



