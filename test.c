// #include "cache_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "cache_pool.h"

int main(int argc, char **argv)
{
    cache_pool pool;
    cache_pool_init(&pool, sizeof(long int), 10, 5);
    printf("cache pool p:%p\n", pool.cache_start_p);
    for (int i = 0; i < 20; i++)
    {
        int *p = cache_pool_alloc(&pool);
        printf("%p\n", p);
        cache_pool_recycle(p);
    }

    // pthread_t array[2];
    // pthread_create(&array[0], NULL, thread_func, (void *)&pool);
    // pthread_create(&array[1], NULL, thread_func, (void *)&pool);
    // pthread_join(array[0], NULL);
    // pthread_join(array[1], NULL);
    // cache_pool_destroy(&pool);
    cache_pool_destroy(&pool);
    return 0;
}