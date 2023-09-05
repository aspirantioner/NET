#pragma once
#ifndef _ARRAY_QUEUE_H
#define _ ARRAY_QUEUE_H

#include <stdbool.h>

typedef struct array_queue
{
	void **array;
	int head;
	int tail;
	int capacity;
	int size;
} array_queue;

struct array_queue *array_queue_create(int capacity);
void array_queue_insert(struct array_queue *queue, void *elem);
void *array_queue_fetch(struct array_queue *queue);
bool array_queue_empty(struct array_queue *queue);
bool array_queue_full(struct array_queue *queue);
void array_queue_destroy(struct array_queue *queue);
#endif
