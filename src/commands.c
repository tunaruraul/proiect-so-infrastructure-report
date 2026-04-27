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

int list_report(Context *ctx) {
	char path[256];
	Report r;
	int fd;
	struct stat st;
	ssize_t bytes_read;
	char perm[16];
	int count = 0;

	snprintf(path, sizeof(path), "%s/reports.dat", ctx->district);

    if (!check_access(path, ctx->role, 1, 0)) {
        return -1;
    }

	if(stat(path, &st) != 0){
		fprintf(stderr, "No reports found for district %s\n", ctx->district);
		return 0;
	}

	mode_to_string(st.st_mode, perm);

	printf("District: %s\n", ctx->district);
	printf("File: %s\n", path);
	printf("Size: %ld\n", (long)st.st_size);
	printf("Permissions: %s\n", perm);
	printf("\n");

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    printf("%-5s %-20s %-15s %-15s %-12s %-10s %-20s %s\n",
           "ID", "Inspector", "Latitude", "Longitude", "Category", "Severity", "Timestamp", "Description");
    printf("-------------------------------------------------------------------------------------------------------------------------\n");

	while ((bytes_read = read(fd, &r, sizeof(Report))) == sizeof(Report)) {
		char time_str[64];
		struct tm *tm_info = localtime(&r.timestamp);	

		strftime(time_str, sizeof(time_str), "%Y-%m-%d %H-%M-%S", tm_info);
    	printf("%-5d %-20s %-15lf %-15lf %-12s %-10d %-20s %s\n",
				r.id, r.inspector_name, r.latitude, r.longitude, r.category,
				r.sec_level, time_str, r.description);
		count++;
	}

	close(fd);

    if (count == 0) {
        printf("No reports found.\n");
    } else {
        printf("\nTotal reports: %d\n", count);
    }

	return 0;
}

int view_report(Context *ctx, int report_id) {
	char path[256];
	int fd;
	Report r;
	ssize_t bytes_read;
	int found = 0;

	snprintf(path, sizeof(path), "%s/reports.dat", ctx->district);

    if (!check_access(path, ctx->role, 1, 0)) {
        return -1;
    }

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return -1;
    }

	while((bytes_read = read(fd, &r, sizeof(Report))) == sizeof(Report)){
		if(r.id == report_id){
            char time_str[64];
            struct tm *tm_info = localtime(&r.timestamp);
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

            printf("=== Report Details ===\n");
            printf("ID:          %d\n", r.id);
            printf("Inspector:   %s\n", r.inspector_name);
            printf("GPS:         %.6f, %.6f\n", r.latitude, r.longitude);
            printf("Category:    %s\n", r.category);
            printf("Severity:    %d (%s)\n", r.sec_level,
                   r.sec_level == 1 ? "minor" : r.sec_level == 2 ? "moderate" : "critical");
            printf("Timestamp:   %s\n", time_str);
            printf("Description: %s\n", r.description);
            found = 1;
            break;
		}
	}

	close(fd);

	if(!found){
		fprintf(stderr, "Report not found");
		return 1;
	}

	return 0;
}

int update_threshold(Context *ctx, int threshold_value){
	char path[256];
	int fd;
	struct stat st;
	char buf[64];
	char action_buf[256];
	
	if(strcmp(ctx->role, "manager") != 0){
		fprintf(stderr, "Role %s is not manager", ctx->role);
		return -1;
	}

    snprintf(path, sizeof(path), "%s/district.cfg", ctx->district);

    if (!check_access(path, ctx->role, 1, 1)) {
        return -1;
    }

	if(stat(path, &st) != 0){
		perror("stat");
		return -1;
	}

	mode_t mode = st.st_mode & 0777;
	if(mode != 0640) {
		fprintf(stderr, "Error: permissions are %o, expected 640.", mode);
		return -1;
	}

	fd = open(path, O_WRONLY | O_TRUNC);
	if(fd < 0){
		perror("open");
		return -1;
	}

	snprintf(buf, sizeof(buf), "threshold=%d", threshold_value);
	if(write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf)){
		perror("write");
		close(fd);
		return -1;
	}

	close(fd);

    snprintf(action_buf, sizeof(action_buf), "changed threshold to %d", threshold_value);
	log_action(ctx, action_buf);

	return 0;
}

int remove_report(Context *ctx, int report_id){
	char path[256];
	int fd;
	Report r;
	ssize_t bytes_read;
	char action_buf[256];
	int found = 0;
	struct stat st;
	off_t targe_pos = -1;
	off_t pos = 0;

	if(strcmp(ctx->role, "manager") != 0){
		fprintf(stderr, "Role %s is not manager", ctx->role);
		return -1;
	}

	snprintf(path, sizeof(path), "%s/reports.dat", ctx->district);

    if (!check_access(path, ctx->role, 1, 1)) {
        return -1;
    }

    fd = open(path, O_RDWR);
    if (fd < 0) {
        perror("open");
        return -1;
    }

	while((bytes_read = read(fd, &r, sizeof(Report))) == sizeof(Report)){
		if(r.id == report_id){
			targe_pos = pos;
			found = 1;
			break;
		}

		pos+=sizeof(Report);
	}

	if(!found){
		fprintf(stderr, "report %d not found in district %s", r.id, ctx->district);
		close(fd);
		return -1;
	}

    if (fstat(fd, &st) != 0) {
        perror("fstat");
        close(fd);
        return -1;
    }

    off_t file_size = st.st_size;
    off_t remaining = file_size - targe_pos - sizeof(Report);

	if(remaining > 0){
		char *buffer = malloc(remaining);
		if(!buffer){
			perror("malloc");
			close(fd);
			return(-1);
		}

		lseek(fd, targe_pos + sizeof(Report), SEEK_SET);
		if(read(fd, buffer, sizeof(buffer)) != remaining){
			perror("read");
			free(buffer);
			close(fd);
			return -1;
		}

		lseek(fd, targe_pos, SEEK_SET);
		if(write(fd, buffer, sizeof(buffer)) != remaining){
			perror("read");
			free(buffer);
			close(fd);
			return -1;
		}

		free(buffer);
	}

    if (ftruncate(fd, file_size - sizeof(Report)) != 0) {
        perror("ftruncate");
        close(fd);
        return -1;
    }

    snprintf(action_buf, sizeof(action_buf), "remove report id=%d", r.id);
    log_action(ctx, action_buf);

    printf("Report %d removed successfully\n", r.id);
    return 0;

	return 0;
}
