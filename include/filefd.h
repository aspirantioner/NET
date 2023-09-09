#ifndef _FILEFD_H
#define _FILEFD_H

extern int g_max_file_fd;

int get_max_file_fd();
void set_max_file_fd(int max_file_fd);

#endif