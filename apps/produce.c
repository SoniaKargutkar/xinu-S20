/* produce.c - produce */

#include <xinu.h>
#include <prodcons.h>
#include <prodcons_bb.h>

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

//adding for assignment 3
void producer_bb(int count) 
{
  // Iterate from 0 to count and for each iteration add iteration value to the global array `arr_q`,
  // print producer process name and written value as,
  // name : producer_1, write : 8
	
	for(int i=1; i<=count; i++)
	{ 	
		wait(full);
		wait(mutex);
		arr_q[rear] = i;
		rear = (rear+1)%5;
		printf("producer name: producer[%d], write: %d\n", getpid(), i);
		signal(mutex);
		signal(empty);
	}
}

