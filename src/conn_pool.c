#include <fcntl.h>
#include "conn_pool.h"
#include "filefd.h"

void conn_pool_init(conn_pool *p)
{
    pthread_spin_init(&p->conn_spin, PTHREAD_PROCESS_PRIVATE);
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
    pthread_spin_lock(&p->conn_spin);

    int max_file_fd = get_max_file_fd() + 1;

    for (int i = 0; i < max_file_fd;i++)
    {
        if (fcntl(F_GETFL, p->conn_arry[i].cli_fd) != -1) // close if file fd is valid
        {
            close(p->conn_arry[i].cli_fd);
        }
    }
    pthread_spin_unlock(&p->conn_spin);
    pthread_spin_destroy(&p->conn_spin);
    free(p->conn_arry);
}