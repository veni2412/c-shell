#ifndef BUILTINS_H
#define BUILTINS_H

#include <stdbool.h>

/**
 * Check if a command is a built-in shell command
 * 
 * @param cmd The command to check
 * @return true if built-in, false otherwise
 */
bool is_builtin(const char *cmd);

/**
 * Run a built-in command
 * 
 * @param argc Number of arguments
 * @param argv Array of argument strings
 * @return Status code of command execution
 */
int run_builtin(int argc, char **argv);

#endif /* BUILTINS_H */
