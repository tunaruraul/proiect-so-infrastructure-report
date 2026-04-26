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

int add(Context *ctx){
	char path[256];
	int fd;
	Report r;
	struct stat st;
	char action_buf[256];

	if(init_district(ctx->district) != 0) return -1;

	snprintf(path, sizeof(path), "%s/reports.dat", ctx->district);

	if(!(check_access(path, ctx->role, 1, 1))) return -1;

    if (stat(path, &st) == 0) {
        mode_t mode = st.st_mode & 0777;
        if (mode != 0664) { // Adding a 0 at the beginning of an integer makes the digits after be percieved in octal
            chmod(path, 0664);
        }
    }

	memset(&r, 0, sizeof(Report));

	return 0;
}
