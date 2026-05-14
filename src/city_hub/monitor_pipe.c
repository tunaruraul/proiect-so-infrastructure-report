#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "monitor_pipe.h"

void check_hub_mon_status(pid_t *hub_mon){
    if(*hub_mon > 0) {
        int status;

        if(waitpid(*hub_mon, &status, WNOHANG) == *hub_mon){
            if(WIFEXITED(status) && WEXITSTATUS(status) != 0){
                *hub_mon = -1;
            }
        }
    }
}

void print_monitor_message(char *buf, int show_prompt) {
    char msg[256];
    int len = snprintf(
            msg,
            sizeof(msg),
            show_prompt ? "\nmonitor: %s\ncity manager > " : "\nmonitor: %s\n",
            buf);

    if(len > 0) {
        write(STDOUT_FILENO, msg, len);
    }
}

int handle_monitor_message(char *buf, int *should_stop) {
    if(strncmp(buf, "ERROR", 5) == 0) {
        print_monitor_message(buf, 1);
        *should_stop = 1;
        return 1;
    }

    if(strncmp(buf, "ENDED", 5) == 0) {
        print_monitor_message(buf, 0);
        *should_stop = 1;
        return 1;
    }

    if(strncmp(buf, "INFO", 4) == 0) {
        print_monitor_message(buf, 1);
        *should_stop = 0;
        return 0;
    }

    *should_stop = 0;
    return 0;
}

int parse_pipe_messages(int *pipefd, pid_t *hub_mon){
    ssize_t n;
    char buf[256];
    int startup_error = 0;
    int should_stop = 0;
    while((n = read(pipefd[0], buf, sizeof(buf)-1)) > 0){
        check_hub_mon_status(hub_mon);

        buf[n] = '\0';
        
        startup_error = handle_monitor_message(buf, &should_stop);

        if(should_stop){
            break;
        }
    }

    return startup_error;
}
