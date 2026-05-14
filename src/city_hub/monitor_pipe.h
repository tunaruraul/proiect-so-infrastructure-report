#ifndef MONITOR_PIPE_H
#define MONITOR_PIPE_H

#include <sys/types.h>

void sa_sigaction_term(int sig);
int parse_pipe_messages(int *pipefd, pid_t *hub_mon);
void start_watcher(int *hub_mon, int *pipefd);

#endif
