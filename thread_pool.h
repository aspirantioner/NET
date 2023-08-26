#ifndef _THREAD_POOL
#define _THREAD_POOL

#include "array_queue.h"
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>

typedef void *(thread_fun)(void *);
typedef void(send_fun)(void *);
typedef void(work_fun)(void *);

typedef enum
{
    BUSY,
    EMPTY,
    DEAD,
    UNBIRTH,
} STATUS;

typedef struct worker_info
{
    int worker_id;
    pthread_t worker_thread_id;
    STATUS worker_status_state;
    struct thread_pool *p;
} worker_info;

typedef struct thread_pool
{

    int work_thread_min_num;
    int work_thread_max_num;
    int work_thread_exit_num;
    int work_thread_empty_num;
    int work_thread_busy_num;

    worker_info *work_state_arry;
    pthread_t thread_regulor_id;

    struct array_queue *queue;
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_full_cond;
    pthread_cond_t queue_empty_cond;

    int exit_sig;
    bool thread_pool_shutdown;

    int regulor_detect_time;
    int regulor_tolerate_time;

} thread_pool;

typedef struct work_unit
{
    void *work_fun;
    void *work_data;

} work_unit;

void send_work_func(thread_pool *pool, void *send_elem);
void thread_pool_init(thread_pool *pool, int queue_capacity, int work_thread_min_num, int work_thread_max_num, int exit_sig, int detect_time, int tolerate_time);
void thread_pool_run(thread_pool *pool);
void thread_pool_destory(thread_pool *pool);
void *thread_pool_worker(void *q);
void *thread_pool_regulor(void *q);
#endif