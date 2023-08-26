#include "dealer.h"
#include "server.h"
#include "thread_pool.h"

void dealer_init(dealer *p, logAppender *log_p, deal_fun *dealer_do_fun, int queue_capacity, int work_thread_min_num, int work_thread_max_num, int exit_sig, int detect_time, int tolerate_time)
{
    thread_pool_init(&p->dealer_thread_pool, queue_capacity, work_thread_min_num, work_thread_max_num, exit_sig, detect_time, tolerate_time);
    p->dealer_do_fun = dealer_do_fun;
    p->logappender = log_p;
}

void *dealer_run(void *q)
{

    struct conn *cli_conn = q;
    struct server *p = cli_conn->p;

    int count = 0, ret = 0;
    char read_buf[READ_BUF_SIZE] = {0};
    while (true)
    {
        ret = read(cli_conn->cli_fd, read_buf + count, READ_BUF_SIZE);
        /*data read over*/
        if (ret == -1)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                write(cli_conn->cli_fd, read_buf, count);
                if (p->de->logappender)
                {
                    struct logInfo *loginfo = (struct logInfo *)malloc(sizeof(struct logInfo));
                    struct logEvent *logevent = (struct logEvent *)malloc(sizeof(struct logEvent));
                    logtimeUpate(logevent);
                    logattrUpate(logevent, INFO, __FILE__, pthread_self(), __LINE__, __FUNCTION__);
                    logevent->_log_str_len += sprintf(logevent->_log_str + logevent->_log_str_len, "client %s:%hu data has read over!\n%s\n", cli_conn->cli_ip, cli_conn->cli_port, read_buf);
                    loginfo->logevent_p = logevent;
                    loginfo->logappender_p = p->de->logappender;
                    struct work_unit *unit = (struct work_unit *)malloc(sizeof(struct work_unit));
                    unit->work_fun = logAppender_run;
                    unit->work_data = loginfo;
                    send_work_func(&p->lo->log_thread_pool, unit);
                }
                break;
            }
            else
            {
                pthread_mutex_lock(&p->co->conn_mutex);
                epoll_ctl(p->ep->epfd, EPOLL_CTL_DEL, cli_conn->cli_fd, NULL);
                close(cli_conn->cli_fd);
                p->co->conn_arry[cli_conn->cli_fd].cli_fd = -1;
                p->co->conn_num--;
                pthread_mutex_unlock(&p->co->conn_mutex);
                break;
            }
        }
        /*client close conn*/
        else if (ret == 0)
        {

            pthread_mutex_lock(&p->co->conn_mutex);
            epoll_ctl(p->ep->epfd, EPOLL_CTL_DEL, cli_conn->cli_fd, NULL);
            close(cli_conn->cli_fd);
            p->co->conn_arry[cli_conn->cli_fd].cli_fd = -1;
            p->co->conn_num--;
            pthread_mutex_unlock(&p->co->conn_mutex);
            if (p->de->logappender)
            {
                struct logInfo *loginfo = (struct logInfo *)malloc(sizeof(struct logInfo));
                struct logEvent *logevent = (struct logEvent *)malloc(sizeof(struct logEvent));
                logtimeUpate(logevent);
                logattrUpate(logevent, INFO, __FILE__, pthread_self(), __LINE__, __FUNCTION__);
                logevent->_log_str_len += sprintf(logevent->_log_str + logevent->_log_str_len, "client %s:%hu data has close connect!\n", cli_conn->cli_ip, cli_conn->cli_port);
                loginfo->logevent_p = logevent;
                loginfo->logappender_p = p->de->logappender;
                struct work_unit *unit = (struct work_unit *)malloc(sizeof(struct work_unit));
                unit->work_fun = logAppender_run;
                unit->work_data = loginfo;
                send_work_func(&p->lo->log_thread_pool, unit);
            }
            break;
        }
        count += ret;
    }
}