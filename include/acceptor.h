#ifndef _ACCEPTOR_H
#define _ACCEPTOR_H

#include "log.h"
#include "logclient.h"
#include "lio_thread.h"
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define ACCEPT_INIT_NUM 100
#define ACCEPT_ADD_NUM 20

typedef struct acceptor
{

    const char *server_ip;
    in_port_t server_port;
    int listen_socket;
    unsigned int listen_queue_size;

    struct sockaddr_in cli_addr;
    socklen_t len;

    struct lio_thread thread;
    struct log_client log_cli;
} acceptor;

/*
ip_type:AF_INET or AF_INET6
socket_type:SOCK_STRAEM or SOCK_DGRAM
server_ip:ipv4 or ipv6 str
server_port:bind server port
list_queue_size: listen socket size
log_p: acceptor log appender
*/
void acceptor_init(struct acceptor *ac, int ip_type, int socket_type, const char *server_ip, in_port_t server_port, unsigned int listen_queue_size);

/*
p:server sys pointer
*/
void *accpetor_run(void *p);
#endif
