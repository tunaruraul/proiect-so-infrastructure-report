#ifndef CONTEXT_H
#define CONTEXT_H

typedef struct {
	char *role;
	char *user;
	char *command;
	char *district;
	int district_id;
} Context;

#endif
