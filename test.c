#include "cache_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void *thread_func(void *p)
{

    cache_pool *pool_p = p;
    void *ptr;
    for (int i = 0; i < 1000; i++)
    {
        while (1)
        {
            pthread_spin_lock(&pool_p->cache_restore_queue_lock);
            if (pool_p->cache_restore_queue.cache_elem_num)
            {
                ptr = cache_pool_alloc(pool_p);
                pthread_spin_unlock(&pool_p->cache_restore_queue_lock);
                break;
            }
            pthread_spin_unlock(&pool_p->cache_restore_queue_lock);
        }
        printf("%d:%p\n", i, ptr);
        pthread_spin_lock(&pool_p->cache_restore_queue_lock);
        cache_pool_recycle(pool_p, ptr);
        pthread_spin_unlock(&pool_p->cache_restore_queue_lock);
    }
    return NULL;
}
struct A{
    int *p;
    struct A *a_p;
};

int main(int argc, char **argv)
{
    cache_pool pool;
    cache_pool_init(&pool, sizeof(struct A), 10, 10);
    cache_pool_alloc(&pool);
    // pthread_t array[2];
    // pthread_create(&array[0], NULL, thread_func, (void *)&pool);
    // pthread_create(&array[1], NULL, thread_func, (void *)&pool);
    // pthread_join(array[0], NULL);
    // pthread_join(array[1], NULL);
    // cache_pool_destroy(&pool);
    return 0;
}