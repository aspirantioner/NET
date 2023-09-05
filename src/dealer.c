#include "dealer.h"
#include "server.h"
#include "thread_pool.h"

void dealer_init(dealer *p, struct thread_pool *dealer_thread_pool_p, deal_fun *dealer_func)
{
    p->dealer_do_fun = dealer_func;
    p->dealer_thread_pool_p = dealer_thread_pool_p;
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
            if (errno == EWOULDBLOCK || errno == EAGAIN) // data has read
            {
                write(cli_conn->cli_fd, read_buf, count); // echo server

                if (p->de->log_cli.log_pkt.log_append.appendfd > 0)
                {
                    logEventInit(&p->de->log_cli.log_pkt.log_event);
                    logtimeUpate(&p->de->log_cli.log_pkt.log_event);
                    logattrUpate(&p->de->log_cli.log_pkt.log_event, INFO, __FILE__, pthread_self(), __LINE__, __FUNCTION__);
                    p->de->log_cli.log_pkt.log_event._log_str_len += sprintf(p->de->log_cli.log_pkt.log_event._log_str + p->de->log_cli.log_pkt.log_event._log_str_len, "client %s:%hu data has read over!\n%s\n", cli_conn->cli_ip, cli_conn->cli_port, read_buf);
                    log_client_write(&p->de->log_cli);
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

            /*update client conn info*/
            pthread_mutex_lock(&p->co->conn_mutex);
            epoll_ctl(p->ep->epfd, EPOLL_CTL_DEL, cli_conn->cli_fd, NULL);
            close(cli_conn->cli_fd);
            p->co->conn_arry[cli_conn->cli_fd].cli_fd = -1;
            p->co->conn_num--;
            pthread_mutex_unlock(&p->co->conn_mutex);

            if (p->de->log_cli.log_pkt.log_append.appendfd > 0)
            {
                logEventInit(&p->de->log_cli.log_pkt.log_event);
                logtimeUpate(&p->de->log_cli.log_pkt.log_event);
                logattrUpate(&p->de->log_cli.log_pkt.log_event, INFO, __FILE__, pthread_self(), __LINE__, __FUNCTION__);
                p->de->log_cli.log_pkt.log_event._log_str_len += sprintf(p->de->log_cli.log_pkt.log_event._log_str + p->de->log_cli.log_pkt.log_event._log_str_len, "client %s:%hu has close connect with server!\n", cli_conn->cli_ip, cli_conn->cli_port);
                log_client_write(&p->de->log_cli);
            }

            break;
        }
        count += ret;
    }
}
