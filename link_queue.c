#include "link_queue.h"
#include <stdlib.h>


struct link_queue *link_queue_create(){
    struct link_queue *link_queue_p = (struct link_queue *)malloc(sizeof(struct link_queue));
    link_queue_p->queue_head = NULL;
    link_queue_p->queue_tail = NULL;
    link_queue_p->link_queue_size = 0;
}

void link_queue_insert(struct link_queue* link_queue_p,struct link_queue_elem* elem){
    if(link_queue_p->link_queue_size == 0){
        link_queue_p->queue_head = elem;
        link_queue_p->queue_tail = elem;
    }
    else{
        link_queue_p->queue_tail->elem_next = elem;
        link_queue_p->queue_tail = elem;
    }
    link_queue_p->link_queue_size++;
}
struct link_queue_elem* link_queue_fecth(struct link_queue* link_queue_p){
    link_queue_elem *elem;
    elem = link_queue_p->queue_head;
    if (link_queue_p->link_queue_size == 1)
    {
        link_queue_p->queue_head = NULL;
        link_queue_p->queue_tail = NULL;
    }
    else{
        link_queue_p->queue_head = link_queue_p->queue_head->elem_next;
    }
    link_queue_p->link_queue_size--;
    return elem;
}

bool link_queue_is_empty(struct link_queue* link_queue_p){
    return link_queue_p->link_queue_size == 0 ? true : false;
}