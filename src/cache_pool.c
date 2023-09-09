#include "cache_pool.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void cache_pool_init(struct cache_pool *cache_pool_p, int cache_elem_size, int cache_elem_sum, int cache_elem_recycle_limit)
{

    /*init cache pool memory*/
    cache_pool_p->cache_start_p = (void *)malloc((cache_elem_size + sizeof(intptr_t)) * cache_elem_sum);
    if (!cache_pool_p->cache_start_p)
    {
        perror("malloc");
        exit(0);
    }

    cache_pool_p->cache_elem_sum = cache_elem_sum;
    cache_pool_p->cache_elem_size = cache_elem_size;
    cache_pool_p->cache_elem_recycle_limit = cache_elem_recycle_limit;

    /*suspend elem in restore link queue*/
    cache_pool_p->cache_restore_queue.cache_queue_head = (intptr_t)cache_pool_p->cache_start_p;
    // memcpy((void *)(cache_pool_p->cache_restore_queue.cache_queue_head + 4), &t, 8);
    int sum_size = cache_elem_size + sizeof(void *);
    for (int i = 0; i < cache_elem_sum - 1; i++)
    {
        *((intptr_t *)(cache_pool_p->cache_restore_queue.cache_queue_head + cache_elem_size + i * sum_size)) = (intptr_t)(cache_pool_p->cache_start_p + (i + 1) * sum_size);
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
            elem_p = (void *)cache_queue_p->cache_queue_head;
            if (cache_queue_p->cache_elem_num == 1)
            {
                cache_queue_p->cache_queue_head = 0;
                cache_queue_p->cache_queue_tail = 0;
            }
            else
            {
                cache_queue_p->cache_queue_head = *(intptr_t *)(cache_queue_p->cache_queue_head + cache_pool_p->cache_elem_size);
            }
            cache_queue_p->cache_elem_num--;
            pthread_spin_unlock(&cache_pool_p->cache_restore_queue_lock);
            break;
        }
        pthread_spin_unlock(&cache_pool_p->cache_restore_queue_lock);
    }
    return elem_p;
}

void cache_pool_recycle(struct cache_pool *cache_pool_p, void *cache_elem_p)
{
    pthread_spin_lock(&cache_pool_p->cache_restore_queue_lock);
    if (cache_pool_p->cache_recycle_queue.cache_elem_num == 0)
    {
        cache_pool_p->cache_recycle_queue.cache_queue_head = (intptr_t)cache_elem_p;
        cache_pool_p->cache_recycle_queue.cache_queue_tail = (intptr_t)cache_elem_p;
    }
    else
    {
        *(intptr_t *)(cache_pool_p->cache_recycle_queue.cache_queue_tail + cache_pool_p->cache_elem_size) = (intptr_t)cache_elem_p;
        cache_pool_p->cache_recycle_queue.cache_queue_tail = (intptr_t)cache_elem_p;
        // printf("head next is %p\n", (void *)(*(intptr_t *)(cache_pool_p->cache_recycle_queue.cache_queue_head + cache_pool_p->cache_elem_size)));
    }
    // printf("tail is %p\n", (void *)cache_pool_p->cache_recycle_queue.cache_queue_tail);
    cache_pool_p->cache_recycle_queue.cache_elem_num++;

    if (cache_pool_p->cache_recycle_queue.cache_elem_num == cache_pool_p->cache_elem_recycle_limit)
    {
        // printf("recycle start!\n");
        //  for (int i = 0; i < cache_pool_p->cache_elem_recycle_limit; i++)
        //  {
        //      printf("recycle is %p\n", (void *)cache_pool_p->cache_recycle_queue.cache_queue_head);
        //      cache_pool_p->cache_recycle_queue.cache_queue_head = *(intptr_t *)(cache_pool_p->cache_recycle_queue.cache_queue_head + cache_pool_p->cache_elem_size);
        //  }
        for (int i = 0; i < cache_pool_p->cache_elem_recycle_limit; i++)
        {
            // printf("recycle is %p\n", (void *)cache_pool_p->cache_recycle_queue.cache_queue_head);
            if (cache_pool_p->cache_restore_queue.cache_elem_num == 0)
            {
                cache_pool_p->cache_restore_queue.cache_queue_head = cache_pool_p->cache_recycle_queue.cache_queue_head;
                cache_pool_p->cache_restore_queue.cache_queue_tail = cache_pool_p->cache_recycle_queue.cache_queue_head;
            }
            else
            {
                *(intptr_t *)(cache_pool_p->cache_restore_queue.cache_queue_tail + cache_pool_p->cache_elem_size) = cache_pool_p->cache_recycle_queue.cache_queue_head;
                cache_pool_p->cache_restore_queue.cache_queue_tail = cache_pool_p->cache_recycle_queue.cache_queue_head;
            }
            cache_pool_p->cache_recycle_queue.cache_queue_head = *(intptr_t *)(cache_pool_p->cache_recycle_queue.cache_queue_head + cache_pool_p->cache_elem_size);
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