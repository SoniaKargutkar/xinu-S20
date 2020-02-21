//globally shared array
extern int arr_q[5];

//globally shared semaphores
extern sid32 full;
extern sid32 empty;
extern sid32 mutex;

//declared globally shared read and write indices
extern int front; //head --remove
extern int rear; //tail -- remove

//function declaration
void consumer_bb(int count);
void producer_bb(int count);