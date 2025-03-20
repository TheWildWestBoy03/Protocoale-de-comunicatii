#ifndef QUEUE_H
#define QUEUE_H

struct queue;
typedef struct queue *queued;

/* create an empty queue */
extern queued queue_create(void);

/* insert an element at the end of the queue */
extern void queue_enq(queued q, void *element);

/* delete the front element on the queue and return it */
extern void *queue_deq(queued q);

/* return a true value if and only if the queue is empty */
extern int queue_empty(queued q);

#endif
