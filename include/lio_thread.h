#ifndef _LIO_THREAD_H
#define _LIO_THREAD_H

#define LIO_THREAD_STOP 41
#define LIO_THREAD_CONT 42
#define LIO_THREAD_QUIT 43

#include <pthread.h>

typedef void *(thread_fun)(void *);

typedef enum THREAD_STATE
{
	THREAD_RUN,
	THREAD_QUIT,
	THREAD_STOP
} THREAD_STATE;

typedef struct lio_thread
{
	THREAD_STATE state;
	pthread_t thread_id;
	thread_fun *func;
	void *thread_arg[2];
} lio_thread;

void lio_thread_set(struct lio_thread *lio_thread_p, thread_fun *func, void *thread_arg);
void lio_thread_run(struct lio_thread *lio_thread_p);
void lio_thread_stop(struct lio_thread *lio_thread_p);
void lio_thread_cont(struct lio_thread *lio_thread_p);
void lio_thread_exit(struct lio_thread *lio_thread_p);
void signal_no_handle(int signum);
void signal_stop_handle(int signum);
#endif
