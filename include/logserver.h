#ifndef _LOG_SERVER_H
#define _LOG_SREVER_H

#include "log.h"
#include "bitmap.h"
#include "thread_pool.h"
#include "cache_pool.h"
#include "lio_thread.h"
#include <pthread.h>
#include <bits/pthreadtypes.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct log_server
{
    const char *server_ip;
    in_port_t server_port;
    struct sockaddr_in log_server_addr;
    int log_socket;

    struct thread_pool *log_thread_pool_p;

    struct lio_thread thread; // log server receive log event deal thread

    struct bitmap *bitmap_p; // store file fd bit-map

    struct cache_pool *cache_pool_p;
} log_server;

typedef struct log_packet
{
    struct logEvent log_event;
    struct logAppender log_append;
} log_packet;

typedef struct log_client_set
{
    log_packet log_pkt;
    struct sockaddr_in client_addr;
    log_server* log_server_p;
} log_client_set;

void log_server_init(struct log_server *log_server_p, const char *server_ip, in_port_t server_port, struct thread_pool *thread_pool_p, struct bitmap *bitmap_p, struct cache_pool* cache_pool_p);
void log_server_run(struct log_server *log_server_p);
void *log_server_receive(void *p);
void *log_deal(void *p);
void log_server_exit(struct log_server *log_server_p);
#endif
