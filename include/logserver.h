#ifndef _LOG_SERVER_H
#define _LOG_SREVER_H

#include "log.h"
#include "bitmap.h"
#include "thread_pool.h"
#include "lio_thread.h"
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct log_server
{
    const char *server_ip;
    in_port_t server_port;
    struct sockaddr_in log_server_addr;
    int log_socket;

    struct thread_pool *log_thread_pool_p;

    struct lio_thread thread; // log server receive log event deal thread

    struct bitmap *bitmap_p; // store file fd bit-map
} log_server;

typedef struct log_packet
{
    logEvent log_event;
    logAppender log_append;
} log_packet;

void log_server_init(struct log_server *log_server_p, const char *server_ip, in_port_t server_port, struct thread_pool *thread_pool_p, struct bitmap *bitmap_p);
void log_server_run(struct log_server *log_server_p);
void *log_server_receive(void *p);
void *log_deal(void *p);
void log_server_exit(struct log_server *log_server_p);
#endif
