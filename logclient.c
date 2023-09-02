#include "logclient.h"

void log_client_init(struct log_client *log_client_p, const char *server_ip, in_port_t server_port)
{
    /*set log server info*/
    log_client_p->server_ip = server_ip;
    log_client_p->server_port = server_port;
    log_client_p->log_socket = socket(AF_INET, SOCK_DGRAM, 0);
    log_client_p->log_server_addr.sin_family = AF_INET;
    log_client_p->log_server_addr.sin_port = htons(server_port);
    log_client_p->log_server_addr.sin_addr.s_addr = inet_addr(server_ip);

    /*set scoket buff size*/
    int buff_size = 0;
    socklen_t opt_len = sizeof(int);
    getsockopt(log_client_p->log_socket, SOL_SOCKET, SO_SNDBUF, &buff_size, &opt_len);
    printf("%d\n", buff_size);
    buff_size *= 2;
    setsockopt(log_client_p->log_socket, SOL_SOCKET, SO_SNDBUF, &buff_size, opt_len);

    /*connect log_server*/
    if (connect(log_client_p->log_socket, (struct sockaddr *)&log_client_p->log_server_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("connect");
    };
    
    log_client_p->log_pkt.log_append.appendfd = 0; // judge if open file fd
}

bool log_client_open_file(struct log_client *log_client_p, char *file_name, int open_mode)
{
    logAppendInit(&log_client_p->log_pkt.log_append, file_name, open_mode);
    int addr_len = sizeof(struct sockaddr);
    write(log_client_p->log_socket, &log_client_p->log_pkt, sizeof(struct log_packet));
    read(log_client_p->log_socket, &log_client_p->log_pkt, sizeof(struct log_packet));
    // sendto(log_client_p->log_socket, &log_client_p->log_pkt, sizeof(struct log_packet), 0, (struct sockaddr *)&log_client_p->log_server_addr, addr_len);
    // recvfrom(log_client_p->log_socket, &log_client_p->log_pkt, sizeof(log_packet), 0, (struct sockaddr *)&log_client_p->log_server_addr, &addr_len);
    return log_client_p->log_pkt.log_append.appendfd > 0 ? true : false;
}

void log_client_write(struct log_client *log_client_p)
{
    log_client_p->log_pkt.log_append.mode = O_WRONLY;
    write(log_client_p->log_socket, &log_client_p->log_pkt, sizeof(struct log_packet));
    // sendto(log_client_p->log_socket, &log_client_p->log_pkt, sizeof(struct log_packet), 0, (struct sockaddr *)&log_client_p->log_server_addr, sizeof(struct sockaddr));
}

void log_client_close(struct log_client *log_client_p)
{
    log_client_p->log_pkt.log_append.mode = -1;
    write(log_client_p->log_socket, &log_client_p->log_pkt, sizeof(struct log_packet));
    // sendto(log_client_p->log_socket, &log_client_p->log_pkt, sizeof(struct log_packet), 0, (struct sockaddr *)&log_client_p->log_server_addr, sizeof(struct sockaddr));
}
