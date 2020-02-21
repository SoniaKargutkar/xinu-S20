/* xsh_run.c - xsh_run */

#include <xinu.h>
#include <prodcons.h>
#include <prodcons_bb.h>
#include <stdio.h>
#include <string.h>

shellcmd xsh_hello(int32, char *[]);
shellcmd xsh_prodcons(int32, char *[]);

void prodcons_bb();
//void prodcons();

//semaphore declaration
sid32 full, empty, mutex;
int front, rear;
/*------------------------------------------------------------------------
 * xsh_run - takes an argument that will map to a function and then run it
 *------------------------------------------------------------------------
 */

//locally declaring globally shared array
int arr_q[5];


shellcmd xsh_run(int nargs, char *args[])
{
   /*  if ((nargs == 1) || (strncmp(args[1], "list", 5) == 0)) 
     {
      printf("hello\n");
      printf("prodcons\n");
      //return OK;
    } */

/* This will go past "run" and pass the function/process name and its
    * arguments.
    */

  

  if ((nargs > 1) || (strncmp(args[1], "list", 5) == 0)) 
     {
      printf("hello\n");
      printf("prodcons\n");
      printf("prodcons_bb\n");

  if(strncmp(args[1], "hello", 13) == 0) 
  {
      /* create a process with the function as an entry point. */
      resume (create(xsh_hello, 4096, 20, "hello", 2, nargs, args));
      args++;
			nargs--;
  }
  else if(strncmp(args[1], "prodcons", 8) == 0) 
  {
     resume (create(xsh_prodcons, 4096, 20, "prodcons", 2, nargs, args));
     args++;
		 nargs--;
 } 
  else if(strncmp(args[1], "prodcons_bb",13) == 0)
		{
      printf("check");
			args++;
			nargs--;
		resume (create(prodcons_bb, 4096, 20, "prodcons_bb", 2, nargs, args));
		} 
     }
return (OK);
}

    

void prodcons_bb(int nargs, char *args[]) 
{
	//create and initialize semaphores to necessary values
	full = semcreate(5);
	empty = semcreate(0);
	mutex = semcreate(1);

	//initialize read and write indices for the queue
	front = 0;
	rear = 0;

	//create producer and consumer processes and put them in ready queue

	for(int i=1; i<=atoi(args[1]); i++)
		resume(create(producer_bb, 1024, 20, "producer", 1, atoi(args[3])));

	for(int i=1; i<=atoi(args[2]); i++)
		resume(create(consumer_bb, 1024, 20, "consumer", 1, atoi(args[4])));
return (OK);
}
  





    

