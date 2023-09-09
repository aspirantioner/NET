#ifndef _LINK_QUEUE_H
#define _LINK_QUEUE_H

#include <stdbool.h>

typedef struct link_queue_elem
{
    void *elem_p;
    struct link_queue_elem *elem_next;
} link_queue_elem;

typedef struct link_queue
{
    struct link_queue_elem *queue_head;
    struct link_queue_elem *queue_tail;
    int link_queue_size;
} link_queue;

struct link_queue *link_queue_create();
void link_queue_insert(struct link_queue* link_queue_p,struct link_queue_elem* elem);
struct link_queue_elem* link_queue_fecth(struct link_queue* link_queue_p);
bool link_queue_is_empty(struct link_queue* link_queue_p);

#endif