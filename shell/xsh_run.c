/* xsh_run.c - xsh_run */

#include <xinu.h>
#include <prodcons.h>
#include <prodcons_bb.h>
#include <stdio.h>
#include <string.h>
#include <future.h>
#include <streamproc.h>

shellcmd xsh_hello(int32, char *[]);
shellcmd xsh_prodcons(int32, char *[]);

void prodcons_bb();
void prodcons();

void futures_test();

int stream_proc();

//semaphore declaration
sid32 full, empty, mutex;
int front, rear;
/*------------------------------------------------------------------------
 * xsh_run - takes an argument that will map to a function and then run it
 *------------------------------------------------------------------------
 */

//locally declaring globally shared array
int arr_q[5];

int one = 1;
int two = 2;
int zero = 0;
int ffib (int n);
int** final_fib;
future_t **fibfut;

/*
shellcmd xsh_run(int nargs, char *args[])
{
   // if ((nargs == 1) || (strncmp(args[1], "list", 5) == 0)) 
     //{
      //printf("hello\n");
      //printf("prodcons\n");
      //return OK;
   // } 

//This will go past "run" and pass the function/process name and its
   // arguments.
   // 

  

  if ((nargs > 1) || (strncmp(args[1], "list", 5) == 0)) 
     {
      printf("hello\n");
      printf("prodcons\n");
      printf("prodcons_bb\n");

  if(strncmp(args[1], "hello", 13) == 0) 
  {
      // create a process with the function as an entry point. 
      resume (create(xsh_hello, 4096, 20, "hello", 2, nargs, args));
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
  else if(strncmp(args[1], "prodcons", 8) == 0) 
  {
     resume (create(xsh_prodcons, 4096, 20, "prodcons", 2, nargs, args));
     args++;
		 nargs--;
 } 
  else if(strncmp(args[1], "futures_test", 12) == 0) 
  {
     resume (create(futures_test, 4096, 20, "futures_test", 2, nargs, args));
     args++;
		 nargs--;
 } 
  
     }
return (OK);
}
*/


shellcmd xsh_run(int nargs, char *args[])
{
	if(nargs <= 1)
		printf("Too few arguments");
	else
	{	
		if(strncmp(args[1], "prodcons_bb",11) == 0)
		{
			args++;
			nargs--;
			resume(create(prodcons_bb, 4096, 20, "prodcons_bb", 2, nargs, args));
		}
		else if(strncmp(args[1], "prodcons",8) == 0)
		{
			args++;
			nargs--;
			resume(create(prodcons, 4096, 20, "prodcons", 2, nargs, args));
		}
		else if(strncmp(args[1], "futures_test", 12) == 0)
		{
			args++;
			nargs--;
			resume(create(futures_test, 4096, 20, "futures_test", 2, nargs, args));
				
		}
		else if(strncmp(args[1], "tscdf", 5) == 0)
		{
			args++;
			nargs--;
			resume(create(stream_proc, 4096, 20, "stream_proc", 2, nargs, args));	
		}
	}
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
  

void prodcons(int nargs, char *args[])
{
	void producer(int);
	void consumer(int);
	int count = 2000;

	if(nargs == 2)
	{
		count = atoi(args[1]);
		resume(create(producer, 1024, 20, "producer", 1, count));
		resume(create(consumer, 1024, 20, "consumer", 1, count));
	}
	else if(nargs < 2)
		printf("Very few arguments\n");
	else 
		printf("Too many arguments\n");
	
	return (OK);
}  

void futures_test(int nargs, char *code[])
{
	//printf("%s",code[1]);
	if(strncmp(code[1], "-pc", 3) == 0)
	{
		printf("Hello");
		future_t* f_exclusive, * f_shared;
		f_exclusive = future_alloc(FUTURE_EXCLUSIVE, sizeof(int), 1);
		f_shared    = future_alloc(FUTURE_SHARED, sizeof(int), 1);

		// Test FUTURE_EXCLUSIVE
		resume( create(future_cons, 1024, 20, "fcons1", 1, f_exclusive) );
		resume( create(future_prod, 1024, 20, "fprod1", 2, f_exclusive, (char*) &one) );

		// Test FUTURE_SHARED
		resume( create(future_cons, 1024, 20, "fcons2", 1, f_shared) );
		resume( create(future_cons, 1024, 20, "fcons3", 1, f_shared) );
		resume( create(future_cons, 1024, 20, "fcons4", 1, f_shared) );
		resume( create(future_cons, 1024, 20, "fcons5", 1, f_shared) );
		resume( create(future_cons, 1024, 20, "fcons6", 1, f_shared) );
		resume( create(future_prod, 1024, 20, "fprod2", 2, f_shared, (char*) &two) );
	}
	else if(strncmp(code[1], "-f", 2) == 0)
	{
		
		int fibo = -1, i;
		int fibresult;
		fibo = atoi(code[2]);

		//if (fibo > -1) {
			//fibresult = (int **)getmem(fib*sizeof(int*));
			int future_flags = FUTURE_SHARED; // TODO - add appropriate future mode here

			// create the array of future pointers
			//if ((fibfut = (future_t **)getmem(sizeof(future_t *) * (fibo + 1))) == (future_t **) SYSERR) {
				//printf("getmem failed\n");
				//return(SYSERR);
			//}
			
			fibfut = (future_t **)getmem(sizeof(future_t *) * (fibo + 1));
			
			printf("fibfut");
			// get futures for the future array
			for (i=0; i <= fibo+1; i++) {
				/*((fibfut[i] = future_alloc(future_flags, sizeof(int), 1)) == (future_t *) SYSERR) {
					printf("future_alloc failed\n");
					return(SYSERR);
				}*/
				fibfut[i] = future_alloc(future_flags, sizeof(int), 1);
				//printf("fibfut 1");
				//printf(FUTURE_EXCLUSIVE);
				//final_fib[i] = (int *)getmem(sizeof(int));
			}
			
			future_set(fibfut[0],&zero);
			future_set(fibfut[1],&one);
			
			for(i=fibo; i>2; i--){
				//printf("thread created");
				resume(create(ffib,1024,20,"fib",1,i));		
			}
			
			
			
			// spawn fib threads and get final value
			// TODO - you need to add your code here
			
			/*for(int i=0; i<fib; i++)
				printf("%d ", fibfut[i]);*/

			future_get(fibfut[fibo], &fibresult);
			printf("%d",fibfut[fibo]);
			for (i=0; i <= fibo; i++) {
				future_free(fibfut[i]);
			}

			freemem((char *)fibfut, sizeof(future_t *) * (fibo + 1));
			printf("Nth Fibonacci value for N=%d is %d\n", fibo, fibresult);
			printf("Future Fibonnaci of N:%d\n", fibfut[fibo]);
			return(OK);
		//}
	}
	
	
}	



    

