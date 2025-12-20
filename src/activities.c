// LLM GENERATED CODE BEGINS HERE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "activities.h"

#define MAX_ACTIVITIES 1024

static pid_t activity_pids[MAX_ACTIVITIES];
static char *activity_commands[MAX_ACTIVITIES];
static int activity_states[MAX_ACTIVITIES];
static int activity_count = 0;


void init_activities() {
    activity_count = 0;
}

void cleanup_activities() {
    for (int i = 0; i < activity_count; i++) {
        free(activity_commands[i]);
    }
    activity_count = 0;
}

int add_process_activity(pid_t pid, const char *command, int state) {
    if (activity_count >= MAX_ACTIVITIES) {
        fprintf(stderr, "Too many processes\n");
        return -1;
    }

    activity_pids[activity_count] = pid;
    activity_commands[activity_count] = strdup(command);
    if (activity_commands[activity_count] == NULL) {
        perror("strdup");
        return -1;
    }
    activity_states[activity_count] = state;
    activity_count++;
    return 0;
}

int update_process_state(pid_t pid, int state) {
    for (int i = 0; i < activity_count; i++) {
        if (activity_pids[i] == pid) {
            activity_states[i] = state;
            return 0;
        }
    }
    return -1;
}

static int remove_terminated_processes() {
    int removed = 0;
    int status;
    for (int i = 0; i < activity_count; ) {
        pid_t result = waitpid(activity_pids[i], &status, WNOHANG);
        if (result > 0) {
            free(activity_commands[i]);
            
            for (int j = i; j < activity_count - 1; j++) {
                activity_pids[j] = activity_pids[j + 1];
                activity_commands[j] = activity_commands[j + 1];
                activity_states[j] = activity_states[j + 1];
            }
            activity_count--;
            removed++;
        } else {
            i++;
        }
    }
    return removed;
}

/* Compare function for qsort */
static int compare_activities(const void *a, const void *b) {
    int idx_a = *(const int *)a;
    int idx_b = *(const int *)b;
    return strcmp(activity_commands[idx_a], activity_commands[idx_b]);
}

/* Display all active processes */
int activities_command() {
    remove_terminated_processes();

    if (activity_count == 0) {
        printf("No active processes\n");
        return 0;
    }

    /* Create index array for sorting */
    int *indices = malloc(activity_count * sizeof(int));
    if (!indices) {
        perror("malloc");
        return -1;
    }
    for (int i = 0; i < activity_count; i++) {
        indices[i] = i;
    }

    qsort(indices, activity_count, sizeof(int), compare_activities);

    for (int i = 0; i < activity_count; i++) {
        int idx = indices[i];
        printf("[%d] : %s - %s\n",
               activity_pids[idx],
               activity_commands[idx],
               (activity_states[idx] == PROCESS_RUNNING ? "Running" : "Stopped"));
    }

    free(indices);
    return 0;
}

// LLM GENERATED CODE ENDS HERE