#include "commands.h"
#include "auxiliary_func.h"
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

    if (stat(path, &st) != 0) {
		fd = open(path, O_CREAT | O_RDWR, 0664);
        mode_t mode = st.st_mode & 0777;
        if (mode != 0664) { // Adding a 0 at the beginning of an integer makes the digits after be percieved in octal
            chmod(path, 0664);
        }
    }

	if(!(check_access(path, ctx->role, 1, 1))) return -1;

	memset(&r, 0, sizeof(Report));

	r.id = get_next_id(ctx->district);
	strncpy(r.inspector_name, ctx->user, sizeof(r.inspector_name));

    printf("Enter latitude: ");
    if (scanf("%lf", &r.latitude) != 1) {
        fprintf(stderr, "Invalid latitude\n");
        return -1;
    }

    printf("Enter longitude: ");
    if (scanf("%lf", &r.longitude) != 1) {
        fprintf(stderr, "Invalid longitude\n");
        return -1;
    }

    printf("Enter category: ");
    scanf("%49s", r.category);

    printf("Enter severity: ");
    if (scanf("%d", &r.sec_level) != 1 || r.sec_level < 1 || r.sec_level > 3) {
        fprintf(stderr, "Invalid severity\n");
        return -1;
    }

	r.timestamp = time(NULL);

	printf("Enter description: ");
	if(scanf("%255s", r.description) != 1){
		fprintf(stderr, "Invalid description");
		return -1;
	}

    fd = open(path, O_WRONLY | O_APPEND);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    if (write(fd, &r, sizeof(Report)) != sizeof(Report)) {
        perror("write");
        close(fd);
        return -1;
    }

    close(fd);

    snprintf(action_buf, sizeof(action_buf), "add report id=%d", r.id);
    log_action(ctx, action_buf);

    printf("Report added successfully with ID %d\n", r.id);

	return 0;
}
