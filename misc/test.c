#define __USE_XOPEN2K 1

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <setjmp.h>
#include <string.h>
#include <stdint.h>
#include <bits/pthreadtypes-arch.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <unistd.h>

#define COPY(a, b) \
memcpy((void *)a, (void *)b, sizeof(a)); \
a;
pthread_spinlock_t g_spin=0;
struct a
{
    int *b;
};
jmp_buf env;
double divide(double a, double b)
{
    if (b == 0)
    {
        longjmp(env, 2);
    }
    return a / b;
}

int array[] = {1, 2, 3, 2, 3, 9};
int arr_len = 6;
int main()
{
    // void **array = (void **)malloc(sizeof(void *));
    // array[0] = (int*)malloc(sizeof(int));
    // intptr_t p1 = (intptr_t)array;
    // intptr_t p2 = (intptr_t)array[0];
    // printf("%ld\t%ld\n", p1,p2);
    // *((int *)p2) = 3;
    // printf("%d\n", *((int*)array[0]));
    // free(array[0]);
    // free(array);
    pthread_spin_init(&g_spin,PTHREAD_PROCESS_PRIVATE);
    pthread_spin_lock(&g_spin);
    
    pthread_spin_unlock(&g_spin);
    pthread_spin_destroy(&g_spin);
    return 0;
}
