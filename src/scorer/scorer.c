#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../city_manager/commands.h"

typedef struct {
    char inspector_name[100];
    int score;
} InspectorScore;

int main(int argc, char **argv) {
    char *district = argv[1];
    char report_path[128];
    Report report;
    InspectorScore inspector[64];
    int nscores = 0;
    snprintf(report_path, sizeof(report_path), "%s/reports.dat", district);
    FILE *fin = fopen(report_path, "rb");
    if(!fin){
        perror("fopen");
        exit(-1);
    }

    while(fread(&report, sizeof(Report), 1, fin) == 1){
        int found = -1;
        for(int i = 0; i < nscores; i++) {
            if(strcmp(inspector[i].inspector_name, report.inspector_name) == 0){
                found = i;
                break;
            }
        }

        if(found >= 0) {
            inspector[found].score += report.sec_level;
        } else {
            if(nscores >= 64) {
                fprintf(stdout, "Too many inspectors");
                break;
            }

            strncpy(inspector[nscores].inspector_name, report.inspector_name, 99);
            inspector[nscores].inspector_name[99] = '\0';
            inspector[nscores].score = report.sec_level;
            nscores++;
        }
    }

    for (int i = 0; i < nscores; i++) {
        printf("%s %d\n", inspector[i].inspector_name, inspector[i].score);
    }

    fclose(fin);

    return 0;
}
