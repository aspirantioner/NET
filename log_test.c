#include "logserver.h"
#include "bitmap.h"
#include "filefd.h"
#include <stdio.h>

int main(int argc, char **argv)
{

    struct thread_pool pool;
    thread_pool_init(&pool, 10, 3, 3);

    struct bitmap map;
    int map_len = get_max_file_fd();
    bitmap_init(&map, map_len);

    struct log_server log_srv;
    log_server_init(&log_srv, "192.168.80.128", 12345, &pool, &map);
    log_server_run(&log_srv);
}
