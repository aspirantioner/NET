#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>


typedef void* elem_type;

typedef struct Node
{
    elem_type data;
    struct Node *next;
} Node; // 队列单元

typedef struct Queue
{
    Node *front;
    Node *rear;
    unsigned int queue_size;
    unsigned int queue_capacity;
} Queue; // 队列

Queue *queueCreate(int capacity); // 创建队列

bool queueAdd(Queue *q, void *e); // 添加队列

bool queueEmpty(Queue *q); // 队列是否为空

bool queueFull(Queue *q);//队列是否装满

elem_type queueDelete(Queue *q); // 取出队列单元

#endif