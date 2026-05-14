#ifndef MONITOR_PIPE_H
#define MONITOR_PIPE_H

#include <sys/types.h>

int parse_pipe_messages(int *pipefd, pid_t *hub_mon);

#endif
