#include <semaphore.h>
#include <stdlib.h>

#include "queue.h"

struct queue {
  int size;   // size of queue
  int start;  // start of queue
  int end;    // end of queue
  sem_t push; // semaphore that locks push function
  sem_t pop;  // semaphore that locks pop function
  sem_t free; // counting semaphore that counts how many free spaces there are
              // in the queue
  sem_t cont; // coutning sempahore that counts how many elements there are in
              // the queue
  void **data; // list of void * to elements
};

queue_t *q_create(int size) {
  queue_t *q = (queue_t *)malloc(sizeof(queue_t));
  if (q == NULL)
    return NULL;
  q->size = size + 1; //+1 to check if queue if full
  q->start = 0;
  q->end = 0;
  q->data = (void **)calloc(q->size, sizeof(void *));
  sem_init(&q->push, 0, 1);
  sem_init(&q->pop, 0, 1);
  sem_init(&q->free, 0, q->size);
  sem_init(&q->cont, 0, 0);
  return q;
}

void q_destroy(queue_t **q) {
  if (q == NULL || *q == NULL)
    return;
  free(q[0]->data); // since the start of the queue increments, we free from the
                    // origin
  free(q[0]);
  *q = NULL;
}

bool q_push(queue_t *q, void *elem) {
  if (q == NULL)
    return false;
  sem_wait(&q->free); // decrement as a counter, we do this first so that if the
                      // sem is 0, it stalls until we can push
  sem_wait(&q->push); // lock for push
  q->data[q->end] = elem;
  q->end = (q->end + 1) % q->size;
  sem_post(&q->cont); // increment as a counter
  sem_post(&q->push); // unlock
  return true;
}

bool q_pop(queue_t *q, void **elem) {
  if (q == NULL)
    return false;
  sem_wait(&q->cont); // decrement counter for num elements to stall if it is 0
  sem_wait(&q->pop);  // lock
  *elem = q->data[q->start];
  q->start = (q->start + 1) % q->size;
  sem_post(&q->free); // increment number of available spaces
  sem_post(&q->pop);  // unlock
  return true;
}
