#ifndef _EPOLLER_H
#define _EPOLLER_H

#define EVENTS_SIZE 1025 // sys max filefd num +1

#include "dealer.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <error.h>
#include <err.h>
#include <stdlib.h>

typedef struct epoller
{
    int epfd;
    struct epoll_event events[EVENTS_SIZE];
    pthread_t self_thread_id;
    logAppender *logappender;
    pthread_mutex_t epoll_mutex;
} epoller;

void epoller_init(struct epoller *p, logAppender *log_p);
void *epoller_run(void *q);
#endif