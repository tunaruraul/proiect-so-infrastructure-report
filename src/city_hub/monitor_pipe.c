#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "monitor_pipe.h"

void check_monitor_status(pid_t *monitor_pid){
    if(*monitor_pid > 0) {
        int status;

        if(waitpid(*monitor_pid, &status, WNOHANG) == *monitor_pid){
            if(WIFEXITED(status) && WEXITSTATUS(status) != 0){
                *monitor_pid = -1;
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
        print_monitor_message(buf, 0);
        *should_stop = 1;
        return 1;
    }

    if(strncmp(buf, "ENDED", 5) == 0) {
        print_monitor_message(buf, 0);
        *should_stop = 1;
        return 0;
    }

    if(strncmp(buf, "INFO", 4) == 0) {
        print_monitor_message(buf, 1);
        *should_stop = 0;
        return 0;
    }

    *should_stop = 0;
    return 0;
}

int parse_pipe_messages(int *pipefd, pid_t *monitor_pid){
    ssize_t n;
    char buf[256];
    int startup_error = 0;
    int should_stop = 0;
    while((n = read(pipefd[0], buf, sizeof(buf)-1)) > 0){
        check_monitor_status(monitor_pid);

        buf[n] = '\0';
        
        startup_error = handle_monitor_message(buf, &should_stop);

        if(should_stop){
            break;
        }
    }

    return startup_error;
}

void setup_term_handler(){
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sa_sigaction_term;
    sigaction(SIGTERM, &sa, NULL);
}

pid_t start_monitor_child_process(int *pipefd) {
    pid_t pid = fork();

    if(pid < 0){
        perror("fork");
        _exit(-1);
    }

    if(pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        // For testing purposes I'm using the relative path to the compiled binary,
        // usually I would include a make install that copies the binaries to /usr/bin 
        // so that they are available from path.
        // This might break depending where the binary is ran from
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


    return pid;
}

int start_hub_mon(int *pipefd, int statusfd){
    setup_term_handler();

    if(pipe(pipefd) < 0) {
        perror("pipe");
        _exit(1);
    }

    pid_t monitor_pid = start_monitor_child_process(pipefd);

    if(monitor_pid < 0){
        close(pipefd[0]);
        close(pipefd[1]);
        return 1;
    }

    close(pipefd[1]);

    int startup_error = parse_pipe_messages(pipefd, &monitor_pid);

    write(statusfd, startup_error ? "E" : "S", 1);
    close(statusfd);

    close(pipefd[0]);

    if(!startup_error) {
        char ended[] = "\nMonitor ended\ncity manager > ";
        write(STDOUT_FILENO, ended, sizeof(ended)-1);
    }

    return startup_error ? 1 : 0;
}

void start_watcher(pid_t *hub_mon, int *pipefd){
    if(*hub_mon > 0) {
        return;
    }
    int statuspipe[2];

    if(pipe(statuspipe) < 0){
        perror("pipe");
        return;
    }

    *hub_mon = fork();

    if(*hub_mon < 0) {
        perror("fork");
        exit(-1);
    }

    if(*hub_mon == 0) {
        close(statuspipe[0]);
        _exit(start_hub_mon(pipefd, statuspipe[1]));
    }

    close(statuspipe[1]);
    
    char status;

    if((read(statuspipe[0], &status, 1) == 1) && status == 'E') {
        waitpid(*hub_mon, NULL, 0);
        *hub_mon = -1;
    }

    close(statuspipe[0]);
}
