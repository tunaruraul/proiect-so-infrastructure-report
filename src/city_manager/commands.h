#ifndef COMMANDS_H
#define COMMANDS_H

#include <time.h>
#include "context.h"

typedef struct {
	int id;
	char inspector_name[100];
	double latitude;
	double longitude;
	char category[50];
	int sec_level;
	time_t timestamp;
	char description[256];
} Report;

int init_district(char *district_name);
int add(Context *ctx);
int list_report(Context *ctx);
int view_report(Context *ctx, int report_id);
int update_threshold(Context *ctx, int threshold_value);
int remove_report(Context *ctx, int report_id);
int filter_reports(Context *ctx, int argc, char **argv, int cond_start);
int remove_district(Context *ctx);

#endif
