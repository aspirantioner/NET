#include "filefd.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h> 
#include <assert.h>

int g_max_file_fd = 0;

int get_max_file_fd()
{
    if (!g_max_file_fd)
    {
        FILE *fp = popen("ulimit -n", "r");
        char buf[1024];
        fgets(buf, 1023, fp);
        pclose(fp);
        g_max_file_fd = atoi(buf);
    }
    return g_max_file_fd;
}

void set_max_file_fd(int max_file_fd)
{
    assert(max_file_fd > 0);
    struct rlimit rl;
    if(getrlimit(RLIMIT_OFILE, &rl)==-1){
        perror("getrilimit");
    }
    rl.rlim_cur = max_file_fd;
    if(setrlimit(RLIMIT_OFILE,&rl)==-1){
        perror("setrlimit");
    }
    g_max_file_fd = max_file_fd;
}