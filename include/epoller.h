#ifndef _EPOLLER_H
#define _EPOLLER_H

#define EVENTS_SIZE 1025 // sys max filefd num +1

#include "dealer.h"
#include "logclient.h"
#include "lio_thread.h"
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
    struct epoll_event *events_array;
    struct lio_thread thread;
    struct log_client log_cli;
} epoller;

void epoller_init(struct epoller *p);
void *epoller_run(void *q);
#endif