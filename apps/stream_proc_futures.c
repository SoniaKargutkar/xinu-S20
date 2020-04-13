#include<xinu.h>
#include<future.h>
#include<streamproc.h>
#include "tscdf.h"

future_t **farray;
//int i, num_streamss, wq_depths, time_windows, op_time, st, ts ,v, head;

int i;
int n_streams, wq_depths, t_windows, op_time;
int st, ts ,v, head;
uint pcport;


int stream_proc_futures(int nargs, char* args[]) 
{
  ulong secs, msecs, time;
  secs = clktime;
  msecs = clkticks;
  char usage[] = "Usage: -s num_streams -w work_queue_depth -t time_window -o output_time\n";
  
  char *a, *ch, c;
  if (!(nargs % 2)) {
    printf("%s", usage);
    return(-1);
  }
  else {
    int i = nargs - 1;
    
    // Parse arguments
    while (i > 0) {
      ch = args[i-1];
      c = *(++ch);
      
      switch(c) {
      case 's':
        n_streams = atoi(args[i]);
        // printf("%d\n",n_streams);
        break;

      case 'w':
        wq_depths = atoi(args[i]);
        // printf("%d\n",wq_depths);
        break;

      case 't':
        t_windows = atoi(args[i]);
        // printf("%d\n",t_windows);
        break;
        
      case 'o':
        op_time = atoi(args[i]);
        // printf("%d\n",op_time);
        break;
      default:
        printf("%s", usage);
        return(-1);
      }

      i -= 2;
    }
  }

  if((pcport = ptcreate(n_streams)) == SYSERR) {
      printf("ptcreate failed\n");
      return(-1);
  }

  // Create array to hold `n_streams` number of futures
  if ((farray = (future_t *)getmem(sizeof(future_t *) * (n_streams))) == (future_t *)SYSERR)
    {
        printf("getmem failed\n");
        return (SYSERR);
    }

    for (i = 0; i < n_streams; i++)
    {
        if ((farray[i] = future_alloc(FUTURE_QUEUE, sizeof(de), wq_depths)) == (future_t *)SYSERR)
        {
            printf("problem allocating error");
            return ;
        }
    }

  // Create consumer processes and allocate futures
  // Use `i` as the stream id.
  // Future mode = FUTURE_QUEUE
  // Size of element = sizeof(struct data_element)
  // Number of elements = work_queue_depth
  for (i = 0; i < n_streams; i++) 
  {
      resume(create((void *)stream_consumer_future, 4096, 20, "stream_consumer_future", 2, i, farray[i]));
  }

  // Parse input header file data and set future values
  for (i = 0; i < n_input; i++)
    {
        a = (char *)stream_input[i];
        st = atoi(a);
        while (*a++ != '\t')
            ;
        ts = atoi(a);
        while (*a++ != '\t')
            ;
        v = atoi(a);
        de *dataelement = (de *)getmem(sizeof(de));
        dataelement->time = ts;
        dataelement->value = v;
        future_set(farray[st], dataelement);
    }


  // Wait for all consumers to exit
  for (i = 0; i < n_streams; i++)
    {
        uint32 pm;
        pm = ptrecv(pcport);
        printf("process %d exited\n", pm);
    }

    ptdelete(pcport, 0);

    time = (((clktime * 1000) + clkticks) - ((secs * 1000) + msecs));
    printf("time in ms: %u\n", time);
    // free futures array

  return 0;
}

void stream_consumer_future(int32 id, future_t *f)
{
    kprintf("stream_consumer_future id:%d ", id);
    kprintf("(pid:%d)\n", getpid());
    struct tscdf *tc;
    int count = 0;
    tc = tscdf_init(t_windows);
    while (TRUE)
    {
        de *dataelement;
        future_get(f, dataelement);

        if (dataelement->time == 0 && dataelement->value == 0)
        {
            break;
        }

        tscdf_update(tc, dataelement->time, dataelement->value);

        if (count++ == (op_time - 1))
        {
            char output[10];
            int *qarray;
            qarray = tscdf_quartiles(tc);
            if (qarray == NULL)
            {
                kprintf("tscdf_quartiles returned NULL\n");
                continue;
            }
            sprintf(output, "s%d: %d %d %d %d %d \n", id, qarray[0], qarray[1], qarray[2], qarray[3], qarray[4]);
            kprintf("%s", output);
            freemem((char *)qarray, (6 * sizeof(int32)));
            count = 0;
        }
    }
    tscdf_free(tc);
    ptsend(pcport, getpid());
}

