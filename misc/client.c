// 客户端
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
#include <netinet/tcp.h>

pthread_mutex_t cli_mutex = PTHREAD_MUTEX_INITIALIZER;
static int count = 1;
void *thread_fun(void *p)
{
    int client_sockfd;
    int len;
    struct sockaddr_in address; // 服务器端网络地址结构体
    int result;
    char ch[1024] = {0};
    strcpy(ch, "server test1!\n");
    client_sockfd = socket(AF_INET, SOCK_STREAM, 0); // 建立客户端socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("192.168.80.129");
    address.sin_port = htons(1234);
    len = sizeof(address);
    result = connect(client_sockfd, (struct sockaddr *)&address, len);

    if (result == -1)
    {
        perror("oops: client2");
        exit(1);
    }
    int flag = 1;
    if (setsockopt(client_sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)) == -1)
    {
        perror("delay error");
    };
    // 第一次读写
    int str_num = strlen(ch);
    write(client_sockfd, ch, str_num);
    strcpy(ch, "server test2!\n");
    write(client_sockfd, ch, str_num);
    bzero(ch, 2 * str_num);
    int ret = read(client_sockfd, ch, 1023);
    if (ret == -1 || ret == 0)
    {
        perror("read error");
    }
    write(STDOUT_FILENO, ch, 2 * str_num);
    close(client_sockfd);
    pthread_mutex_lock(&cli_mutex);
    printf("%d\n", count++);
    pthread_mutex_unlock(&cli_mutex);
    return NULL;
}

int main()
{
    pthread_t thread_arry[2000];
    for (int i = 0; i < 2000; i++)
    {
        pthread_create(&thread_arry[i], NULL, thread_fun, NULL);
        pthread_detach(thread_arry[i]);
    }
    pthread_exit(NULL);
    return 0;
}
