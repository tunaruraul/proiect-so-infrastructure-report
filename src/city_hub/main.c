#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>

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
    struct sigaction sa;

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

            hub_mon = fork();

            if(hub_mon < 0) {
                perror("fork");
                exit(-1);
            }

            if(hub_mon > 0) {
                int status;
                waitpid(hub_mon, &status, 0);

                if(WIFEXITED(status) && WEXITSTATUS(status) != 0){
                    hub_mon = -1;
                }
            }

            if(hub_mon == 0){
                memset(&sa, 0, sizeof(sa));
                sa.sa_handler = sa_sigaction_term;
                sigaction(SIGTERM, &sa, NULL);

                if(pipe(pipefd) < 0) {
                    perror("pipe");
                    _exit(1);
                }

                monitor_pid = fork();

                if(monitor_pid < 0){
                    perror("fork");
                    _exit(-1);
                }

                if(monitor_pid == 0) {
                    close(pipefd[0]);
                    dup2(pipefd[1], STDOUT_FILENO);
                    close(pipefd[1]);

                    // For testing purposes, usually I would include a make install that copies the binaries to /usr/bin to be available from path, this might break depending where the binary is ran from
                    char *argv[] = {
                        "./bin/monitor_reports",
                        NULL
                    };
                    char *envp[] = {
                        NULL
                    };
                    execve("./bin/monitor_reports", argv, envp); 
                    perror("execl");
                    _exit(127);
                }

                close(pipefd[1]);

                char buf[256];
                ssize_t n;
                int startup_error = 0;
                while((n = read(pipefd[0], buf, sizeof(buf)-1)) > 0){
                    buf[n] = '\0';
                    if((strncmp(buf, "ERROR", 5)) == 0){
                        startup_error = 1;

                        char msg[256];
                        int len = snprintf(msg, sizeof(msg), "monitor: %s", buf);
                        if(len > 0){
                            write(STDOUT_FILENO, msg, len);
                        }

                        break;
                    }

                    char msg[256];
                    int len = snprintf(msg, sizeof(msg), "monitor: %s", buf);
                    if(len > 0){
                        write(STDOUT_FILENO, msg, len);
                    }
                }

                close(pipefd[0]);
                if(startup_error == 0) {
                    char ended[] = "Monitor ended\n";
                    write(STDOUT_FILENO, ended, sizeof(ended)-1);
                }
                _exit(startup_error ? 1 : 0);

            }
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
