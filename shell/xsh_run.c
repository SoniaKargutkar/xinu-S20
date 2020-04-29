/* xsh_run.c - xsh_run */

#include <xinu.h>
#include <stdio.h>
#include <string.h>
#include <prodcons.h>
#include <prodcons_bb.h>
#include <future.h>
#include <streamproc.h>
#include <fs.h>

#define SIZE 1200

shellcmd xsh_hello(int32, char *[]);
shellcmd xsh_prodcons(int32, char *[]);

void prodcons_bb();
void prodcons();

void futures_test();

int stream_proc();

uint future_prod(future_t*, char*);
uint future_cons(future_t* );
uint fstest(int nargs, char *args[]);

int stream_proc_futures(int, char*);
void futureq_test1 (int nargs, char *args[]);
void futureq_test2 (int nargs, char *args[]);
void futureq_test3 (int nargs, char *args[]);
void fs_testbitmask(void);

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
			
			if(strncmp(args[2], "-fq1", 4) == 0)
			{
				args++;
				resume(create(futureq_test1, 4096, 20, "futureq_test1", 2, nargs, args));
			}	
			else if(strncmp(args[2], "-fq2", 4) == 0)
			{
				args++;
				resume(create(futureq_test2, 4096, 20, "futureq_test2", 2, nargs, args));
			}
			else if(strncmp(args[2], "-fq3", 4) == 0)
			{
				args++; 
				resume(create(futureq_test3, 4096, 20, "futureq_test3", 2, nargs, args));
			}
			else{
				args++;
				nargs--;
				resume(create(futures_test, 4096, 20, "futures_test", 2, nargs, args));
			}
		}
		else if(strncmp(args[1], "tscdf_fq", 8) == 0)
		{
			args++;
			nargs--;
			resume(create(stream_proc_futures, 4096, 20, "stream_proc_futures", 2, nargs, args));	
		}
		else if(strncmp(args[1], "tscdf", 5) == 0)
		{
			args++;
			nargs--;
			resume(create(stream_proc, 4096, 20, "stream_proc", 2, nargs, args));	
		}
		else  if(strncmp(args[1], "fstest", 6) == 0) 
		{
			args++;
			nargs--;
			resume(create(fstest, 4096, 20, "fstest", 2, nargs, args));	
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

void futureq_test1 (int nargs, char *args[]) {
    int three = 3, four = 4, five = 5, six = 6;
    future_t *f_queue;
    f_queue = future_alloc(FUTURE_QUEUE, sizeof(int), 3);

    resume(create(future_cons, 1024, 20, "fcons6", 1, f_queue));
    resume(create(future_cons, 1024, 20, "fcons7", 1, f_queue));
    resume(create(future_cons, 1024, 20, "fcons8", 1, f_queue));
    resume(create(future_cons, 1024, 20, "fcons9", 1, f_queue));
    resume(create(future_prod, 1024, 20, "fprod3", 2, f_queue, (char *)&three));
    resume(create(future_prod, 1024, 20, "fprod4", 2, f_queue, (char *)&four));
    resume(create(future_prod, 1024, 20, "fprod5", 2, f_queue, (char *)&five));
    resume(create(future_prod, 1024, 20, "fprod6", 2, f_queue, (char *)&six));
    sleep(1);
}

void futureq_test2 (int nargs, char *args[]) {
    int seven = 7, eight = 8, nine=9, ten = 10, eleven = 11;
    future_t *f_queue;
    f_queue = future_alloc(FUTURE_QUEUE, sizeof(int), 3);

    resume(create(future_prod, 1024, 20, "fprod10", 2, f_queue, (char *)&seven));
    resume(create(future_prod, 1024, 20, "fprod11", 2, f_queue, (char *)&eight));
    resume(create(future_prod, 1024, 20, "fprod12", 2, f_queue, (char *)&nine));
    resume(create(future_prod, 1024, 20, "fprod13", 2, f_queue, (char *)&ten));
    resume(create(future_prod, 1024, 20, "fprod13", 2, f_queue, (char *)&eleven));

    resume(create(future_cons, 1024, 20, "fcons14", 1, f_queue));
    resume(create(future_cons, 1024, 20, "fcons15", 1, f_queue));
    resume(create(future_cons, 1024, 20, "fcons16", 1, f_queue));
    resume(create(future_cons, 1024, 20, "fcons17", 1, f_queue));
    resume(create(future_cons, 1024, 20, "fcons18", 1, f_queue));
    sleep(1);
}

void futureq_test3 (int nargs, char *args[]) {
    int three = 3, four = 4, five = 5, six = 6;
    future_t *f_queue;
    f_queue = future_alloc(FUTURE_QUEUE, sizeof(int), 3);

    resume( create(future_cons, 1024, 20, "fcons6", 1, f_queue) );
    resume( create(future_prod, 1024, 20, "fprod3", 2, f_queue, (char*) &three) );
    resume( create(future_prod, 1024, 20, "fprod4", 2, f_queue, (char*) &four) );
    resume( create(future_prod, 1024, 20, "fprod5", 2, f_queue, (char*) &five) );
    resume( create(future_prod, 1024, 20, "fprod6", 2, f_queue, (char*) &six) );
    resume( create(future_cons, 1024, 20, "fcons7", 1, f_queue) );
    resume( create(future_cons, 1024, 20, "fcons8", 1, f_queue) );
    resume( create(future_cons, 1024, 20, "fcons9", 1, f_queue) );
    sleep(1);
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

	uint fstest(int nargs, char *args[]) {
    int rval, rval2;
    int fd, i, j;
    char *buf1, *buf2, *buf3;
    
    // printf("The number of arguments are: %d\n", nargs);
    
    /* Output help, if '--help' argument was supplied */
    if (nargs == 2 && strncmp(args[1], "--help", 7) == 0)
    {
        printf("Usage: %s\n\n", args[0]);
        printf("Description:\n");
        printf("\tFilesystem Test\n");
        printf("Options:\n");
        printf("\t--help\tdisplay this help and exit\n");
        return OK;
    }
 
    /* Check for correct number of arguments */
    if (nargs > 1)
    {
        fprintf(stderr, "%s: too many arguments\n", args[0]);
        fprintf(stderr, "Try '%s --help' for more information\n",
                args[0]);
        return SYSERR;
    }
    if (nargs < 1)
    {
        fprintf(stderr, "%s: too few arguments\n", args[0]);
        fprintf(stderr, "Try '%s --help' for more information\n",
                args[0]);
        return SYSERR;
    }
 
#ifdef FS
 
    bs_mkdev(0, MDEV_BLOCK_SIZE, MDEV_NUM_BLOCKS); /* device "0" and default blocksize (=0) and count */
    fs_mkfs(0,DEFAULT_NUM_INODES); /* bsdev 0*/
    fs_testbitmask();
    
    buf1 = getmem(SIZE*sizeof(char));
    buf2 = getmem(SIZE*sizeof(char));
    buf3 = getmem(SIZE*sizeof(char));
    
// Test 1
    // Create test file
    fd = fs_create("Test_File", O_CREAT);
    
    // Fill buffer with random stuff
    for(i=0; i<SIZE; i++)
    {
        j = i%(127-33);
        j = j+33;
        buf1[i] = (char) j;
        // printf("%s", &buf1[i]);
    }
 
    
    rval = fs_write(fd,buf1,SIZE);
    if(rval == 0 || rval != SIZE )
    {
        printf("\n\r File write failed");
        goto clean_up;
    }
 
    // Now my file offset is pointing at EOF file, i need to bring it back to start of file
    // Assuming here implementation of fs_seek is like "original_offset = original_offset + input_offset_from_fs_seek"
    fs_seek(fd,-rval); 
    
    //read the file 
    rval = fs_read(fd, buf2, rval);
    buf2[rval] = '\0';
 
    if(rval == 0)
    {
        printf("\n\r File read failed");
        goto clean_up;
    }
        
    printf("\n\rContent of file %s",buf2);
    
    rval2 = fs_close(fd);
    if(rval2 != OK)
    {
        printf("\n\rReturn val for fclose : %d",rval);
    }
 
// Test 2
   // Link dst file to file from above
   rval2 = fs_link("Test_File", "Dst_Test_File");
   if(rval2 != OK)
   {
       printf("\n\r File link failed");
       goto clean_up;
   }
   int fd1 = fs_open("Dst_Test_File", 0);
   //read the file 
   rval = fs_read(fd1, buf3, rval);
   buf3[rval] = '\0';
 
   if(rval == 0)
   {
       printf("\n\r File read failed");
       goto clean_up;
   }
       
   printf("\n\rContent of file %s",buf3);
 
   rval2 = fs_close(fd1);
   if(rval2 != OK)
   {
       printf("\n\rReturn val for fclose : %d",rval);
   }
 
// Test 3
   rval2 = fs_unlink("Dst_Test_File");
   if(rval2 != OK)
   {
       printf("\n\r File unlink failed");
       goto clean_up;
   }
 
clean_up:
    freemem(buf1,SIZE);
    freemem(buf2,SIZE);
    freemem(buf3,SIZE);
 
#else
    printf("No filesystem support\n");
#endif
 
    return OK;
}
 
void fs_testbitmask(void) {
 
    fs_setmaskbit(31); fs_setmaskbit(95); fs_setmaskbit(159);fs_setmaskbit(223);
    fs_setmaskbit(287); fs_setmaskbit(351); fs_setmaskbit(415);fs_setmaskbit(479);
    fs_setmaskbit(90); fs_setmaskbit(154);fs_setmaskbit(218); fs_setmaskbit(282);
    fs_setmaskbit(346); fs_setmaskbit(347); fs_setmaskbit(348); fs_setmaskbit(349);
    fs_setmaskbit(350); fs_setmaskbit(100); fs_setmaskbit(164);fs_setmaskbit(228);
    fs_setmaskbit(292); fs_setmaskbit(356); fs_setmaskbit(355); fs_setmaskbit(354);
    fs_setmaskbit(353); fs_setmaskbit(352);
    
    fs_printfreemask();
 
    fs_clearmaskbit(31); fs_clearmaskbit(95); fs_clearmaskbit(159);fs_clearmaskbit(223);
    fs_clearmaskbit(287); fs_clearmaskbit(351); fs_clearmaskbit(415);fs_clearmaskbit(479);
    fs_clearmaskbit(90); fs_clearmaskbit(154);fs_clearmaskbit(218); fs_clearmaskbit(282);
    fs_clearmaskbit(346); fs_clearmaskbit(347); fs_clearmaskbit(348); fs_clearmaskbit(349);
    fs_clearmaskbit(350); fs_clearmaskbit(100); fs_clearmaskbit(164);fs_clearmaskbit(228);
    fs_clearmaskbit(292); fs_clearmaskbit(356); fs_clearmaskbit(355); fs_clearmaskbit(354);
    fs_clearmaskbit(353); fs_clearmaskbit(352);
 
    fs_printfreemask();
}
	
	

    

