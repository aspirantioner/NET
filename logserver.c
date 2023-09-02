#include "logserver.h"
#include <signal.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void log_server_init(struct log_server *log_server_p, const char *server_ip, in_port_t server_port, struct thread_pool *thread_pool_p, struct bitmap *bitmap_p)
{

    /*bind server ip and port*/
    log_server_p->server_ip = server_ip;
    log_server_p->server_port = server_port;
    log_server_p->log_socket = socket(AF_INET, SOCK_DGRAM, 0);
    log_server_p->log_server_addr.sin_family = AF_INET;
    log_server_p->log_server_addr.sin_port = htons(server_port);
    log_server_p->log_server_addr.sin_addr.s_addr = inet_addr(server_ip);
    int ret = bind(log_server_p->log_socket, (struct sockaddr *)&log_server_p->log_server_addr, sizeof(struct sockaddr));
    if (ret == -1)
    {
        perror("bind error");
    }

    /*set scoket buff size*/
    int buff_size = 0;
    socklen_t opt_len = sizeof(int);
    getsockopt(log_server_p->log_socket, SOL_SOCKET, SO_RCVBUF, &buff_size, &opt_len);
    printf("%d\n", buff_size);
    buff_size *= 2;
    setsockopt(log_server_p->log_socket, SOL_SOCKET, SO_RCVBUF, &buff_size, opt_len);

    log_server_p->log_thread_pool_p = thread_pool_p; // set thread pool
    log_server_p->bitmap_p = bitmap_p;               // set file fd bitmap
}

void *log_server_receive(void *p)
{
    void **args = p;
    struct lio_thread *lio_thread_p = args[0];
    struct log_server *log_server_p = args[1];

    struct sockaddr_in client_addr;
    int addr_len = sizeof(struct sockaddr); // get client ip and port info

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

        struct log_packet *log_pkt_p = (struct log_packet *)malloc(sizeof(struct log_packet));
        int ret = recvfrom(log_server_p->log_socket, log_pkt_p, sizeof(struct log_packet), 0, (struct sockaddr *)&client_addr, &addr_len);

        if (lio_thread_p->state == THREAD_QUIT)
        {
            return NULL;
        }
        else if (ret == -1)
        {
            perror("recvfrom erorr");
            exit(0);
        }
        sigprocmask(SIG_BLOCK, &sigset, NULL);

        void **args = (void **)malloc(sizeof(void *) * 3);
        struct sockaddr_in *client_addr_p = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
        memcpy(client_addr_p, &client_addr, addr_len);
        args[0] = log_server_p;
        args[1] = log_pkt_p;
        args[2] = client_addr_p;
        work_unit *unit_p = (work_unit *)malloc(sizeof(work_unit));
        unit_p->work_data = (void *)args;
        unit_p->work_fun = log_deal;
        send_work_func(log_server_p->log_thread_pool_p, unit_p);
    }
    return NULL;
}

void log_server_run(struct log_server *log_server_p)
{
    /*judge components is valid*/
    assert(log_server_p->log_thread_pool_p != NULL);
    assert(log_server_p->bitmap_p != NULL);

    thread_pool_run(log_server_p->log_thread_pool_p); // firstly run thread pool

    sleep(2);

    lio_thread_set(&log_server_p->thread, log_server_receive, log_server_p);
    lio_thread_run(&log_server_p->thread); // secondly run log receiver
    sleep(2);

    char cmd_buf[1024];
    while (true)
    {
        write(STDOUT_FILENO, "srv: ", strlen("srv: "));
        read(STDIN_FILENO, cmd_buf, 1023);
        if (strncmp(cmd_buf, "exit", 4) == 0)
        {
            log_server_exit(log_server_p);
            write(STDOUT_FILENO, "log server has exit successfully!\n", 35);
            break;
        }
        bzero(cmd_buf, 1024);
    }
}

void *log_deal(void *p)
{
    void **args = p;
    struct log_server *log_server_p = args[0];
    struct log_packet *log_pkt_p = args[1];
    struct sockaddr_in *client_addr_p = args[2];

    /*client request open file fd first*/
    if (log_pkt_p->log_append.appendfd == 0) // notice element arithmetic priority
    {
        write(STDOUT_FILENO, "open file!\n", 12);
        log_pkt_p->log_append.appendfd = open(log_pkt_p->log_append.appendfile, log_pkt_p->log_append.mode, S_IRUSR | S_IWUSR);
        perror("Erorr:");
        printf("open file fd is %d\n", log_pkt_p->log_append.appendfd);
        sendto(log_server_p->log_socket, log_pkt_p, sizeof(struct log_packet), 0, (struct sockaddr *)client_addr_p, sizeof(struct sockaddr));
        btimap_set_one(log_server_p->bitmap_p, log_pkt_p->log_append.appendfd);
    }

    /*close opened client log file fd*/
    else if (log_pkt_p->log_append.mode == -1)
    {
        write(STDOUT_FILENO, "close file fd!\n", 16);
        if (fcntl(log_pkt_p->log_append.appendfd, F_GETFL) != -1)
        {
            close(log_pkt_p->log_append.appendfd);
            btimap_set_zero(log_server_p->bitmap_p, log_pkt_p->log_append.appendfd);
        }
    }

    /*write in log file*/
    else if (log_pkt_p->log_append.mode != -1)
    {
        if (fcntl(log_pkt_p->log_append.appendfd, F_GETFL) != -1) // first judge filefd is valid
        {
            if (log_pkt_p->log_append.mode == O_WRONLY)
            {
                logAppend(&log_pkt_p->log_event, &log_pkt_p->log_append);
            }
        }
    }

    free(log_pkt_p);
    free(client_addr_p);

    return NULL;
}

void log_server_exit(struct log_server *log_server_p)
{
    /*close log  receive socket*/
    close(log_server_p->log_socket);

    /*exit receive log thread*/
    lio_thread_exit(&log_server_p->thread);
    pthread_join(log_server_p->thread.thread_id, NULL);

    /*destroy thread pool*/
    thread_pool_destory(log_server_p->log_thread_pool_p);

    /*close all open file fd*/
    for (int i = 0; i < log_server_p->bitmap_p->bitmap_len; i++)
    {
        if (bitmap_bit_check(log_server_p->bitmap_p, i) == 1)
        {
            write(STDOUT_FILENO, "entry!\n", 8);
            close(i);
        }
    }
    write(STDOUT_FILENO, "log exit!\n", 11);
}
