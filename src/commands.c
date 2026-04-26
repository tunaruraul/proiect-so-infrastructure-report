#include "commands.h"
#include "context.h"
#include "permissions.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>

typedef struct {
	int id;
	char inspector_name[100];
	int latitude;
	int longitude;
	char category[50];
	int sec_level;
	time_t timestamp;
	char description[256];
} Report;

void log_action(Context *ctx, const char *action) {
    char path[512];
    int fd;
    char buf[1024];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_str[64];

    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    snprintf(buf, sizeof(buf), "[%s] role=%s user=%s action=%s\n", 
             time_str, role, user, action);

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

int init_district (char *district_name){
	struct stat st;

	if(stat(district_name, &st) !=0){
		if(mkdir(district_name, 0750) != 0) return -1;
		chmod(district_name, 0750);
	} else if (!S_ISDIR(st.st_mode)) {
		printf("File %s exists, but it is not a directory", district_name);
		return -1;
	}

	int fd;
	char path[512];

	snprintf(path, sizeof(path), "%s/district.cfg", district_name);
	if(stat(path, &st) != 0){
		fd = open(path, O_CREAT | O_RDWR, 0640);
		if(fd >= 0){
			chmod(path, 0640);
			close(fd);
		}
	}

	snprintf(path, sizeof(path), "%s/logged_district", district_name);
	if(stat(path, &st) != 0){
		fd = open(path, O_CREAT | O_RDWR, 0644);
		if(fd >= 0){
			chmod(path, 0644);
			close(fd);
		}
	}

	return 0;
}

void add(Context *ctx){
}
