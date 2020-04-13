//#include <stddef.h>
#include <xinu.h>
#define MAX_SIZE 100
#ifndef _FUTURE_H_
#define _FUTURE_H_
#define QUEUE_SIZE 100

typedef enum {
  FUTURE_EMPTY,
  FUTURE_WAITING,
  FUTURE_READY
} future_state_t;

typedef enum {
  FUTURE_EXCLUSIVE,
  FUTURE_SHARED,
  FUTURE_QUEUE
} future_mode_t;

typedef struct future_t {
  char *data;
  uint size;
  future_state_t state;
  future_mode_t mode;
  pid32 pid;
  //qid16 set_queue;
 // qid16 get_queue;
  pid32 set_queue[QUEUE_SIZE];
  pid32 get_queue[QUEUE_SIZE];
  int32 setq_front, setq_rear, getq_front, getq_rear;
  // int32 front_s, front_g, rear_s, rear_g;
  uint16 max_elems;
  uint16 count;
  uint16 head;
  uint16 tail;
  struct data_element *queue;
} future_t;


void insert_in_setq(future_t *, pid32);
void insert_in_getq(future_t *, pid32);
pid32 remove_from_setq(future_t *);
pid32 remove_from_getq(future_t *);

/* Interface for the Futures system calls */
future_t* future_alloc(future_mode_t mode, uint size, uint n);
syscall future_free(future_t*);
syscall future_get(future_t*, char*);
syscall future_set(future_t*, char*);

uint future_prod(future_t*, char*);
uint future_cons(future_t* );

void stream_consumer_future(int32, future_t*);

extern sid32 mutex;
extern int zero, one, two;
extern future_t **fibfut;

#endif /* _FUTURE_H_ */