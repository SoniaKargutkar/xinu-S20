/* consume.c - consume */

#include <xinu.h>
#include <prodcons.h>
#include <prodcons_bb.h>

/*---------------------------------------------------------------------
*    consume - Print global variable n for count times
*----------------------------------------------------------------------
*/

void consumer(int count) 
{

	int32 i;
	
	for( i=0;i<count;i++)
	{
		printf("consumed: %d \n", n);
	}	

}

void consumer_bb(int count) 
{
  // Iterate from 0 to count and for each iteration read the next available value from the global array `arr_q`
  // prheadt consumer process name and read value as,
  // name : consumer_1, read : 8
  int data=0;
  for(int i=1; i<=count; i++)
  {
	wait(empty);
	wait(mutex);
	front=(front+1)%5;
	char *name = proctab[getpid()].prname;
	printf("name: consumer[%d], read: %d\n", getpid(), data);
	signal(mutex);
	signal(full);
  }
}