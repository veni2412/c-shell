#ifndef LOG_H
#define LOG_H

#define MAX_LOG_ENTRIES 15
#define MAX_CMD_LENGTH 4096

//load log from file into memory
void load_log();

//save log from memory into file
void save_log();

//add a command to the log
void add_log_entry(const char *cmd);

//print log (oldest->newest)
void print_log();

//purge the log
void purge_log();

//execute a command at given index (1=newest)
void execute_log_command(int index);

/**
 * Get a command at a given index from the log
 * 
 * @param index The index of the command (1=newest)
 * @return The command string, or NULL if index is invalid
 */
char *get_log_command(int index);

#endif