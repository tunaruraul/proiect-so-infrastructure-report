#ifndef AUXILIARY_FUNC_H
#define AUXILIARY_FUNC_H

#include "context.h"
#include "commands.h"

static int get_next_id(const char *district_name);

void log_action(Context *ctx, const char *action);

#endif
