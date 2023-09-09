#ifndef _THREAD_POOL
#define _THREAD_POOL

#include "array_queue.h"
#include "lio_thread.h"
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>

typedef void *(thread_fun)(void *);
typedef void(send_fun)(void *);
typedef void *(work_fun)(void *);

typedef enum
{
    BUSY,
    EMPTY,
    DEAD,
    UNBIRTH,
} STATUS;

typedef struct thread_worker
{
    int worker_id;
    STATUS worker_status_state;
    struct lio_thread thread;
    struct thread_pool *pool_p;
} thread_worker;

typedef struct thread_regulor
{
    struct lio_thread thread;
    int regulor_detect_time;
    int regulor_tolerate_time;
    struct thread_pool *pool_p;
} thread_regulor;

typedef struct thread_pool
{

    int work_thread_min_num;
    int work_thread_max_num;
    int work_thread_exit_num;
    int work_thread_empty_num;
    int work_thread_busy_num;

    thread_worker *thread_worker_arry;

    struct array_queue *queue;
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_full_cond;
    pthread_cond_t queue_empty_cond;

    thread_regulor *regulor_p;

} thread_pool;

typedef struct work_unit
{
    void *work_fun;
    void *work_data;

} work_unit;

void send_work_func(thread_pool *pool, void *send_elem);
void thread_pool_init(thread_pool *pool, int queue_capacity, int work_thread_min_num, int work_thread_max_num);
void thread_regulor_init(struct thread_regulor *regulor_p, int regulor_detect_time, int regulor_tolerate_time);
void thread_pool_bind_regulor(thread_pool *pool, thread_regulor *regulor_p);
void thread_pool_run(thread_pool *pool);
void thread_pool_destory(thread_pool *pool);
void *thread_pool_worker(void *q);
void *thread_pool_regulor(void *q);
#endif
