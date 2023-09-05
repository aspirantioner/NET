#ifndef _LOG_CLIENT_H
#define _LOG_CLIENT_H

#include "logserver.h"
#include "log.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct log_client
{
    const char *server_ip;
    in_port_t server_port;
    struct sockaddr_in log_server_addr;
    int log_socket;
    struct log_packet log_pkt;
} log_client;

void log_client_init(struct log_client *log_client_p, const char *server_ip, in_port_t server_port);
bool log_client_open_file(struct log_client *log_client_p, char *file_name, int open_mode);
void log_client_write(struct log_client *log_client_p);
void log_client_close(struct log_client *log_client_p);

#endif