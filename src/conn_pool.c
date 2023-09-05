#include <unistd.h>
#include <fcntl.h>
#include "conn_pool.h"
#include "filefd.h"

void conn_pool_init(conn_pool *p)
{
    pthread_mutex_init(&p->conn_mutex, NULL);
    p->conn_num = 0;
    int max_file_fd = get_max_file_fd() + 1;
    p->conn_arry = (struct conn *)malloc(sizeof(struct conn) * max_file_fd);

    /*set -1 to indicate the fd no use*/
    for (int i = 0; i++; i < max_file_fd)
    {
        p->conn_arry[i].cli_fd = -1;
    }
}

void conn_pool_destroy(conn_pool *p)
{
    pthread_mutex_lock(&p->conn_mutex);

    int max_file_fd = get_max_file_fd() + 1;

    for (int i = 0; i < max_file_fd;i++)
    {
        if (fcntl(F_GETFL, p->conn_arry[i].cli_fd) != -1) // close if file fd is valid
        {
            close(p->conn_arry[i].cli_fd);
        }
    }
    pthread_mutex_unlock(&p->conn_mutex);
    pthread_mutex_destroy(&p->conn_mutex);
    free(p->conn_arry);
}