#ifndef _ACCEPTOR_H
#define _ACCEPTOR_H

#include "log.h"
#include "logclient.h"
#include "lio_thread.h"
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

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
*/
void acceptor_init(struct acceptor *ac, int ip_type, int socket_type, const char *server_ip, in_port_t server_port, unsigned int listen_queue_size);

/*
p:server sys pointer
*/
void *acceptor_run(void *p);

/*destroy acceptor*/
void acceptor_destroy(struct acceptor *ac_p);
#endif
