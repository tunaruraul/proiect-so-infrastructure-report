#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

int monitor_pid = -1;

void sa_sigaction_term(int sig){
    if(monitor_pid > 0){
        kill(monitor_pid, SIGINT);
    }
}

int main() {
    char line[256];
    char command[64];
    int monitor_pid = -1;
    int hub_mon = -1;
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = sa_sigaction_term;

    sigset_t mask;
    sigemptyset(&mask);

    sigaction(SIGTERM, &sa, NULL);

    while(1){
        printf("city hub > ");

        if(fgets(line, sizeof(line), stdin) == NULL){
            break;
        }

        sscanf(line, "%63s", command);

        if(strcmp(command, "calculate_scores") == 0){

        }

        if(strcmp(command, "start_monitor") == 0) {
            pid_t hub_mon = fork();
            if(hub_mon < 0) {
                perror("fork");
                exit(-1);
            }

            if(hub_mon == 0){
                pid_t monitor_pid = fork();

                if(monitor_pid < 0){
                    perror("fork");
                    exit(-1);
                }

                if(monitor_pid == 0) {
                    // For testing purposes, usually I would include a make install that copies the binaries to /usr/bin to be available from path, this might break depending where the binary is ran from
                    execl("./bin/monitor_reports", "./bin/monitor_reports", NULL); 
                    perror("execl");
                    exit(127);
                }
            }

            waitpid(monitor_pid, NULL, 0);
        }

        if(strcmp(command, "exit") == 0){
            printf("Exiting city hub\n");
            kill(hub_mon, SIGTERM);

            exit(0);
        }
    }
    
    return 0;
}
