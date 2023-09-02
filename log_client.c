#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#include "log.h"
#include "logserver.h"

int main(int argc, char **argv)
{

    int cli_fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in cli_addr;
    int addr_len = sizeof(cli_addr);
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_port = htons(1234);
    cli_addr.sin_addr.s_addr = inet_addr("192.168.80.129");
    log_packet log_pkt;
    logEventInit(&log_pkt.log_event);
    logAppendInit(&log_pkt.log_append, "logserver.log", O_CREAT | O_TRUNC | O_RDWR);
    sendto(cli_fd, &log_pkt, sizeof(log_pkt), 0, (struct sockaddr *)&cli_addr, addr_len);
    recvfrom(cli_fd, &log_pkt, sizeof(log_pkt), 0, (struct sockaddr *)&cli_addr, &addr_len);

    printf("file fd is %d\n", log_pkt.log_append.appendfd);

    logtimeUpate(&log_pkt.log_event);
    logattrUpate(&log_pkt.log_event, INFO, __FILE__, pthread_self(), __LINE__, __FUNCTION__);
    log_pkt.log_append.mode = O_WRONLY;
    log_pkt.log_event._log_str_len += sprintf(log_pkt.log_event._log_str + log_pkt.log_event._log_str_len, "write this msg in log file!\n");
    sendto(cli_fd, &log_pkt, sizeof(log_pkt), 0, (struct sockaddr *)&cli_addr, addr_len);

    log_pkt.log_append.mode = -1;
    sendto(cli_fd, &log_pkt, sizeof(log_pkt), 0, (struct sockaddr *)&cli_addr, addr_len);

    close(cli_fd);
    return 0;
}