#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "commands.h"
#include "permissions.h"
#include "context.h"
#include <time.h>

int main(int argc, char **argv) {
	struct stat st;
	Context *ctx = malloc(sizeof(Context));

	ctx->district = "dacia";
	ctx->role = "manager";
	ctx->user = "manager";
	ctx->command = "add";
	ctx->district_id = 0;
	
	stat("elisabetin/district.cfg", &st);
	mode_t mode = st.st_mode & 0777;
	printf("%o", mode);
	init_district("elisabetin");	
	check_access("elisabetin/district.cfg", "inspector", 0, 1);
	add(ctx);
	return 0;	
}
