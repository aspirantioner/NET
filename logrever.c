#include "logrever.h"
#include "thread_pool.h"

void log_rever_init(log_rever* p,int queue_capacity, int work_thread_min_num, int work_thread_max_num, int exit_sig, int detect_time, int tolerate_time){
    thread_pool_init(&p->log_thread_pool, queue_capacity, work_thread_min_num, work_thread_max_num, exit_sig, detect_time, tolerate_time);
    return;
}