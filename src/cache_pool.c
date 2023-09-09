#include "cache_pool.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void cache_pool_init(struct cache_pool *cache_pool_p, int cache_elem_size, int cache_elem_sum, int cache_elem_recycle_limit)
{

    /*init cache pool memory*/
    int sum_size = cache_elem_size + 2 * sizeof(void *);
    cache_pool_p->cache_start_p = (void *)malloc(sum_size * cache_elem_sum);
    if (!cache_pool_p->cache_start_p)
    {
        perror("malloc");
        exit(0);
    }
    cache_pool_p->cache_elem_sum = cache_elem_sum;                     // set cache pool elem number
    cache_pool_p->cache_elem_size = cache_elem_size;                   // set cache pool elem memory size
    cache_pool_p->cache_elem_recycle_limit = cache_elem_recycle_limit; // set recycle elem num limit

    /*suspend elem in restore link queue*/
    cache_pool_p->cache_restore_queue.cache_queue_head = (intptr_t)cache_pool_p->cache_start_p;
    for (int i = 0; i < cache_elem_sum; i++)
    {
        *((intptr_t *)(cache_pool_p->cache_restore_queue.cache_queue_head + i * sum_size)) = (intptr_t)(cache_pool_p->cache_start_p + (i + 1) * sum_size);
        *((intptr_t *)(cache_pool_p->cache_restore_queue.cache_queue_head + i * sum_size + sizeof(intptr_t))) = (intptr_t)cache_pool_p;
    }

    cache_pool_p->cache_restore_queue.cache_queue_tail = (intptr_t)cache_pool_p->cache_start_p + (sum_size * (cache_elem_sum - 1));
    cache_pool_p->cache_restore_queue.cache_elem_num = cache_elem_sum;

    /*init recycle link queue*/
    cache_pool_p->cache_recycle_queue.cache_queue_head = 0;
    cache_pool_p->cache_recycle_queue.cache_queue_tail = 0;
    cache_pool_p->cache_recycle_queue.cache_elem_num = 0;

    /*init restore spin lock*/
    pthread_spin_init(&cache_pool_p->cache_restore_queue_lock, PTHREAD_PROCESS_PRIVATE);
}

void *cache_pool_alloc(struct cache_pool *cache_pool_p)
{
    struct cache_queue *cache_queue_p = &cache_pool_p->cache_restore_queue;
    void *elem_p;
    while (1)
    {
        pthread_spin_lock(&cache_pool_p->cache_restore_queue_lock);
        if (cache_queue_p->cache_elem_num != 0)
        {
            elem_p = (void *)(cache_queue_p->cache_queue_head + 2 * sizeof(intptr_t));
            if (cache_queue_p->cache_elem_num == 1)
            {
                cache_queue_p->cache_queue_head = 0;
                cache_queue_p->cache_queue_tail = 0;
            }
            else
            {
                cache_queue_p->cache_queue_head = *((intptr_t *)cache_queue_p->cache_queue_head);
            }
            cache_queue_p->cache_elem_num--;
            pthread_spin_unlock(&cache_pool_p->cache_restore_queue_lock);
            break;
        }
        pthread_spin_unlock(&cache_pool_p->cache_restore_queue_lock);
    }
    return elem_p;
}

void cache_pool_recycle(void *cache_elem_p)
{
    struct cache_pool *cache_pool_p = (struct cache_pool *)(*(intptr_t *)(cache_elem_p - sizeof(intptr_t)));
    pthread_spin_lock(&cache_pool_p->cache_restore_queue_lock);
    if (cache_pool_p->cache_recycle_queue.cache_elem_num == 0)
    {
        cache_pool_p->cache_recycle_queue.cache_queue_head = (intptr_t)(cache_elem_p - 2 * sizeof(intptr_t));
        cache_pool_p->cache_recycle_queue.cache_queue_tail = (intptr_t)(cache_elem_p - 2 * sizeof(intptr_t));
    }
    else
    {
        *(intptr_t *)cache_pool_p->cache_recycle_queue.cache_queue_tail = (intptr_t)(cache_elem_p - 2 * sizeof(intptr_t));
        cache_pool_p->cache_recycle_queue.cache_queue_tail = (intptr_t)(cache_elem_p - 2 * sizeof(intptr_t));
    }
    cache_pool_p->cache_recycle_queue.cache_elem_num++;

    /*start clear recycle queue if recycle num equal limit*/
    if (cache_pool_p->cache_recycle_queue.cache_elem_num == cache_pool_p->cache_elem_recycle_limit)
    {
        for (int i = 0; i < cache_pool_p->cache_elem_recycle_limit; i++)
        {
            if (cache_pool_p->cache_restore_queue.cache_elem_num == 0)
            {
                cache_pool_p->cache_restore_queue.cache_queue_head = cache_pool_p->cache_recycle_queue.cache_queue_head;
                cache_pool_p->cache_restore_queue.cache_queue_tail = cache_pool_p->cache_recycle_queue.cache_queue_head;
            }
            else
            {
                *(intptr_t *)(cache_pool_p->cache_restore_queue.cache_queue_tail) = cache_pool_p->cache_recycle_queue.cache_queue_head;
                cache_pool_p->cache_restore_queue.cache_queue_tail = cache_pool_p->cache_recycle_queue.cache_queue_head;
            }
            cache_pool_p->cache_recycle_queue.cache_queue_head = *(intptr_t *)(cache_pool_p->cache_recycle_queue.cache_queue_head);
            cache_pool_p->cache_restore_queue.cache_elem_num++;
        }
        cache_pool_p->cache_recycle_queue.cache_elem_num = 0;
    }
    pthread_spin_unlock(&cache_pool_p->cache_restore_queue_lock);
}

void cache_pool_destroy(struct cache_pool *cache_pool_p)
{
    free(cache_pool_p->cache_start_p);
    pthread_spin_destroy(&cache_pool_p->cache_restore_queue_lock);
}