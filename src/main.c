#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commands.h"

void print_usage(char *prog) {
    fprintf(stderr, "Usage: %s --role <manager|inspector> --user <name> <command> [args...]\n", prog);
    fprintf(stderr, "\nCommands:\n");
    fprintf(stderr, "  add <district_id>                    Add a new report\n");
    fprintf(stderr, "  list <district_id>                   List all reports\n");
    fprintf(stderr, "  view <district_id> <report_id>       View a specific report\n");
    fprintf(stderr, "  remove_report <district_id> <id>     Remove a report (manager only)\n");
    fprintf(stderr, "  update_threshold <district_id> <val> Update threshold (manager only)\n");
	fprintf(stderr, "  filter <district_id> [field:op:value ...]      Filter reports by conditions\n");
    fprintf(stderr, "\nExamples:\n");
    fprintf(stderr, "  %s --role inspector --user bob add downtown\n", prog);
    fprintf(stderr, "  %s --role manager --user alice remove downtown 3\n", prog);
}

int main(int argc, char **argv) {
	Context *ctx = malloc(sizeof(Context));
	int cmd_idx = 0;
	int result = 0;

    if (argc < 6) {
        print_usage(argv[0]);
		free(ctx);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--role") == 0 && i + 1 < argc) {
            ctx->role = argv[++i];
        } else if (strcmp(argv[i], "--user") == 0 && i + 1 < argc) {
            ctx->user = argv[++i];
        } else if (argv[i][0] != '-') {
            ctx->command = argv[i];
            cmd_idx = i;
            break;
        }
    }

    if (!ctx->role || !ctx->user || !ctx->command) {
        fprintf(stderr, "Error: missing --role, --user, or command\n");
        print_usage(argv[0]);
		free(ctx);
        return 1;
    }

    if (strcmp(ctx->role, "manager") != 0 && strcmp(ctx->role, "inspector") != 0) {
        fprintf(stderr, "Error: role must be 'manager' or 'inspector'\n");
		free(ctx);
        return 1;
    }

    if (strcmp(ctx->command, "add") == 0) {
        if (cmd_idx + 1 >= argc) {
            fprintf(stderr, "Error: add requires district_id\n");
			free(ctx);
            return 1;
        }
		ctx->district = argv[cmd_idx + 1];
        result = add(ctx);
    }
    else if (strcmp(ctx->command, "list") == 0) {
        if (cmd_idx + 1 >= argc) {
            fprintf(stderr, "Error: list requires district_id\n");
			free(ctx);
            return 1;
        }
		ctx->district = argv[cmd_idx + 1];
        result = list_report(ctx);
    }
    else if (strcmp(ctx->command, "view") == 0) {
        if (cmd_idx + 2 >= argc) {
            fprintf(stderr, "Error: view requires district_id and report_id\n");
			free(ctx);
            return 1;
        }
		ctx->district = argv[cmd_idx + 1];
        result = view_report(ctx, atoi(argv[cmd_idx + 2]));
    }
    else if (strcmp(ctx->command, "remove") == 0) {
        if (cmd_idx + 2 >= argc) {
            fprintf(stderr, "Error: remove_report requires district_id and report_id\n");
			free(ctx);
            return 1;
        }
		ctx->district = argv[cmd_idx + 1];
        result = remove_report(ctx, atoi(argv[cmd_idx + 2]));
    }
    else if (strcmp(ctx->command, "update_threshold") == 0) {
        if (cmd_idx + 2 >= argc) {
            fprintf(stderr, "Error: update_threshold requires district_id and value\n");
			free(ctx);
            return 1;
        }
		ctx->district = argv[cmd_idx + 1];
        result = update_threshold(ctx, atoi(argv[cmd_idx + 2]));
    } else if (strcmp(ctx->command, "filter") == 0) {
    	if (cmd_idx + 1 >= argc) {
        	fprintf(stderr, "Error: filter requires district_id and at least one condition\n");
        	free(ctx);
        	return 1;
    	}
    	ctx->district = argv[cmd_idx + 1];
    	result = filter_reports(ctx, argc, argv, cmd_idx + 2);
	}
    else {
        fprintf(stderr, "Error: unknown command '%s'\n", ctx->command);
        print_usage(argv[0]);
		free(ctx);
        return 1;
    }

	free(ctx);
	return result;	
}
