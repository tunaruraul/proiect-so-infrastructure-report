#include "permissions.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

void mode_to_string(mode_t mode, char *str){
	str[0] = (mode & S_IRUSR) ? 'r' : '-';
	str[1] = (mode & S_IWUSR) ? 'w' : '-';
	str[2] = (mode & S_IXUSR) ? 'x' : '-';
	str[3] = (mode & S_IRGRP) ? 'r' : '-';
	str[4] = (mode & S_IWGRP) ? 'w' : '-';
	str[5] = (mode & S_IXGRP) ? 'x' : '-';
	str[6] = (mode & S_IROTH) ? 'r' : '-';
	str[7] = (mode & S_IWOTH) ? 'w' : '-';
	str[8] = (mode & S_IXOTH) ? 'x' : '-';
	str[9] = '\0';
}

int check_access(char *path, char *role, int need_read, int need_write){
	struct stat st;

	if(stat(path, &st) != 0){
		perror("stat");
		return 0;
	}

	mode_t mode = st.st_mode;

	if(strcmp(role, "manager") == 0){
		if(need_read && !(mode & S_IRUSR)){
			fprintf(stderr, "Access denied: manager lacks read permission rights for %s", path);
			return 0;
		}
		
		if(need_write && !(mode & S_IWUSR)){
			fprintf(stderr, "Access denied: manager lacks write permission rights for %s", path);
			return 0;
		}
	} else if (strcmp(role, "inspector") == 0) {
		if(need_read && !(mode & S_IRGRP)){
			fprintf(stderr, "Access denied: inspector lacks read permission rights for %s", path);
			return 0;
		}
		
		if(need_write && !(mode & S_IWGRP)){
			fprintf(stderr, "Access denied: inspector lacks write permission rights for %s", path);
			return 0;
		}
	} else {
		fprintf(stderr, "Role not found: %s\n", role);
		return 0;
	}

	return 1;
}
