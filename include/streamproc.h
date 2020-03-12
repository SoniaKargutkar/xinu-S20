
typedef struct data_element {
  int32 time;
  int32 value;
} de;

typedef struct stream {
  sid32 spaces;
  sid32 items;
  sid32 mutex;
  int32 head;
  int32 tail;
  struct data_element *queue;
}stream;


void stream_consumer(int id, struct stream *str);