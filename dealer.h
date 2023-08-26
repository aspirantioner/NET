#ifndef _DEALER_H
#define _DEALER_H

#include "thread_pool.h"
#include "log.h"

typedef void *(deal_fun)(void *);

typedef struct dealer
{
    struct thread_pool dealer_thread_pool;
    logAppender *logappender;
    deal_fun *dealer_do_fun;
} dealer;

void dealer_init(dealer *p, logAppender *logappender, deal_fun *dealer_do_fun, int queue_capacity, int work_thread_min_num, int work_thread_max_num, int exit_sig, int detect_time, int tolerate_time);
void *dealer_run(void *p);
#endif