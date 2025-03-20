#include "queue.h"
#include "list.h"
#include <stdlib.h>
#include <assert.h>

struct queue
{
	list head;
	list tail;
};

queued queue_create(void)
{
	queued q = (queued)malloc(sizeof(queued));
	q->head = q->tail = NULL;
	return q;
}

int queue_empty(queued q)
{
	return q->head == NULL;
}

void queue_enq(queued q, void *element)
{
	if(queue_empty(q)) {
		q->head = q->tail = cons(element, NULL);
	} else {
		q->tail->next = cons(element, NULL);
		q->tail = q->tail->next;
	}
}

void *queue_deq(queued q)
{
	assert(!queue_empty(q));
	{
		void *temp = q->head->element;
		q->head = cdr_and_free(q->head);
		return temp;
	}
}
