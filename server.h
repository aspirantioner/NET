#ifndef _SERVER_H
#define _SERVER_H

#define READ_BUF_SIZE 1024

#include "log.h"
#include "acceptor.h"
#include "epoller.h"
#include "conner.h"
#include "queue.h"
#include "dealer.h"
#include "thread_pool.h"
#include <pthread.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <stdbool.h>
#include <errno.h>

typedef struct server
{

    /*accepter*/
    acceptor *ac;

    /*epoller*/
    epoller *ep;

    /*conner*/
    conner *co;

    /*dealer*/
    dealer *de;

    /*log client*/
    log_client log_cli;

} server;
void server_init(server *srv, acceptor *ac, epoller *ep, conner *co, dealer *de);
void server_run(struct server *p);
void server_cmd(struct server *p);
void exit_handle(int sig);
void server_destroy(struct server *p);
#endif
