#ifndef ACTIVITIES_H
#define ACTIVITIES_H

#include <sys/types.h>

#define PROCESS_RUNNING 0
#define PROCESS_STOPPED 1

int activities_command();
int update_process_state(pid_t pid, int state);
int add_process_activity(pid_t pid, const char *command, int state);
void init_activities();
void cleanup_activities();

#endif