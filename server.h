#ifndef _SERVER_H
#define _SERVER_H

#define READ_BUF_SIZE 1024

#include "log.h"
#include "acceptor.h"
#include "epoller.h"
#include "conner.h"
#include "queue.h"
#include "dealer.h"
#include "logrever.h"
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

    /*logrever*/
    log_rever *lo;

    /*server logappender*/
    logAppender *logappender;

    /*server exit sig*/
    int exit_sig;

} server;
void server_init(server *srv, acceptor *ac, epoller *ep, conner *co, dealer *de, log_rever *lo, logAppender *log_p, int exit_sig);
void server_run(struct server *p);
void server_cmd(struct server *p);
void exit_handle(int sig);
#endif
