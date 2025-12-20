#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "parser.h"
#include "builtins.h"   // <- header for hop, reveal, log
#include "log.h"        // for get_log_command
#include "redirection.h" // for handle_input/output_redirection

// Forward declare
int execute_pipeline(char **commands, int n);

int execute_pipeline(char **commands, int n) {
    int pipefd[2], in_fd = 0, status;
    pid_t pid;

    for (int i = 0; i < n; i++) {
        pipe(pipefd);

        if ((pid = fork()) == 0) {
            // Child
            dup2(in_fd, 0);
            if (i < n - 1) dup2(pipefd[1], 1);

            close(pipefd[0]);
            close(pipefd[1]);

            // --- builtin handling ---
            char *argv[64] = {0};
            int token_count = 0;
            char *cmd_copy = strdup(commands[i]);
            
            // Handle redirections in the command
            char *input_file = NULL;
            char *output_file = NULL;
            int append_mode = 0;
            
            // Tokenize and process redirections
            char *token = strtok(cmd_copy, " \t");
            while (token && token_count < 63) {
                if (strcmp(token, "<") == 0) {
                    token = strtok(NULL, " \t");
                    if (token) input_file = strdup(token);
                } else if (strcmp(token, ">") == 0) {
                    token = strtok(NULL, " \t");
                    if (token) {
                        output_file = strdup(token);
                        append_mode = 0;
                    }
                } else if (strcmp(token, ">>") == 0) {
                    token = strtok(NULL, " \t");
                    if (token) {
                        output_file = strdup(token);
                        append_mode = 1;
                    }
                } else {
                    argv[token_count++] = strdup(token);
                }
                token = strtok(NULL, " \t");
            }
            argv[token_count] = NULL;
            free(cmd_copy);
            
            // Apply redirections for the last command in the pipeline
            if (i == n - 1 && output_file) {
                if (handle_output_redirection(output_file, append_mode) < 0) {
                    // Clean up
                    if (input_file) free(input_file);
                    if (output_file) free(output_file);
                    for (int j = 0; j < token_count; j++) {
                        if (argv[j]) free(argv[j]);
                    }
                    exit(1);
                }
            }
            
            // Apply input redirection for the first command
            if (i == 0 && input_file) {
                if (handle_input_redirection(input_file) < 0) {
                    // Clean up
                    if (input_file) free(input_file);
                    if (output_file) free(output_file);
                    for (int j = 0; j < token_count; j++) {
                        if (argv[j]) free(argv[j]);
                    }
                    exit(1);
                }
            }

            if (is_builtin(argv[0])) {
                // special case: log execute
                if (strcmp(argv[0], "log") == 0 && argv[1] && strcmp(argv[1], "execute") == 0) {
                    if (!argv[2]) {
                        fprintf(stderr, "log: Missing index!\n");
                        exit(1);
                    }
                    int idx = atoi(argv[2]);
                    char *log_cmd = get_log_command(idx);
                    if (!log_cmd) {
                        fprintf(stderr, "log: Invalid index!\n");
                        exit(1);
                    }

                    // Split stored command into pipeline stages manually
                    char *log_parts[128] = {0};
                    int log_n = 0;
                    
                    char *log_copy = strdup(log_cmd);
                    char *part = strtok(log_copy, "|");
                    while (part && log_n < 127) {
                        // Trim leading/trailing whitespace
                        while (*part && (*part == ' ' || *part == '\t')) part++;
                        char *end = part + strlen(part) - 1;
                        while (end > part && (*end == ' ' || *end == '\t')) *end-- = '\0';
                        
                        if (*part) {
                            log_parts[log_n++] = strdup(part);
                        }
                        part = strtok(NULL, "|");
                    }
                    log_parts[log_n] = NULL;
                    free(log_copy);
                    
                    execute_pipeline(log_parts, log_n);
                    
                    // Free allocated memory
                    for (int j = 0; j < log_n; j++) {
                        free(log_parts[j]);
                    }
                    
                    // Clean up other memory
                    if (input_file) free(input_file);
                    if (output_file) free(output_file);
                    for (int j = 0; j < token_count; j++) {
                        if (argv[j]) free(argv[j]);
                    }
                    exit(0);
                }
                // normal builtin
                run_builtin(token_count, argv);
                
                // Clean up
                if (input_file) free(input_file);
                if (output_file) free(output_file);
                for (int j = 0; j < token_count; j++) {
                    if (argv[j]) free(argv[j]);
                }
                exit(0);
            } else {
                execvp(argv[0], argv);
                fprintf(stderr, "Command not found!\n");
                
                // Clean up
                if (input_file) free(input_file);
                if (output_file) free(output_file);
                for (int j = 0; j < token_count; j++) {
                    if (argv[j]) free(argv[j]);
                }
                exit(1);
            }
        }

        // Parent
        waitpid(pid, &status, 0);
        close(pipefd[1]);
        in_fd = pipefd[0];
    }

    return status;
}