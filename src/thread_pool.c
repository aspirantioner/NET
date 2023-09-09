#include "thread_pool.h"
#include "cache_pool.h"
#include "server.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

/*
produceor send work_unit to work_queue
pool: receive unit worker in thread pool
send_elem: send work unit
*/
void send_work_func(thread_pool *pool, void *send_elem)
{
    if (send_elem == NULL)
    {
        write(STDOUT_FILENO, "unit is NULL!\n", 15);
        exit(0);
    }

    /*send work unit util queue not full*/
    pthread_mutex_lock(&pool->queue_mutex);
    while (array_queue_full(pool->queue))
    {
        pthread_cond_wait(&pool->queue_empty_cond, &pool->queue_mutex);
    }
    array_queue_insert(pool->queue, send_elem);
    pthread_cond_signal(&pool->queue_full_cond);
    pthread_mutex_unlock(&pool->queue_mutex);
    return;
}

/*init work thread pool
min work thread num
max work thread num
exit work thread num
empty work thread num
busy work thread num
regulor thread run func
worker thread run func
thread can receive exit sig
*/
void thread_pool_init(thread_pool *pool, int queue_capacity, int work_thread_min_num, int work_thread_max_num)
{
    /*init and set queue size*/
    pool->queue = array_queue_create(queue_capacity);
    pool->queue->capacity = queue_capacity;

    /*set min and max thread num*/
    pool->work_thread_min_num = work_thread_min_num;
    pool->work_thread_max_num = work_thread_max_num;

    /*exit is min num and all is empty whhen start*/
    pool->work_thread_exit_num = work_thread_min_num;
    pool->work_thread_empty_num = work_thread_min_num;
    pool->work_thread_busy_num = 0;

    /*init mutex and cond*/
    pthread_mutex_init(&pool->queue_mutex, NULL);
    pthread_cond_init(&pool->queue_empty_cond, NULL);
    pthread_cond_init(&pool->queue_full_cond, NULL);

    /*set arry for restore work thread info*/
    pool->thread_worker_arry = (thread_worker *)malloc(work_thread_max_num * sizeof(struct thread_worker));

    /*min num thread empty and other birth when init*/
    for (int i = 0; i < pool->work_thread_max_num; i++)
    {
        pool->thread_worker_arry[i].pool_p = pool;
        pool->thread_worker_arry[i].worker_id = i + 1;
        lio_thread_set(&(pool->thread_worker_arry[i].thread), thread_pool_worker, pool->thread_worker_arry + i);
        /*set thread worker state*/
        if (i >= pool->work_thread_exit_num)
        {
            pool->thread_worker_arry[i].worker_status_state = UNBIRTH;
        }
        else
        {
            pool->thread_worker_arry[i].worker_status_state = EMPTY;
        }
    }

    /*set init regulor is null*/
    pool->regulor_p = NULL;
}

/*
bind a regulor for thread pool
*/
void thread_pool_bind_regulor(thread_pool *pool, thread_regulor *regulor_p)
{
    if (regulor_p)
    {
        pool->regulor_p = regulor_p;
        regulor_p->pool_p = pool;
        lio_thread_set(&regulor_p->thread, thread_pool_regulor, regulor_p);
    }
}

/*
set thread pool regulor detect and telorate time
*/
void thread_regulor_init(struct thread_regulor *regulor_p, int regulor_detect_time, int regulor_tolerate_time)
{
    regulor_p->regulor_detect_time = regulor_detect_time;
    regulor_p->regulor_tolerate_time = regulor_tolerate_time;
}

/*
start worker thread and regulor thread
first run worker thread
then run regulor thread
*/
void thread_pool_run(thread_pool *pool)
{

    /*create worker thread*/
    for (int i = 0; i < pool->work_thread_exit_num; i++)
    {
        lio_thread_run(&pool->thread_worker_arry[i].thread);
    }

    if (pool->regulor_p)
    {
        lio_thread_run(&pool->regulor_p->thread);
    }
}

/*
worker thread run fuc
*/
void *thread_pool_worker(void *q)
{
    void **args = q;
    struct lio_thread *lio_thread_p = args[0];
    struct thread_worker *w = args[1];
    struct thread_pool *p = w->pool_p;

    /*loop wait work unit and do*/
    for (;;)
    {
        /*receive work unit if queue not empty*/
        pthread_mutex_lock(&p->queue_mutex);
        while (array_queue_empty(p->queue) && lio_thread_p->state == THREAD_RUN)
        {
            pthread_cond_wait(&p->queue_full_cond, &p->queue_mutex);
        }

        if (lio_thread_p->state == THREAD_QUIT)
        {
            pthread_mutex_unlock(&p->queue_mutex);
            return NULL;
        }

        /*set state is busy*/
        w->worker_status_state = BUSY;
        p->work_thread_busy_num++;
        p->work_thread_empty_num--;

        /*fetch one unit in queue*/
        struct work_unit *unit = array_queue_fetch(p->queue);
        pthread_cond_signal(&p->queue_empty_cond);
        pthread_mutex_unlock(&p->queue_mutex);

        work_fun *work = unit->work_fun;
        work(unit->work_data);
        cache_pool_recycle((void *)unit);

        /*change state after finish task*/
        pthread_mutex_lock(&p->queue_mutex);
        w->worker_status_state = EMPTY;
        p->work_thread_busy_num--;
        p->work_thread_empty_num++;
        pthread_mutex_unlock((&p->queue_mutex));
    }
}

