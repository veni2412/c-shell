// ################### LLM Generated Code Begins ###################

#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/wait.h>
#include "prompt.h"

static char log_entries[MAX_LOG_ENTRIES][MAX_CMD_LENGTH];
static int log_count = 0;
static int log_head = 0;

static char log_file_path[1024];

static void get_log_file_path(void) {
    const char *home = get_shell_home();
    if (home[0] != '\0') {
        snprintf(log_file_path, sizeof(log_file_path), "%s/.myshell_log", home);
    }
}

void load_log(){
    get_log_file_path();

    FILE *f = fopen(log_file_path, "r");
    
    if (!f){
        return;
    }

    log_count = 0;
    log_head = 0;

    char buffer[MAX_CMD_LENGTH];

    while (fgets(buffer, sizeof(buffer), f)){
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strlen(buffer)==0){
            continue;
        }

        strncpy(log_entries[log_count], buffer, MAX_CMD_LENGTH -1);
        log_entries[log_count][MAX_CMD_LENGTH-1]='\0';
        log_count++;
        if (log_count >= MAX_LOG_ENTRIES){
            break;
        }
    }
    fclose(f);
}

void save_log() {
    FILE *f = fopen(log_file_path, "w");
    if (!f){
        return;
    }

    for (int i=0; i<log_count; i++){
        int idx = (log_head + i) % MAX_LOG_ENTRIES;
        fprintf(f, "%s\n", log_entries[idx]);
    }

    fclose(f);
}

void add_log_entry(const char *cmd){
    if (strncmp(cmd, "log", 3)==0){
        return;
    }

    if (log_count > 0){
        int newest_idx = (log_head + log_count -1) % MAX_LOG_ENTRIES;
        if (strcmp(log_entries[newest_idx], cmd)==0){
            return;
        }
    }

    int insert_idx;
    if (log_count < MAX_LOG_ENTRIES){
        insert_idx = (log_head + log_count) % MAX_LOG_ENTRIES;
        log_count++;
    }
    else {
        insert_idx = log_head;
        log_head = (log_head + 1) % MAX_LOG_ENTRIES;
    }

    strncpy(log_entries[insert_idx], cmd, MAX_CMD_LENGTH-1);
    log_entries[insert_idx][MAX_CMD_LENGTH-1]='\0';

    save_log();
}

void print_log(){
    for (int i=0; i<log_count; i++){
        int idx = (log_head +i)%MAX_LOG_ENTRIES;
        printf("%s\n", log_entries[idx]);
    }
}

void purge_log() {
    log_count = 0;
    log_head = 0;

    FILE *f = fopen(log_file_path, "w");
    if (f){
        fclose(f);
    }
}

void execute_log_command(int index) {
    if (index <= 0 || index > log_count) {
        fprintf(stderr, "log: Invalid index!\n");
        return;
    }

    // newest = 1
    int newest_idx = (log_head + log_count - 1) % MAX_LOG_ENTRIES;
    int target_idx = (newest_idx - (index - 1) + MAX_LOG_ENTRIES) % MAX_LOG_ENTRIES;

    char *cmd = log_entries[target_idx];
    if (!cmd || strlen(cmd) == 0) {
        fprintf(stderr, "log: Invalid index!\n");
        return;
    }

    // Return the command to be executed by the caller
    printf("%s\n", cmd);
}

/**
 * Get a command at a given index from the log
 * 
 * @param index The index of the command (1=newest)
 * @return The command string, or NULL if index is invalid
 */
char *get_log_command(int index) {
    if (index <= 0 || index > log_count) {
        return NULL;
    }

    // newest = 1
    int newest_idx = (log_head + log_count - 1) % MAX_LOG_ENTRIES;
    int target_idx = (newest_idx - (index - 1) + MAX_LOG_ENTRIES) % MAX_LOG_ENTRIES;

    return log_entries[target_idx];
}

// ################### LLM Generated Code Ends ###################