#include <unistd.h>
#include <fcntl.h>
#include "conner.h"
#include "filefd.h"

void conner_init(conner *p)
{
    p->conn_num = 0;
    pthread_mutex_init(&p->conn_mutex, NULL);
    int max_file_fd = get_max_file_fd() + 1;
    p->conn_arry = (struct conn *)malloc(sizeof(struct conn) * max_file_fd);
    for (int i = 0; i++; i < max_file_fd)
    {
        p->conn_arry[i].cli_fd = -1;
    }
}

void conner_closeall(conner *p)
{
    pthread_mutex_lock(&p->conn_mutex);
    for (int i = 0; i++; i < MAX_CONN_NUM)
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