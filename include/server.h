#ifndef _SERVER_H
#define _SERVER_H

#define READ_BUF_SIZE 1024

#include "log.h"
#include "acceptor.h"
#include "epoller.h"
#include "conn_pool.h"
#include "array_queue.h"
#include "dealer.h"
#include "thread_pool.h"
#include "cache_pool.h"
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

    /*conn_pool*/
    conn_pool *co;

    /*dealer*/
    dealer *de;

    /*cache_pool*/
    cache_pool *ca;

    /*log client*/
    log_client log_cli;

} server;
void server_init(server *srv, acceptor *ac, epoller *ep, conn_pool *co, dealer *de,cache_pool* ca);
void server_run(struct server *p);
void server_cmd(struct server *p);
void server_destroy(struct server *p);
#endif
