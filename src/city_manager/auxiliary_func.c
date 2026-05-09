#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
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

int create_symlink(char *district_name) {
    char linkname[512];
    char target[512];
    struct stat st;

    snprintf(linkname, sizeof(linkname), "active_reports-%s", district_name);
    snprintf(target, sizeof(target), "%s/reports.dat", district_name);

    lstat(linkname, &st);
    if (S_ISLNK(st.st_mode)) {
        unlink(linkname);
    }

    if (symlink(target, linkname) != 0) {
        perror("symlink");
        return -1;
    }
    return 0;
}

int parse_condition(const char *input, char *field, char *op, char *value) {
    const char *first_colon = strchr(input, ':');
    if (!first_colon) return -1;

    const char *second_colon = strchr(first_colon + 1, ':');
    if (!second_colon) return -1;

    size_t field_len = first_colon - input;
    size_t op_len    = second_colon - (first_colon + 1);

    if (field_len == 0 || field_len >= 64) return -1;
    if (op_len == 0    || op_len >= 8)     return -1;
    if (strlen(second_colon + 1) == 0)     return -1;

    strncpy(field, input,           field_len); field[field_len] = '\0';
    strncpy(op,    first_colon + 1, op_len);    op[op_len]       = '\0';
    strncpy(value, second_colon + 1, 128 - 1);  value[127]       = '\0';

    return 0;
}

int match_condition(Report *r, const char *field, const char *op, const char *value) {
    /* ── numeric comparisons ── */
    if (strcmp(field, "severity") == 0 || strcmp(field, "id") == 0) {
        int rval = (strcmp(field, "id") == 0) ? r->id : r->sec_level;
        int cval = atoi(value);

        if (strcmp(op, "eq") == 0) return rval == cval;
        if (strcmp(op, "ne") == 0) return rval != cval;
        if (strcmp(op, "lt") == 0) return rval <  cval;
        if (strcmp(op, "gt") == 0) return rval >  cval;
        if (strcmp(op, "le") == 0) return rval <= cval;
        if (strcmp(op, "ge") == 0) return rval >= cval;
        fprintf(stderr, "Unknown operator for numeric field: %s\n", op);
        return 0;
    }

    /* ── string comparisons ── */
    const char *sval = NULL;
    if      (strcmp(field, "inspector") == 0)  sval = r->inspector_name;
    else if (strcmp(field, "category")  == 0)  sval = r->category;
    else if (strcmp(field, "description") == 0) sval = r->description;
    else {
        fprintf(stderr, "Unknown field: %s\n", field);
        return 0;
    }

    if (strcmp(op, "eq") == 0) return strcmp(sval, value) == 0;
    if (strcmp(op, "ne") == 0) return strcmp(sval, value) != 0;
    if (strcmp(op, "contains") == 0) return strstr(sval, value) != NULL;

    fprintf(stderr, "Unknown operator for string field: %s\n", op);
    return 0;
}
