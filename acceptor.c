#include "acceptor.h"
#include "server.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <assert.h>

/*
init acceptor with ip and port
*/
void acceptor_init(struct acceptor *ac, int ip_type, int socket_type, const char *server_ip, in_port_t server_port, unsigned int listen_queue_size, logAppender *log_p)
{

    /*init socket*/
    ac->listen_socket = socket(ip_type, socket_type, 0);
    assert(ac->listen_socket != -1);
    /*bind socket witch ip and port*/
    ac->server_ip = server_ip;
    ac->server_port = server_port;
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(ac->server_port);
    server_addr.sin_addr.s_addr = inet_addr(ac->server_ip);
    assert(bind(ac->listen_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) != -1);

    /*set listen queue size*/
    ac->listen_queue_size = listen_queue_size;
    listen(ac->listen_socket, ac->listen_queue_size);
    ac->len = sizeof(ac->cli_addr);
    bzero(&ac->cli_addr, ac->len);

    /*set logappender*/
    ac->logappender = log_p;
}

void *accpetor_run(void *q)
{

    write(STDOUT_FILENO, "srv: acceptor running!\n", 24);

    struct server *p = q;
    p->ac->self_thread_id = pthread_self();
    while (true)
    {
        /*set clid nonblock when accpet*/
        int cli_fd = accept4(p->ac->listen_socket, &p->ac->cli_addr, &p->ac->len, SOCK_NONBLOCK);

        if (cli_fd > 0)
        {
            /*add cli_fd in epoll*/
            struct epoll_event ev;
            ev.data.fd = cli_fd;
            char *cli_ip = inet_ntoa(p->ac->cli_addr.sin_addr);
            memcpy(p->co->conn_arry[cli_fd].cli_ip, cli_ip, strlen(cli_ip));
            pthread_mutex_lock(&p->co->conn_mutex);
            p->co->conn_arry[cli_fd].cli_port = ntohs(p->ac->cli_addr.sin_port);
            p->co->conn_arry[cli_fd].cli_fd = cli_fd;
            p->co->conn_arry[cli_fd].p = p;
            p->co->conn_num++;
            pthread_mutex_unlock(&p->co->conn_mutex);
            ev.data.ptr = p->co->conn_arry + cli_fd;
            ev.events = EPOLLIN | EPOLLET;
            //pthread_mutex_lock(&p->ep->epoll_mutex);
            if (epoll_ctl(p->ep->epfd, EPOLL_CTL_ADD, cli_fd, &ev) == -1)
            {
                perror("epoll_ctl!");
                exit(0);
            };
            //pthread_mutex_unlock(&p->ep->epoll_mutex);

            if (!p->ac->logappender) // appender is null if not bind logappender
            {
                continue;
            }

            /*allocate log*/
            struct logInfo *loginfo = (struct logInfo *)malloc(sizeof(struct logInfo));
            struct logEvent *logevent = (struct logEvent *)malloc(sizeof(struct logEvent));
            logtimeUpate(logevent);
            logattrUpate(logevent, INFO, __FILE__, p->ac->self_thread_id, __LINE__, __FUNCTION__);
            logevent->_log_str_len += sprintf(logevent->_log_str + logevent->_log_str_len, "client %s:%hu has connect server!\n", inet_ntoa(p->ac->cli_addr.sin_addr), ntohs(p->ac->cli_addr.sin_port));
            loginfo->logevent_p = logevent;
            loginfo->logappender_p = p->ac->logappender;
            struct work_unit *unit = (struct work_unit *)malloc(sizeof(struct work_unit));
            unit->work_fun = logAppender_run;
            unit->work_data = loginfo;
            send_work_func(&p->lo->log_thread_pool, unit);
        }
    }

    return NULL;
}