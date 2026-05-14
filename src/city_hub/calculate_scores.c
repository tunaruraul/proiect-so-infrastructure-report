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

void start_scorers_processes(char **district, int nwords) {
    pid_t pid[64];
    int pipefd[64][2];

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

        }
    }
}
