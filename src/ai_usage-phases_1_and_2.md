## Phase 1

For this I am using the free version of Claude

### Initial prompt:
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

### Response:
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

### Initial run errors:
```bash
make compile
gcc -Wall -Wextra -g src/*.c -o bin/main
src/auxiliary_func.c: In function ‘match_condition’:
src/auxiliary_func.c:77:20: error: implicit declaration of function ‘atoi’ [-Wimplicit-function-declaration]
   77 |         int cval = atoi(value);
      |                    ^~~~
make: *** [Makefile:6: compile] Error 1
```

### How I solved problems:
stdlib.h was not imported, added import now it is compiling

## Phase 2

### Initial prompt:
1. Add a new command to the existing city_manager program, remove_district <district_id> that deletes the entire district directory and all its contents, plus the corresponding active_reports-* symlink. For deleting the directory, the program creates a child process, in which it calls the external command rm -rf <district_directory>. Be very careful when you test this so that you don't pass wrong arguments to rm thus deleting useful files or folders on your computer! The command is manager role only.

I was using execlp for this, but from my understanding that does a child process but the parent process would still run after that, I should do something similar to async await, should I build the function in the helper file with execlp and then in main wait for it to end then return the execution value?

Give me as little code as possible, I want more to be able to think for myself and will send snippets of code so you can work more as a code-reviewer/debugger if something goes wrong

### Response:
Yes: use fork() + execlp() in the child, and waitpid() in the parent. execlp() does not create a child process by itself; it replaces the current process image. So if you call execlp() directly in your main manager process, your program becomes rm and does not continue normally.

### Initial clarifications:
Everything after fork and container in if(pid ==0) is running for the child process
Helped me realize I should build safety measures for running the rm command, so that it throws an error if it tries to delete folders such as: "/" "." ".."

### help for points 2 and 3:
2. Write a new program called monitor_reports that works as follows:
at startup it creates or overwrites a hidden text file called .monitor_pid where it stores its main process ID. The file is situated at the same level of the directory tree as the district directories.
when it ends, it deletes the above file
the program only ends when it receives SIGINT, and writes a message on the standard output when it does
the program responds to SIGUSR1 signals (which informs it that a new report has been added) by writing a message on the standard output
3. Modify the existing city_manager program so that, when adding a new report, it notifies the monitor program of the event (by SIGUSR1), and writes a message acknowledging the notification in the corresponding log file for the report. If it doesn't find a PID of the monitor in the .monitor_pid file, or if any other error occurs (e.g., sending the signal did not work), the message in the log file must explicitly show that the monitor could not be informed of the event.

Can you help me make sense of this? I need to create a forever running process with while(1) for the monitor that is like a watcher for new reports? the city manager has variable pid for each run, as it does not create something that runs in the background, I have to create a file, write to the file and sent a signal to the main monitor process, an estimate, how many lines of code would this be and what is the complexity of this task?

### Response summarized:
Yes, you understood the big picture.

You are creating two separate programs:

monitor_reports
  runs continuously
  saves its PID in .monitor_pid
  waits for signals
  exits only on SIGINT

city_manager
  runs normally per command
  when add_report happens:
    reads .monitor_pid
    sends SIGUSR1 to that PID
    logs whether notification succeeded

The monitor is not “watching” the directory automatically. It is only waiting for city_manager to notify it.

it tried to get me to use a volatile thing variable, which felt a bit over the top, and I did not want to spend time to research it, I just wanted to do a simple while(1), on of my questions were more if I am running a while(1) how do I make it stop when a signal is sent to the program, that is how I discovered pause()

The manual page for pause() redirected me to use sigsuspend() as pause is obsolete


Initial generated code:
```c
volatile sig_atomic_t keep_running = 1;

void handle_sigint(int sig) {
    write(STDOUT_FILENO, "Monitor stopping\n", 17);
    keep_running = 0;
}

void handle_sigusr1(int sig) {
    write(STDOUT_FILENO, "New report added\n", 17);
}

int main(void) {
    FILE *f = fopen(".monitor_pid", "w");
    fprintf(f, "%d\n", getpid());
    fclose(f);

    signal(SIGINT, handle_sigint);
    signal(SIGUSR1, handle_sigusr1);

    while (keep_running) {
        pause();
    }

    unlink(".monitor_pid");
    return 0;
}
```

### Next question:
```c
    while(1) {
        sigsuspend(SIG_BLOCK);
    }
```
Can I do something like this? I see that pause is obsolete and the man pages recommend to use sigsuspend instead, but I do not fully understand how the mask works

Nota Bene: Still can't really wrap my head around how mask, I mean I have an idea, but don't really fully understand it

### Response:
Not like this:
```c
sigsuspend(SIG_BLOCK);
```

SIG_BLOCK is not a signal mask. It is a constant used with sigprocmask().

sigsuspend() expects a pointer to a sigset_t:
```c
sigsuspend(&mask);
```

For your assignment, the simplest correct sigsuspend() version is:
```c
sigset_t mask;

sigemptyset(&mask);

while (1) {
    sigsuspend(&mask);
}
```

Meaning:
```
Use an empty mask while sleeping.
So no signals are blocked while waiting.
Wake up when a signal arrives.
```

### Last question help request:
3. Modify the existing city_manager program so that, when adding a new report, it notifies the monitor program of the event (by SIGUSR1), and writes a message acknowledging the notification in the corresponding log file for the report. If it doesn't find a PID of the monitor in the .monitor_pid file, or if any other error occurs (e.g., sending the signal did not work), the message in the log file must explicitly show that the monitor could not be informed of the event. 

For this, should I create another function for getting the pid of the function and writing a log for the event in which it can not read or send a signal then call it into the add function?

### Response:
Yes. That is the cleanest approach.

Create a helper function like:
```c
int notify_monitor(void);
```

It should:
```
1. open .monitor_pid
2. read PID from it
3. send SIGUSR1 using kill()
4. return success or failure
```

So the responsibility is split nicely:

```
add_report()
  creates report
  writes normal report log
  calls notify_monitor()
  writes success/failure notification log

notify_monitor()
  only handles reading .monitor_pid and sending SIGUSR1
```

Use kill() like this:

```
kill(monitor_pid, SIGUSR1);
```

Despite the name, kill() just sends a signal.

Also include:
```
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
```

The log message should be written no matter what:
```
success:
Monitor was informed about the new report.

failure:
Monitor could not be informed about the new report.
```
