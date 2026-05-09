#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

void sa_sigaction_int(int signal_id) {
    char msg[] = "Interrupt signal action received\n";
    write(STDOUT_FILENO, msg, sizeof(msg));

    unlink(".monitor_pid");

    exit(0);
}

void sa_sigaction_sigusr(int signal_id) {
    char msg[] = "New report added\n";

    write(STDOUT_FILENO, msg, sizeof(msg));
}

int main() {
    int fd;

    fd = open(".monitor_pid", O_CREAT | O_RDWR | O_APPEND, 0644);

    if(fd < 0) {
        perror("open");
        return -1;
    }

    pid_t pid = getppid();
    char pid_string[32];

    snprintf(pid_string, sizeof(pid_string), "%d\n", pid);

    write(fd, pid_string, sizeof(pid));

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
