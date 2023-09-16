#include "logserver.h"
#include "cache_pool.h"
#include "bitmap.h"
#include "filefd.h"
#include <stdio.h>

int main(int argc, char **argv)
{

    struct thread_pool pool;
    thread_pool_init(&pool, 10, 3, 3);

    struct bitmap map;
    int map_len = get_max_file_fd() + 1;
    bitmap_init(&map, map_len);

    struct cache_pool ca;
    cache_pool_init(&ca, sizeof(log_client_set) + sizeof(work_unit), 1000, 100);
    struct log_server log_srv;
    log_server_init(&log_srv, "192.168.80.128", 12345, &pool, &map, &ca);
    log_server_run(&log_srv);
}
