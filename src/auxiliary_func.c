#include <stdio.h>
#include "auxiliary_func.h"
#include "permissions.h"
#include "commands.h"
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int get_next_id(char *district_name) {
    char path[512];
    struct stat st;
    int fd;
    int count = 0;

    snprintf(path, sizeof(path), "%s/reports.dat", district_name);

    if (stat(path, &st) != 0) {
        return 1; /* First report gets ID 1 */
    }

    count = st.st_size / sizeof(Report);
    return count + 1;
}

void log_action(Context *ctx, char *action) {
    char path[512];
    int fd;
    char buf[1024];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_str[64];

    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    snprintf(buf, sizeof(buf), "[%s] role=%s user=%s action=%s\n", 
             time_str, ctx->role, ctx->user, action);

    snprintf(path, sizeof(path), "%s/logged_district", ctx->district);

    if (!check_access(path, ctx->role, 0, 1)) {
        return;
    }

    fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd >= 0) {
        write(fd, buf, strlen(buf));
        close(fd);
        chmod(path, 0644);
    }
}

