#include "array_queue.h"
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

struct array_queue *array_queue_create(int capacity)
{
	assert(capacity > 0);
	struct array_queue *queue = (struct array_queue *)malloc(sizeof(struct array_queue));
	assert(queue != NULL);
	queue->array = (void **)malloc(sizeof(void *) * capacity);
	assert(queue->array != NULL);
	queue->capacity = capacity;
	queue->size = 0;
	return queue;
}
void array_queue_insert(struct array_queue *queue, void *elem)
{
	queue->array[queue->tail] = elem;
	queue->tail = (queue->tail + 1) % queue->capacity;
	queue->size++;
}
void *array_queue_fetch(struct array_queue *queue)
{
	void *elem = queue->array[queue->head];
	queue->head = (queue->head + 1) % queue->capacity;
	queue->size--;
	return elem;
}
bool array_queue_empty(struct array_queue *queue)
{
	return queue->size == 0 ? true : false;
}

bool array_queue_full(struct array_queue *queue)
{
	return queue->size == queue->capacity ? true : false;
}

void array_queue_destroy(array_queue *queue)
{
	free(queue->array);
	free(queue);
}
