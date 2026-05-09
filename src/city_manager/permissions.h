#ifndef PERMISSIONS_H
#define PERMISSIONS_H

#include <sys/stat.h>

void mode_to_string(mode_t mode, char *str);

int check_access(char *path, char *role, int need_read, int need_write);

#endif
