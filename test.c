#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "acceptor.h"
#include "httpparser.h"
#include <string.h>
#include <arpa/inet.h>

void print(void *elem)
{
    printf("%d\n", *(int *)elem);
    return;
}
int main()
{
    struct acceptor ac;
    acceptor_init(&ac, AF_INET, SOCK_STREAM, "192.168.31.87", 4321, 10, NULL);
    while (true)
    {
        int cli_fd = accept(ac.listen_socket, (struct sockaddr *)&ac.cli_addr, &ac.len);
        char buf[10 * 1024];
        if (cli_fd > 0)
        {
            printf("connect cli is %s:%hu\n", inet_ntoa(ac.cli_addr.sin_addr), ntohs(ac.cli_addr.sin_port));
            while (true)
            {
                int ret = read(cli_fd, buf, 10 * 1024);
                if (ret == 0)
                {
                    close(cli_fd);
                    write(STDOUT_FILENO, "cli closed!\n", 13);
                    break;
                }
                else if (ret == -1)
                {
                    close(cli_fd);
                    write(STDOUT_FILENO, "cli fd bad!\n", 13);
                    perror("read");
                    break;
                }
                else
                {
                    int len = strlen(buf);
                    write(STDOUT_FILENO, buf, len);
                    struct http_request_head *req_head = (struct http_request_head *)malloc(sizeof(struct http_request_head));
                    httpHeadextract(buf, req_head);
                    struct http_response res;
                    setResHead(&res.res_head, req_head->version, 200);
                    setResConnection(&res, "keep-alive");
                    setResFile(&res, &req_head->file[1]);
                    sendResPacket(&res, cli_fd);
                    bzero(buf, len);
                }
            }
        }
    }
}
