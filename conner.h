#ifndef _CONNER_H
#define _CONNER_H

#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>

#define IP_LEN 30
#define MAX_CONN_NUM 1025

typedef struct conn
{
    char cli_ip[IP_LEN];
    in_port_t cli_port;
    int cli_fd;
    struct server *p;
} conn;

typedef struct conner
{
    struct conn *conn_arry;
    int conn_num;
    pthread_mutex_t conn_mutex;
} conner;

void conner_init(conner *p);
void conner_closeall(conner *p);
#endif
