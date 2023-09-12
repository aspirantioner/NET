// #include "cache_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "cache_pool.h"

void *thread_func(void *p)
{
    cache_pool *pool_p = p;
    for (int i = 0; i < 100; i++)
    {
        void *ptr = cache_pool_alloc(pool_p);
        *(int *)ptr = i;
        printf("%d:%p\n", *(int *)ptr, ptr);
        cache_pool_recycle(ptr);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    cache_pool pool;
    cache_pool_init(&pool, sizeof(int), 10, 10);
    pthread_t array[2];
    pthread_create(&array[0], NULL, thread_func, &pool);
    pthread_create(&array[1], NULL, thread_func, &pool);
    pthread_join(array[0], NULL);
    pthread_join(array[1], NULL);
    cache_pool_destroy(&pool);
    return 0;
}