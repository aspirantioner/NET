#include "filefd.h"
#include "stdio.h"
#include "stdlib.h"

int get_max_file_fd()
{
    FILE *fp = popen("ulimit -n", "r");
    char buf[1024];
    fgets(buf, 1023, fp);
    pclose(fp);
    return atoi(buf);
}