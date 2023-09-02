#include "epoller.h"
#include "logclient.h"
#include "server.h"
#include "filefd.h"

void epoller_init(struct epoller *p)
{
    p->epfd = epoll_create1(0); // create epoll fd
    if (p->epfd == -1)
    {
        perror("epoll_create!");
    }

    p->events_array = (struct epoll_event *)malloc(sizeof(struct epoll_event) * get_max_file_fd() + 1); // allocate max file fd array
    return;
}

void *epoller_run(void *q)
{
    void **args = q;
    struct lio_thread *lio_thread_p = args[0];
    struct server *p = args[1];

    write(STDOUT_FILENO, "srv: epoller running!\n", 23);
    int start = 0;
    int count = 0;
    int event_max_num = get_max_file_fd() + 1;
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, LIO_THREAD_QUIT);
    while (true)
    {
        // pthread_mutex_lock(&p->ep->epoll_mutex);
        if (lio_thread_p->state == THREAD_QUIT)
        {
            return NULL;
        }

        count = epoll_wait(p->ep->epfd, p->ep->events_array, event_max_num, -1); // wait util client send data

        if (lio_thread_p->state == THREAD_QUIT)
        {
            return NULL;
        }

        if (count == -1)
        {
            write(STDERR_FILENO, "epoll_wait error!\n", 19);
            exit(0);
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
            unit->work_data = p->ep->events_array[start].data.ptr;
            unit->work_fun = p->de->dealer_do_fun;
            send_work_func(p->de->dealer_thread_pool_p, unit);
            start++;
        }

        if (p->ep->log_cli.log_pkt.log_append.appendfd > 0)
        {
            start = 0;
            while (start < count)
            {
                /*allocate log*/
                conn *conn_cli = p->ep->events_array[start].data.ptr;
                logEventInit(&p->ep->log_cli.log_pkt.log_event);
                logtimeUpate(&p->ep->log_cli.log_pkt.log_event);
                logattrUpate(&p->ep->log_cli.log_pkt.log_event, INFO, __FILE__, lio_thread_p->thread_id, __LINE__, __FUNCTION__);
                p->ep->log_cli.log_pkt.log_event._log_str_len += sprintf(p->ep->log_cli.log_pkt.log_event._log_str + p->ep->log_cli.log_pkt.log_event._log_str_len, "client %s:%hu send data to server!\n", conn_cli->cli_ip, conn_cli->cli_port);
                log_client_write(&p->ep->log_cli);
                start++;
            }
        }
        sigprocmask(SIG_UNBLOCK, &sigset, NULL);
    }
}