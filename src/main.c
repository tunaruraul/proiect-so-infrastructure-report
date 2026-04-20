#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

struct dirent *dir_structure;

int main(int argc, char **argv) {
	DIR *din = opendir("city_infrastructure");
	while(dir_structure = readdir(din)){
		printf("%s\n", dir_structure->d_name);
	}
	return 0;	
}
