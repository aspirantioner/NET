#include "acceptor.h"
#include "server.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <assert.h>

/*
init acceptor with ip and port
*/
void acceptor_init(struct acceptor *ac, int ip_type, int socket_type, const char *server_ip, in_port_t server_port, unsigned int listen_queue_size)
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
}

void *acceptor_run(void *q)
{

    write(STDOUT_FILENO, "srv: acceptor running!\n", 24);

    void **args = q;
    struct lio_thread *lio_thread_p = args[0];
    struct server *p = args[1];

    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, LIO_THREAD_QUIT);

    while (true)
    {
        sigprocmask(SIG_UNBLOCK, &sigset, NULL);
        if (lio_thread_p->state == THREAD_QUIT)
        {
            return NULL;
        }

        /*set clid nonblock when accpet*/
        int cli_fd = accept4(p->ac->listen_socket, &p->ac->cli_addr, &p->ac->len, SOCK_NONBLOCK);

        if (lio_thread_p->state == THREAD_QUIT)
        {
            return NULL;
        }
        sigprocmask(SIG_BLOCK, &sigset, NULL);

        if (cli_fd < 0)
        {
            perror("accept");
            exit(0);
        }

        else if (cli_fd > 0)
        {
            /*add cli_fd in epoll*/
            struct epoll_event ev;
            ev.data.fd = cli_fd;

            char *cli_ip = inet_ntoa(p->ac->cli_addr.sin_addr);
            in_port_t cli_port = ntohs(p->ac->cli_addr.sin_port);

            memcpy(p->co->conn_arry[cli_fd].cli_ip, cli_ip, strlen(cli_ip));

            pthread_spin_lock(&p->co->conn_spin);
            p->co->conn_arry[cli_fd].cli_port = cli_port;
            p->co->conn_arry[cli_fd].cli_fd = cli_fd;
            p->co->conn_arry[cli_fd].p = p;
            p->co->conn_num++;
            pthread_spin_unlock(&p->co->conn_spin);

            ev.data.ptr = p->co->conn_arry + cli_fd;
            ev.events = EPOLLIN | EPOLLET;

            if (epoll_ctl(p->ep->epfd, EPOLL_CTL_ADD, cli_fd, &ev) == -1)
            {
                perror("epoll_ctl!");
                exit(0);
            };

            if (p->ac->log_cli.log_pkt.log_append.appendfd > 0) // appender file fd is valid if connect log server
            {
                /*allocate log*/
                logEventInit(&p->ac->log_cli.log_pkt.log_event);
                logtimeUpate(&p->ac->log_cli.log_pkt.log_event);
                logattrUpate(&p->ac->log_cli.log_pkt.log_event, INFO, __FILE__, lio_thread_p->thread_id, __LINE__, __FUNCTION__);
                p->ac->log_cli.log_pkt.log_event._log_str_len += sprintf(p->ac->log_cli.log_pkt.log_event._log_str + p->ac->log_cli.log_pkt.log_event._log_str_len, "client %s:%hu has connected to server!\n", cli_ip, cli_port);
                log_client_write(&p->ac->log_cli);
            }
        }
    }

    return NULL;
}

void acceptor_destroy(struct acceptor* ac_p){

    lio_thread_exit(&ac_p->thread);
    pthread_join(ac_p->thread.thread_id, NULL);
    
    close(ac_p->listen_socket);

    if(ac_p->log_cli.log_pkt.log_append.appendfd>0){
        log_client_close(&ac_p->log_cli);
    }
}
