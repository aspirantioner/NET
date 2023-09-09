#ifndef _CACHE_POOL_H
#define _CACHE_POOL_H

#include <pthread.h>
#include <unistd.h>
#include <stdint.h>

typedef struct cache_queue
{
    intptr_t cache_queue_head;
    intptr_t cache_queue_tail;
    int cache_elem_num;
} cache_queue;

typedef struct cache_pool
{
    void *cache_start_p;
    int cache_elem_size;
    int cache_elem_sum;
    int cache_elem_recycle_limit;
    cache_queue cache_restore_queue;
    cache_queue cache_recycle_queue;
    pthread_spinlock_t cache_restore_queue_lock;
} cache_pool;

void cache_pool_init(struct cache_pool *cache_pool_p, int cache_elem_size, int cache_elem_sum, int cache_elem_recycle_size);
void *cache_pool_alloc(struct cache_pool *cache_pool_p);
void cache_pool_recycle(struct cache_pool *cache_pool_p, void *cache_elem_p);
void cache_pool_destroy(struct cache_pool *cache_pool_p);

#endif