#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>

void sa_sigaction_int(int sig) {
    char msg[] = "ENDED INTERRUPT SIGNAL RECEIVED\n";
    write(STDOUT_FILENO, msg, sizeof(msg)-1);

    unlink(".monitor_pid");

    _exit(0);
}

void sa_sigaction_sigusr(int sig) {
    char msg[] ="INFO NEW REPORT ADDED\n";

    write(STDOUT_FILENO, msg, sizeof(msg)-1);
}

int main() {
    int fd;
    struct stat s;

    if(stat(".monitor_pid", &s) == 0){
        char msg[128];

        int fd = open(".monitor_pid", O_RDONLY);
        char pidbuf[32];

        ssize_t n = -1;

        if(fd > 0){
            n = read(fd, pidbuf, sizeof(pidbuf)-1);
            close(fd);
        }

        if(n > 0) {
            pidbuf[n] = '\0';
        } else {
            strcpy(pidbuf, "unknown");
        }

        int len = snprintf(msg, sizeof(msg), "ERROR MONITOR ALREADY RUNNING WITH PID: %s\n", pidbuf);

        write(STDOUT_FILENO, msg, len);

        _exit(1);
    }

    fd = open(".monitor_pid", O_CREAT | O_RDWR | O_APPEND, 0644);

    if(fd < 0) {
        perror("open");
        return -1;
    }

    pid_t pid = getpid();
    char pid_string[32];

    snprintf(pid_string, sizeof(pid_string), "%d", pid);

    write(fd, pid_string, strlen(pid_string));

    close(fd);

    struct sigaction sig_int;
    struct sigaction sig_usr;

    memset(&sig_int, 0, sizeof(sig_int));
    memset(&sig_usr, 0, sizeof(sig_usr));

    sig_int.sa_handler = sa_sigaction_int;
    sig_usr.sa_handler = sa_sigaction_sigusr;

    sigset_t mask;
    sigemptyset(&mask);

    sigaction(SIGINT, &sig_int, NULL);
    sigaction(SIGUSR1, &sig_usr, NULL);

    while(1) {
        sigsuspend(&mask);
    }

    return 0;
}
