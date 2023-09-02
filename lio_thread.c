#include "lio_thread.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>

void lio_thread_set(struct lio_thread *lio_thread_p, thread_fun *func, void *thread_arg)
{
	lio_thread_p->thread_arg[0] = lio_thread_p;
	lio_thread_p->thread_arg[1] = thread_arg;
	lio_thread_p->func = func;
	struct sigaction sa;
	sa.sa_handler = signal_no_handle;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if(sigaction(LIO_THREAD_QUIT,&sa,NULL)==-1){
		perror("sigaction");
	}
	if(sigaction(LIO_THREAD_STOP,&sa,NULL)==-1){
		perror("sigaction");
	}
}

void lio_thread_run(struct lio_thread *lio_thread_p)
{
	lio_thread_p->state = THREAD_RUN; // set init thread state
	pthread_create(&lio_thread_p->thread_id, NULL, lio_thread_p->func, (void *)lio_thread_p->thread_arg);
}

void lio_thread_stop(struct lio_thread *lio_thread_p)
{
	lio_thread_p->state = THREAD_STOP;
	pthread_kill(lio_thread_p->thread_id, LIO_THREAD_STOP);
}
void lio_thread_cont(struct lio_thread *lio_thread_p)
{
	lio_thread_p->state = THREAD_RUN;
	pthread_kill(lio_thread_p->thread_id, LIO_THREAD_CONT);
}
void lio_thread_exit(struct lio_thread *lio_thread_p)
{
	lio_thread_p->state = THREAD_QUIT;
	pthread_kill(lio_thread_p->thread_id, LIO_THREAD_QUIT);
}

void signal_no_handle(int signum)
{
	write(STDOUT_FILENO, "receive signal!\n", 17);
}
void signal_stop_handle(int signum)
{

	if (signum == LIO_THREAD_STOP)
	{
		sigset_t set;
		int sig;
		sigemptyset(&set);
		sigaddset(&set, LIO_THREAD_CONT);
		sigprocmask(SIG_BLOCK, &set, NULL);
		sigwait(&set, &sig);
		if (sig == LIO_THREAD_CONT)
		{
			sigprocmask(SIG_UNBLOCK, &set, NULL);
			return;
		}
	}
}
