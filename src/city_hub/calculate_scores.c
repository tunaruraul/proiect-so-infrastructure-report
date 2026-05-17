#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "calculate_scores.h"

char **collet_arguments(char *line, int *nwords){

    char *word;
    char **district = malloc(64 * sizeof(char *));
    *nwords = 0;
    for(word = strtok(line, " \t\n"); word; word = strtok(NULL, " \t\n")){
        if(strcmp(word, "calculate_scores") == 0){
            continue;
        }
        district[(*nwords)++] = word; 
    }

    return district;
}

void print_scores(char *buf, char *district) {
    char msg[256];
    int len = snprintf(msg, sizeof(msg), "=== SCORE REPORT %s ===\n%s", district, buf);

    if(len > 0) {
        write(STDOUT_FILENO, msg, len);
    }
}

void start_scorers_processes(char *line) {
    pid_t pid[64];
    int pipefd[64][2];
    char buf[128];
    int nwords = 0;
    char **district = collet_arguments(line, &nwords);
    if(district == NULL){
        perror("malloc");
        return;
    }

    for (int i = 0; i < nwords; i++) {
        if(pipe(pipefd[i]) < 0){
            perror("pipe");
            continue;
        }

        pid[i] = fork();

        if(pid[i] < 0){
            perror("fork");
            close(pipefd[i][0]);
            close(pipefd[i][1]);
            continue;
        }

        if(pid[i] == 0){
            close(pipefd[i][0]);
            dup2(pipefd[i][1], STDOUT_FILENO);
            close(pipefd[i][1]);

            char *argv[] = {
                "./bin/scorer",
                district[i],
                NULL
            };

            execve("./bin/scorer", argv, NULL);

            perror("execve");
            _exit(127);
        }

        close(pipefd[i][1]);

        ssize_t n;
        while((n = read(pipefd[i][0], buf, sizeof(buf)-1)) > 0){
            buf[n] = '\0';
            print_scores(buf, district[i]); 
        }

        close(pipefd[i][0]);
        waitpid(pid[i], NULL, 0);
    }

    free(district);
}
