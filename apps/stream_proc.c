#include <xinu.h>
#include <streamproc.h>
#include<string.h>
#include<stdio.h>
#include "tscdf.h"
#include <stdlib.h>

static int pointer_array[2]; 
int work_queue_depth=10;
int num_streams=10;
int time_window=10;
int output_time=10;
int st;
int ts;
int v;


void stream_consumer(int32 id, struct stream *skstr ){ 

    int z=0;
    kprintf("stream_consumer id:%d (pid:%d)\n", id, getpid());
    char result[256];
    struct tscdf *tc = tscdf_init(time_window);
    int qtime=0;  
    int qval=0; 
    uint pcport;
      while(1){
        wait(skstr->spaces);
        wait(skstr->mutex);
        qtime = skstr->queue[skstr->tail].time;
        qval = skstr->queue[skstr->tail].value;
        tscdf_update(tc, skstr->queue[skstr->tail].time , skstr->queue[skstr->tail].value);


        skstr->tail = (skstr->tail + 1) % work_queue_depth ;

      signal(skstr->mutex);
      signal(skstr->items);
       if(qtime==0 ){
                    tscdf_free(tc);
                    kprintf("stream_consumer %d exiting\n", id);
                            break;
                            }

              z++;
     if(z % output_time == 0){
     
     int32 *qarray = tscdf_quartiles(tc);

          if(qarray == NULL) {
            kprintf("tscdf_quartiles returned NULL\n");
            continue;
          }

          sprintf(result, "s%d: %d %d %d %d %d", id, qarray[0], qarray[1], qarray[2], qarray[3], qarray[4]);
          kprintf("%s\n", result);

          }
         }

      ptsend(pcport, getpid());

}
int stream_proc(int nargs, char* args[]) {
  // Parse arguments
    int i;
  char usage[] = "Usage: -s num_streams -w work_queue_depth -t time_window -o output_time\n";

    /* Parse arguments out of flags */
    /* if not even # args, print error and exit */
    char *chh; 
    char c; 
    if (!(nargs % 2)) {
      printf("%s", usage);
      return(-1);
    }
    else {
      i = nargs - 1;
      while (i > 0) {
        chh = args[i-1];
        c = *(++chh);

        switch(c) {
        case 's':
          num_streams = atoi(args[i]);
            printf("This");
          break;

        case 'w':
          work_queue_depth = atoi(args[i]);
          break;

        case 't':
          time_window = atoi(args[i]);
          break;

        case 'o':
          output_time = atoi(args[i]);
          break;

        default:
          printf("%s", usage);
          return(-1);
        }

        i -= 2;
      }
      }

  ulong secs, msecs, time;
  uint pcport;
    secs = clktime;
    msecs = clkticks;
      if((pcport = ptcreate(num_streams)) == SYSERR) {
          printf("ptcreate failed\n");
          return(-1);
      }

  // Create streams
  stream *streamsk; 
    if((streamsk = (stream *)getmem(sizeof(stream)*(work_queue_depth))) == (stream *) SYSERR){
              printf("NOTT WORKINGGG");
              return (stream *)SYSERR;
              }
      for(i=0; i<num_streams; i++){
      streamsk[i].tail = 0;
      streamsk[i].head = 0;
      streamsk[i].items = semcreate(work_queue_depth);
      streamsk[i].mutex = semcreate(1);
      streamsk[i].spaces = semcreate(0);
      streamsk[i].queue = (de *)getmem(sizeof(de) *work_queue_depth);

      }

      //resume(create(stream_producer, 4096,20, "stream_producer", 1, stream1));
  // Create consumer processes and initialize streams
  // Use `i` as the stream id.
  for (i = 0; i < num_streams; i++) {
        resume(create(stream_consumer, 4096,20, "stream_consumer", 2, i,  &streamsk[i]));
  }
  // Parse input header file data and populate work queue
  char *a;
  for(i=0;i<n_input;i++){
     a = (char *)stream_input[i];
        st = atoi(a);
        while (*a++ != '\t');
        ts = atoi(a);
        while (*a++ != '\t');
        v = atoi(a);



          wait(streamsk[st].items);
             wait(streamsk[st].mutex);

            streamsk[st].queue[streamsk[st].head].time = ts;
            streamsk[st].queue[streamsk[st].head].value = v;

             streamsk[st].head = ((streamsk[st].head +1)% work_queue_depth);
            signal(streamsk[st].mutex);
            signal(streamsk[st].spaces);
        //printf("\n\n\n%d %d %d\n\n\n",st, ts,v);
        }


   time = (((clktime * 1000) + clkticks) - ((secs * 1000) + msecs));


    for(i=0; i < num_streams; i++) {
        uint32 pm;
        pm = ptrecv(pcport);
        kprintf("process %d exited\n", pm);

    }

    ptdelete(pcport, 0);

kprintf("time in ms: %u\n", time);


  return 0;
}
