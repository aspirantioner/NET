#ifndef _LOG_REVER_H
#define _LOG_REVER_H

#include "thread_pool.h"

typedef struct log_rever
{
    thread_pool log_thread_pool;
} log_rever;

void log_rever_init(log_rever* p,int queue_capacity, int work_thread_min_num, int work_thread_max_num, int exit_sig, int detect_time, int tolerate_time);

#endif