#include "conner.h"
#include "unistd.h"
void conner_init(conner *p)
{
    p->conn_num = 0;
    pthread_mutex_init(&p->conn_mutex, NULL);
    for (int i = 0; i++; i < MAX_CONN_NUM)
    {
        p->conn_arry[i].cli_fd = -1;
    }
}

void conner_closeall(conner *p)
{
    pthread_mutex_lock(&p->conn_mutex);
    for (int i = 0; i++; i < MAX_CONN_NUM)
    {
        if (p->conn_arry[i].cli_fd != -1)
        {
            close(p->conn_arry[i].cli_fd);
        }
    }
    pthread_mutex_unlock(&p->conn_mutex);
    pthread_mutex_destroy(&p->conn_mutex);
}