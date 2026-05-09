#ifndef AUXILIARY_FUNC_H
#define AUXILIARY_FUNC_H

#include "context.h"
#include "commands.h"

int get_next_id(char *district_name);

void log_action(Context *ctx, char *action);
int create_symlink(char *district_name);

int parse_condition(const char *input, char *field, char *op, char *value);
int match_condition(Report *r, const char *field, const char *op, const char *value);

#endif
