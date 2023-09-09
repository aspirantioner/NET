#ifndef _CONN_POOL_H
#define _CONN_POOL_H

#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "bitmap.h"

#define IP_LEN 30

typedef struct conn
{
    char cli_ip[IP_LEN];
    in_port_t cli_port;
    int cli_fd;
    struct server *p;
} conn;

typedef struct conn_pool
{
    struct conn *conn_arry;
    int conn_num;
    pthread_spinlock_t conn_spin;
} conn_pool;

void conn_pool_init(conn_pool *p);
void conn_pool_destroy(conn_pool *p);
#endif
