#ifndef REDIRECTION_H
#define REDIRECTION_H

/**
 * Handle input redirection for a command
 * 
 * @param input_file Path to the input file
 * @return 0 on success, -1 on failure
 */
int handle_input_redirection(const char *input_file);

/**
 * Handle output redirection for a command
 * 
 * @param output_file Path to the output file
 * @param append_mode If true, append to file; if false, truncate file
 * @return 0 on success, -1 on failure
 */
int handle_output_redirection(const char *output_file, int append_mode);

/**
 * Create an output file without redirecting to it
 * Used for Bash-like behavior with multiple redirections
 * 
 * @param output_file Path to the output file
 * @param append_mode If true, append to file; if false, truncate file
 * @return 0 on success, -1 on failure
 */
int create_output_file(const char *output_file, int append_mode);

/**
 * Cleanup any allocated memory for redirection
 * Should be called when the shell exits
 */
void cleanup_redirection();

#endif /* REDIRECTION_H */
