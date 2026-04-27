# This file shows how I built the helper functions for the filter function

For this I am using the free version of Claude

## Initial prompt:
Uploaded header and .c files:
This is the structure of my project so far, I need to implement this filter helper functions:

Use an AI assistant to help you implement two functions:
Describe your record structure and ask the AI to generate a function
        int parse_condition(const char *input, char *field, char *op, char *value);
      which splits a field:operator:value string into its three parts.
Describe the fields and their types and ask the AI to generate a function
        int match_condition(Report *r, const char *field, const char *op, const char *value);
      which returns 1 if the record satisfies the condition and 0 otherwise.
Review both functions line by line. You must be able to explain them fully at your presentation.
Write the filter logic yourself: open reports.dat, read records one by one with read(), parse each command-line condition with parse_condition(), test each record against every condition with match_condition(), and print records where all conditions return 1.
Describe in an ai_[usage.md](http://usage.md) file the tool used, the prompts you gave, what was generated, what you changed and why, and what you learned. This file is a required deliverable and will be discussed at your presentation.

Just give me the pieces of code for helpers and how they are used in the filter function, do not rewrite existing code logic and sections, the match and parse will be contained into auxiliary functions and then the filter will be inside commands.c

## Response:
Looking at your `Report` struct and existing code patterns, here are the two helper functions and the filter command:

**In `auxiliary_func.h`** — add these declarations:

```c
int parse_condition(const char *input, char *field, char *op, char *value);
int match_condition(Report *r, const char *field, const char *op, const char *value);
```

**In `auxiliary_func.c`** — add these implementations:

```c
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
```

The function walks the input string looking for two `:` separators. It uses pointer arithmetic to measure each segment's length, then copies each piece into the caller-supplied buffers. Returning `-1` on any malformed input keeps error handling simple for the caller.

```c
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
```

Each branch first resolves which record field is being tested, then dispatches on the operator string. Numeric fields (`id`, `severity`) support the six relational operators spelled out as two-letter words to avoid shell-quoting headaches with `<` and `>`. String fields support `eq`, `ne`, and `contains`.

---

**In `commands.c`** — add the `filter_reports` function (also add its prototype to `commands.h`):

```c
int filter_reports(Context *ctx, int argc, char **argv, int cond_start) {
    char path[256];
    int fd;
    Report r;
    ssize_t bytes_read;
    int count = 0;

    snprintf(path, sizeof(path), "%s/reports.dat", ctx->district);

    if (!check_access(path, ctx->role, 1, 0))
        return -1;

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    /* parse every condition up-front so bad syntax fails before any I/O */
    int nconds = argc - cond_start;
    char fields[16][64], ops[16][8], values[16][128];

    if (nconds > 16) {
        fprintf(stderr, "Too many conditions (max 16)\n");
        close(fd);
        return -1;
    }

    for (int i = 0; i < nconds; i++) {
        if (parse_condition(argv[cond_start + i], fields[i], ops[i], values[i]) != 0) {
            fprintf(stderr, "Bad condition syntax: '%s'  (expected field:op:value)\n",
                    argv[cond_start + i]);
            close(fd);
            return -1;
        }
    }

    printf("%-5s %-20s %-12s %-10s %s\n",
           "ID", "Inspector", "Category", "Severity", "Description");
    printf("------------------------------------------------------------\n");

    while ((bytes_read = read(fd, &r, sizeof(Report))) == sizeof(Report)) {
        int match = 1;
        for (int i = 0; i < nconds; i++) {
            if (!match_condition(&r, fields[i], ops[i], values[i])) {
                match = 0;
                break;
            }
        }
        if (match) {
            printf("%-5d %-20s %-12s %-10d %s\n",
                   r.id, r.inspector_name, r.category, r.sec_level, r.description);
            count++;
        }
    }

    close(fd);

    if (count == 0)
        printf("No reports matched.\n");
    else
        printf("\nMatched: %d report(s)\n", count);

    return 0;
}
```

The function pre-validates every condition string before opening the data file, so a typo in condition #3 won't leave a half-printed table on screen. Then it reads records sequentially with `read()` — same pattern you use in `list_report` — and only prints a record when every condition returns 1 (short-circuiting on the first mismatch).

---

**Wiring it into `main.c`** — add this branch alongside your other `strcmp(ctx->command, ...)` blocks:

```c
else if (strcmp(ctx->command, "filter") == 0) {
    if (cmd_idx + 1 >= argc) {
        fprintf(stderr, "Error: filter requires district_id and at least one condition\n");
        free(ctx);
        return 1;
    }
    ctx->district = argv[cmd_idx + 1];
    result = filter_reports(ctx, argc, argv, cmd_idx + 2);
}
```

`cmd_idx + 2` is the index of the first condition argument, which is forwarded directly to `filter_reports` so the function can walk `argv` itself without needing a copied array.

---

**Example usage:**

```bash
# All critical reports
./inspector --role inspector --user bob filter downtown severity:eq:3

# Reports by a specific inspector in a category
./inspector --role manager --user alice filter downtown inspector:eq:bob category:eq:flood

# Severity at least 2 with a keyword in the description
./inspector --role inspector --user bob filter downtown severity:ge:2 description:contains:leak
```

## Initial run errors:
```bash
make compile
gcc -Wall -Wextra -g src/*.c -o bin/main
src/auxiliary_func.c: In function ‘match_condition’:
src/auxiliary_func.c:77:20: error: implicit declaration of function ‘atoi’ [-Wimplicit-function-declaration]
   77 |         int cval = atoi(value);
      |                    ^~~~
make: *** [Makefile:6: compile] Error 1
```

## How I solved problems:
stdlib.h was not imported, added import now it is compiling
