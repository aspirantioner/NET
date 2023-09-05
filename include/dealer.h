#ifndef _DEALER_H
#define _DEALER_H

#include "thread_pool.h"
#include "log.h"
#include "logclient.h"

typedef void *(deal_fun)(void *);

typedef struct dealer
{
    struct thread_pool *dealer_thread_pool_p;
    log_client log_cli;
    deal_fun *dealer_do_fun; // receive client data callback func
} dealer;

void dealer_init(dealer *p, struct thread_pool *dealer_thread_pool_p, deal_fun *dealer_func);
void *dealer_run(void *p);
#endif
