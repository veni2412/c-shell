#ifndef SEQUENCE_H
#define SEQUENCE_H

/**
 * Execute a sequence of commands separated by semicolons
 * 
 * @param command_string The complete command string potentially containing semicolons
 * @return 0 on success, -1 on failure
 */
int execute_command_sequence(char *command_string);

#endif /* SEQUENCE_H */
