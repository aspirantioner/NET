#include "queue.h"
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

/*队列创建*/
Queue *queueCreate(int capacity)
{
    Queue *q = (Queue *)malloc(sizeof(Queue));
    assert(q != NULL);
    q->front = NULL;
    q->rear = NULL;
    q->queue_size = 0;
    q->queue_capacity = capacity;
    return q;
}

/*队列添加*/
bool queueAdd(Queue *q, elem_type e)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    assert(newNode != NULL);
    /*insert element*/
    newNode->data = e;
    newNode->next = NULL;
    if (!q->rear)
    {
        q->front = newNode;
        q->rear = newNode;
    }
    else if (q->rear)
    {
        q->rear->next = newNode;
        q->rear = q->rear->next;
    }
    q->queue_size++; // increase queue element num
    return true;
}

/*队列是否为空*/
bool queueEmpty(Queue *q)
{
    return q->front == NULL ? true : false;
}

/*队列是否为满*/
bool queueFull(Queue *q)
{
    return q->queue_size == q->queue_capacity ? true : false;
}

/*队列取出*/
elem_type queueDelete(Queue *q)
{

    void *e;
    Node *temp;
    if (q->front == q->rear)
    {
        temp = q->front;
        q->front = NULL;
        q->rear = NULL;
    }
    else
    {
        temp = q->front;
        q->front = q->front->next;
    }
    e = temp->data;
    free(temp);      // free dynamic memory
    q->queue_size--; // reduce queue num
    return e;
}
