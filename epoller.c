#include "epoller.h"
#include "server.h"

void epoller_init(struct epoller *p, logAppender *log_p)
{
    p->epfd = epoll_create1(0); // create epoll fd
    if (p->epfd == -1)
    {
        perror("epoll_create!");
    }
    p->logappender = log_p;
    pthread_mutex_init(&p->epoll_mutex, NULL);
    return;
}

void *epoller_run(void *q)
{

    struct server *p = (struct server *)q;
    p->ep->self_thread_id = pthread_self();
    write(STDOUT_FILENO, "srv: epoller running!\n", 23);
    int start = 0;
    int count = 0;
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, p->exit_sig);
    while (true)
    {
        //pthread_mutex_lock(&p->ep->epoll_mutex);
        count = epoll_wait(p->ep->epfd, p->ep->events, EVENTS_SIZE, 0); // wait util client send data
        //pthread_mutex_unlock(&p->ep->epoll_mutex);
        // printf("wait ok!\n");
        if (count == -1)
        {
            write(STDERR_FILENO, "epoll_wait error!\n", 19);
            pthread_exit(NULL);
        }
        else if (count == 0)
        {
            continue;
        }
        /*lay all unit in dealer queue*/
        sigprocmask(SIG_BLOCK, &sigset, NULL);
        start = 0;
        while (start < count)
        {
            struct work_unit *unit = (struct work_unit *)malloc(sizeof(struct work_unit));
            unit->work_data = p->ep->events[start].data.ptr;
            unit->work_fun = p->de->dealer_do_fun;
            send_work_func(&(p->de->dealer_thread_pool), unit);
            start++;
        }
        if (!p->ep->logappender)
        {
            continue;
        }
        else
        {
            start = 0;
            while (start < count)
            {
                struct logInfo *loginfo = (struct logInfo *)malloc(sizeof(struct logInfo));
                struct logEvent *logevent = (struct logEvent *)malloc(sizeof(struct logEvent));
                logtimeUpate(logevent);
                logattrUpate(logevent, INFO, __FILE__, p->ep->self_thread_id, __LINE__, __FUNCTION__);
                conn *conn_cli = p->ep->events[start].data.ptr;
                logevent->_log_str_len += sprintf(logevent->_log_str + logevent->_log_str_len, "client %s:%hu send data to server!\n", conn_cli->cli_ip, conn_cli->cli_port);
                loginfo->logevent_p = logevent;
                loginfo->logappender_p = p->ep->logappender;
                struct work_unit *unit = (struct work_unit *)malloc(sizeof(struct work_unit));
                unit->work_fun = logAppender_run;
                unit->work_data = loginfo;
                send_work_func(&p->lo->log_thread_pool, unit);
                start++;
            }
        }
        sigprocmask(SIG_UNBLOCK, &sigset, NULL);
    }
}
