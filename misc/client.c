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

pthread_mutex_t cli_mutex = PTHREAD_MUTEX_INITIALIZER;
static int count = 1;
void *thread_fun(void *p)
{
    int client_sockfd;
    int len;
    struct sockaddr_in address; // 服务器端网络地址结构体
    int result;
    char ch[1024] = {0};
    strcpy(ch, "server test!\n");
    client_sockfd = socket(AF_INET, SOCK_STREAM, 0); // 建立客户端socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("192.168.80.129");
    address.sin_port = htons(1234);
    len = sizeof(address);
    result = connect(client_sockfd, (struct sockaddr *)&address, len);
    char buf[10];
    sprintf(buf, "%d\n", client_sockfd);
    write(STDOUT_FILENO, buf, strlen(buf));
    if (result == -1)
    {
        perror("oops: client2");
        exit(1);
    }
    // 第一次读写
    int str_num = strlen(ch);
    write(client_sockfd, ch, str_num);
    bzero(ch, str_num);
    int ret = read(client_sockfd, ch, str_num);
    if (ret == -1 || ret == 0)
    {
        perror("read error");
    }
    write(STDOUT_FILENO, ch, str_num);
    close(client_sockfd);
    pthread_mutex_lock(&cli_mutex);
    printf("%d\n", count++);
    pthread_mutex_unlock(&cli_mutex);
    return NULL;
}

int main()
{
    pthread_t thread_arry[1000];
    for (int i = 0; i < 1000; i++)
    {
        pthread_create(&thread_arry[i], NULL, thread_fun, NULL);
        pthread_detach(thread_arry[i]);
    }
    pthread_exit(NULL);
    return 0;
}
