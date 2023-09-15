#include "../lio_thread.h"
#include <stdio.h>
#include <unistd.h>
void *thread_func(void *arg)
{
    lio_thread *lio_thread_p = ((void **)arg)[0];
    printf("thread %ld is  running!\n", lio_thread_p->thread_id);
    while (1)
    {
        sleep(5);
        if (lio_thread_p->state == LIO_THREAD_QUIT)
        {
            return NULL;
        }
    }
}

int main(int argc, char **argv)
{
    lio_thread li;
    lio_thread_init(&li, thread_func, NULL);
    lio_thread_run(&li);
    sleep(2);
    lio_thread_stop(&li);
    sleep(1);
    lio_thread_cont(&li);
    sleep(2);
    lio_thread_exit(&li);
    pthread_join(li.thread_id, NULL);
    return 1;
}
