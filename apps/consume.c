/* consume.c - consume */

#include <xinu.h>
#include <prodcons.h>

/*---------------------------------------------------------------------
*    consume - Print global variable n for count times
*----------------------------------------------------------------------
*/

void consumer(int count) {

	int32 i;
	
	for( i=0;i<count;i++)
	{
		printf("consumed: %d \n", n);
	}

}

