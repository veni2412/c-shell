#ifndef SHELL_PARSER
#define SHELL_PARSER

void init_shell_parser(const char *str);

int parse_cmd_shell();

/**
 * Tokenize a command string into an array of tokens
 * 
 * @param input_str The command string to tokenize
 * @param count Pointer to an integer that will store the number of tokens
 * @return Array of token strings, must be freed with free_tokens
 */
char **tokenize_command(const char *input_str, int *count);

/**
 * Free the memory allocated for tokens
 * 
 * @param tokens Array of token strings
 * @param count Number of tokens
 */
void free_tokens(char **tokens, int count);

/**
 * Split a command string into pipeline stages
 * 
 * @param cmd The command string to split
 * @param parts Array to store the pipeline parts
 * @return Number of pipeline parts
 */
int split_pipeline_commands(const char *cmd, char **parts);

#endif