/*
destory thread pool
first kill thread regulor
then kill all exit thread worker
*/
void thread_pool_destory(thread_pool *p)
{
    /*kill regulor*/
    if (p->regulor_p)
    {
        pthread_mutex_lock(&p->queue_mutex);
        // pthread_kill(p->thread_regulor_id, p->exit_sig);
        lio_thread_exit(&p->regulor_p->thread);
        pthread_mutex_unlock(&p->queue_mutex);
        pthread_join(p->regulor_p->thread.thread_id, NULL);
        write(STDOUT_FILENO, "regulor exit!\n", 15);
    }
    /*wait all task is done*/
    pthread_mutex_lock(&p->queue_mutex);
    while (p->queue->size != 0 && p->work_thread_busy_num != 0)
    {
        pthread_cond_signal(&p->queue_full_cond);
        // write(STDOUT_FILENO,"send signal ok!\n",17);
        pthread_cond_wait(&p->queue_empty_cond, &p->queue_mutex);
        // write(STDOUT_FILENO,"receive signal ok!\n",20);
    }
    write(STDOUT_FILENO, "reset clear!\n", 14);
    // printf("rest clear!\n");
    // printf("thread busy num is %d\n", p->work_thread_busy_num);
    /*shutdown all existed worker thread*/
    for (int i = 0; i < p->work_thread_max_num; i++)
    {
        if (p->thread_worker_arry[i].worker_status_state == EMPTY)
        {
            lio_thread_exit(&(p->thread_worker_arry[i].thread));
        }
    }
    pthread_cond_broadcast(&p->queue_full_cond);
    pthread_mutex_unlock(&p->queue_mutex);
    // printf("start thread join!\n");
    for (int i = 0; i < p->work_thread_max_num; i++)
    {
        if (p->thread_worker_arry[i].worker_status_state == EMPTY)
        {
            pthread_join(p->thread_worker_arry[i].thread.thread_id, NULL);
        }
    }
    // write(STDOUT_FILENO,"thread all closed!\n",20);

    /*destroy condition and mutex*/
    pthread_mutex_destroy(&p->queue_mutex);
    pthread_cond_destroy(&p->queue_empty_cond);
    pthread_cond_destroy(&p->queue_full_cond);

    array_queue_destroy(p->queue);
    free(p->thread_worker_arry);
}

/*
regulor thread run fuc
see worker in thread pool state
full then add new empty thread worker
empty then kill empty thread worker
*/
void *thread_pool_regulor(void *q)
{
    void **args = q;
    struct lio_thread *lio_thread_p = args[0];
    struct thread_regulor *r = args[1];
    struct thread_pool *p = r->pool_p;
    while (true)
    {
        sleep(r->regulor_detect_time);
        if (lio_thread_p->state == THREAD_QUIT)
        {
            return NULL;
        }

        pthread_mutex_lock(&p->queue_mutex);
        if (lio_thread_p->state == THREAD_QUIT)
        {
            return NULL;
        }

        /*detect if all thread worker is busy state*/
        if (p->work_thread_exit_num == p->work_thread_busy_num)
        {
            pthread_mutex_unlock(&p->queue_mutex);
            sleep(r->regulor_tolerate_time);
            pthread_mutex_lock(&p->queue_mutex);
            if (p->work_thread_exit_num == p->work_thread_busy_num && p->work_thread_exit_num < p->work_thread_max_num) // new a thread worker if rest
            {
                for (int i = 0; i < p->work_thread_max_num; i++)
                {
                    /*new a thread wokrer*/
                    if (p->thread_worker_arry[i].worker_status_state == DEAD || p->thread_worker_arry[i].worker_status_state == UNBIRTH)
                    {
                        p->thread_worker_arry[i].worker_status_state = EMPTY;
                        p->work_thread_exit_num++;
                        p->work_thread_empty_num++;
                        lio_thread_set(&p->thread_worker_arry[i].thread, thread_pool_worker, p->thread_worker_arry + i);
                        lio_thread_run(&p->thread_worker_arry[i].thread);
                        break;
                    }
                }
            }
            pthread_mutex_unlock(&p->queue_mutex);
        }

        /*if empty thread worker num out of min empty num*/
        else if (p->work_thread_empty_num > p->work_thread_min_num)
        {
            pthread_mutex_unlock(&p->queue_mutex);
            sleep(r->regulor_tolerate_time);
            pthread_mutex_lock(&p->queue_mutex);
            if (p->work_thread_empty_num > p->work_thread_min_num)
            {
                /*kill a empty worker thread*/
                for (int i = 0; i < p->work_thread_max_num; i++)
                {
                    if (p->thread_worker_arry[i].worker_status_state == EMPTY)
                    {
                        lio_thread_exit(&p->thread_worker_arry[i].thread);
                        p->thread_worker_arry[i].worker_status_state = DEAD;
                        p->work_thread_exit_num--;
                        p->work_thread_empty_num--;
                        pthread_mutex_unlock(&p->queue_mutex);
                        if (pthread_join(p->thread_worker_arry[i].thread.thread_id, NULL) == -1)
                        {
                            perror("thread join");
                        };
                        break;
                    }
                }
            }
            else
            {
                pthread_mutex_unlock(&p->queue_mutex);
            }
        }
        else
        {
            pthread_mutex_unlock(&p->queue_mutex);
        }
    }
    return NULL;
}
