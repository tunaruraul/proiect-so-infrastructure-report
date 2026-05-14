#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "monitor_pipe.h"

static pid_t monitor_pid = -1;
static pid_t hub_mon = -1;

void sa_sigaction_term(int sig){
    if(monitor_pid > 0){
        kill(monitor_pid, SIGINT);
    }

    _exit(0);
}


int main() {
    char line[256];
    char command[64];
    int pipefd[2];

    while(1){
        char prompt[] = "city manager > ";
        write(STDOUT_FILENO, prompt, sizeof(prompt) - 1);

        if(fgets(line, sizeof(line), stdin) == NULL){
            break;
        }

        sscanf(line, "%63s", command);

        if(strcmp(command, "calculate_scores") == 0){

        }

        if(strcmp(command, "start_monitor") == 0) {
            start_watcher(&hub_mon, pipefd);
        }

        if(strcmp(command, "exit") == 0){
            char exit_hub[] = "Exiting city hub\n";
            write(STDOUT_FILENO, exit_hub, sizeof(exit_hub)-1);
            if(hub_mon > 0){
                kill(hub_mon, SIGTERM);
                waitpid(hub_mon, NULL, 0);
            }

            exit(0);
        }
    }
    
    return 0;
}
