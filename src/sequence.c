#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>
#include "reveal.h"
#include "hop.h"
#include "log.h"
#include "pipe.h"
#include "background.h"
#include "activities.h"
#include "redirection.h"
#include "job_control.h"
#include "parser.h"

#define MAX_TOKENS 128
#define MAX_CMD_LEN 4096

// Forward declarations
int execute_command_sequence(char *command_string);

// Tokenization Functions
// =====================
static int tokenize_command_internal(char *input, char **tokens) {
    int count = 0;
    char *saveptr = NULL;
    char *token = strtok_r(input, " \t", &saveptr);
    while (token && count < MAX_TOKENS - 1) {
        tokens[count++] = token;
        token = strtok_r(NULL, " \t", &saveptr);
    }
    tokens[count] = NULL;
    return count;
}

// =====================
// Single Command Execution
// =====================
static int execute_single_command(char *cmd_str) {
    while (*cmd_str && isspace(*cmd_str)) cmd_str++;
    if (*cmd_str == '\0') return 0;

    // Validate command syntax using the grammar parser
    init_shell_parser(cmd_str);
    if (!parse_cmd_shell()) {
        fprintf(stderr, "Invalid Syntax!\n");
        return 0;
    }

    // Check for background execution '&'
    int background = 0;
    size_t len = strlen(cmd_str);
    if (len > 0 && cmd_str[len-1] == '&') {
        background = 1;
        cmd_str[len-1] = '\0';
    }

    // Copy for pipeline parsing
    char cmd_copy[MAX_CMD_LEN];
    strncpy(cmd_copy, cmd_str, MAX_CMD_LEN - 1);
    cmd_copy[MAX_CMD_LEN - 1] = '\0';

    // Check for pipeline
    if (strchr(cmd_copy, '|')) {
        // The parser should have already validated this command at the beginning of the function,
        // but we'll check for specific pipeline syntax issues here as well
        char *commands[64];
        int cmd_count = 0;
        int invalid_syntax = 0;
        char *tmp = strtok(cmd_copy, "|");
        while (tmp && cmd_count < 64) {
            // Trim leading whitespace
            while (*tmp && isspace(*tmp)) tmp++;
            // Check for empty command (after trimming)
            if (*tmp == '\0') {
                invalid_syntax = 1;
            }
            commands[cmd_count++] = strdup(tmp);
            tmp = strtok(NULL, "|");
        }
        // Also check for trailing pipe (e.g., 'cmd |')
        if (cmd_count == 0 || invalid_syntax) {
            fprintf(stderr, "Invalid Syntax!\n");
            for (int i = 0; i < cmd_count; i++) free(commands[i]);
            return 0;
        }
        execute_pipeline(commands, cmd_count);
        for (int i = 0; i < cmd_count; i++) free(commands[i]);
        return 0;
    }

    // Tokenize command
    char *tokens[MAX_TOKENS];
    int token_count = tokenize_command_internal(cmd_str, tokens);
    
    // Handle redirection - initialize pointers to NULL
    char *input_file = NULL;
    char *output_file = NULL;
    int append_mode = 0;
    
    // Keep track of all output files to create them like bash does
    char *all_output_files[MAX_TOKENS] = {NULL};
    int all_append_modes[MAX_TOKENS] = {0};
    int output_file_count = 0;
    
    for (int i = 0; i < token_count; i++) {
        if (strcmp(tokens[i], "<") == 0 && i + 1 < token_count) {
            input_file = tokens[i + 1]; tokens[i] = NULL; tokens[i+1] = NULL; i++;
        } else if (strcmp(tokens[i], ">") == 0 && i + 1 < token_count) {
            // Track all output files for Bash-like behavior
            all_output_files[output_file_count] = tokens[i + 1];
            all_append_modes[output_file_count] = 0;
            output_file_count++;
            
            // Last one is the actual one we'll use
            output_file = tokens[i + 1]; 
            append_mode = 0; 
            tokens[i] = NULL; 
            tokens[i+1] = NULL; 
            i++;
        } else if (strcmp(tokens[i], ">>") == 0 && i + 1 < token_count) {
            // Track all output files for Bash-like behavior
            all_output_files[output_file_count] = tokens[i + 1];
            all_append_modes[output_file_count] = 1;
            output_file_count++;
            
            // Last one is the actual one we'll use
            output_file = tokens[i + 1]; 
            append_mode = 1; 
            tokens[i] = NULL; 
            tokens[i+1] = NULL; 
            i++;
        }
    }

    // Find command name
    char *cmd = NULL;
    for (int i = 0; i < token_count; i++) {
        if (tokens[i]) {
            cmd = tokens[i];
            break;
        }
    }
    
    if (!cmd) return 0;
    
    if (strcmp(cmd, "hop") == 0) {
        // Collect all hop arguments
        char *hop_args[MAX_TOKENS];
        int hop_arg_count = 0;
        
        for (int i = 0; i < token_count; i++) {
            if (tokens[i] && strcmp(tokens[i], cmd) != 0) {
                hop_args[hop_arg_count++] = tokens[i];
            }
        }
        
        // Setup redirection
        int stdout_fd = -1;
        if (output_file) {
            stdout_fd = dup(STDOUT_FILENO);
            handle_output_redirection(output_file, append_mode);
        }
        
        // No args -> hop to home
        if (hop_arg_count == 0) {
            hop_command(NULL);
        } else if (hop_arg_count == 1) {
            // Single argument case
            hop_command(hop_args[0]);
        } else {
            // Multiple arguments case
            hop_command_multiple(hop_args, hop_arg_count);
        }
        
        // Restore stdout if needed
        if (stdout_fd != -1) {
            dup2(stdout_fd, STDOUT_FILENO);
            close(stdout_fd);
        }
        
        return 0;
    }
    else if (strcmp(cmd, "reveal") == 0) {
        int a = 0, l = 0;
        char *path = NULL;
        
        // Parse arguments
        for (int i = 0; i < token_count; i++) {
            if (!tokens[i] || strcmp(tokens[i], cmd) == 0) continue;
            
            // Handle combined flags like -la or -al
            if (tokens[i][0] == '-' && tokens[i][1] != '\0' && tokens[i][1] != '-') {
                for (int j = 1; tokens[i][j] != '\0'; j++) {
                    if (tokens[i][j] == 'a') a = 1;
                    else if (tokens[i][j] == 'l') l = 1;
                    else {
                        fprintf(stderr, "reveal: Invalid Syntax!\n");
                        return -1;
                    }
                }
            }
            else if (!path) path = tokens[i];
            else {
                fprintf(stderr, "reveal: Invalid Syntax!\n");
                return -1;
            }
        }
        
        // Setup redirection
        int stdout_fd = -1;
        if (output_file) {
            stdout_fd = dup(STDOUT_FILENO);
            handle_output_redirection(output_file, append_mode);
        }
        
        // Only update prev_cwd if not using the "-" option
        if (path == NULL || strcmp(path, "-") != 0) {
            update_reveal_prev_cwd();
        }
        
        reveal_command(a, l, path);
        
        // Restore stdout if needed
        if (stdout_fd != -1) {
            dup2(stdout_fd, STDOUT_FILENO);
            close(stdout_fd);
        }
        
        return 0;
    }
    else if (strcmp(cmd, "log") == 0) {
        char *subcmd = NULL;
        for (int i = 0; i < token_count; i++) {
            if (tokens[i] && strcmp(tokens[i], cmd) != 0) {
                subcmd = tokens[i];
                break;
            }
        }
        
        // Setup redirection
        int stdout_fd = -1;
        if (output_file) {
            stdout_fd = dup(STDOUT_FILENO);
            handle_output_redirection(output_file, append_mode);
        }
        
        if (!subcmd) {
            print_log();
        }
        else if (strcmp(subcmd, "purge") == 0) {
            purge_log();
        }
        else if (strcmp(subcmd, "execute") == 0) {
            int idx = -1;
            for (int i = 1; i < token_count; i++) {
                if (tokens[i] && isdigit(tokens[i][0])) { idx = atoi(tokens[i]); break; }
            }
            char *log_cmd = get_log_command(idx);
            if (!log_cmd) { fprintf(stderr, "log: Invalid index!\n"); return -1; }
            return execute_command_sequence(log_cmd);
        } else { fprintf(stderr, "Invalid Syntax!\n"); return -1; }
        
        // Restore stdout if needed
        if (stdout_fd != -1) {
            dup2(stdout_fd, STDOUT_FILENO);
            close(stdout_fd);
        }
        
        return 0;
    }
    else if (strcmp(cmd, "fg") == 0) {
        char *job_id = NULL;
        for (int i = 0; i < token_count; i++) {
            if (tokens[i] && strcmp(tokens[i], cmd) != 0) {
                job_id = tokens[i];
                break;
            }
        }
        
        // Setup redirection if needed
        int stdout_fd = -1;
        if (output_file) {
            stdout_fd = dup(STDOUT_FILENO);
            handle_output_redirection(output_file, append_mode);
        }
        
        // Call fg_command with job_id
        fg_command(job_id);
        
        // Restore stdout if needed
        if (stdout_fd != -1) {
            dup2(stdout_fd, STDOUT_FILENO);
            close(stdout_fd);
        }
        
        return 0;
    }
    else if (strcmp(cmd, "bg") == 0) {
        char *job_id = NULL;
        for (int i = 0; i < token_count; i++) {
            if (tokens[i] && strcmp(tokens[i], cmd) != 0) {
                job_id = tokens[i];
                break;
            }
        }
        
        // Setup redirection if needed
        int stdout_fd = -1;
        if (output_file) {
            stdout_fd = dup(STDOUT_FILENO);
            handle_output_redirection(output_file, append_mode);
        }
        
        // Call bg_command with job_id
        bg_command(job_id);
        
        // Restore stdout if needed
        if (stdout_fd != -1) {
            dup2(stdout_fd, STDOUT_FILENO);
            close(stdout_fd);
        }
        
        return 0;
    }
    
    // For external commands, fork and exec
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        
        // In Bash, all redirected files are created, even if only the last one is used
        // First create all output files that were specified
        for (int i = 0; i < output_file_count - 1; i++) {
            if (all_output_files[i]) {
                // Use our new function that creates files without redirection
                create_output_file(all_output_files[i], all_append_modes[i]);
            }
        }
        
        // Now handle actual redirection for input and the last output
        if (input_file) {
            if (handle_input_redirection(input_file) < 0) {
                exit(1); // Exit child process if input redirection fails
            }
        }
        if (output_file) {
            if (handle_output_redirection(output_file, append_mode) < 0) {
                exit(1); // Exit child process if output redirection fails
            }
        }
        
        // Find first non-null token after command
        int arg_start = 0;
        while (arg_start < token_count && tokens[arg_start] != cmd) arg_start++;
        if (arg_start == token_count) { fprintf(stderr, "Command not found!\n"); exit(1); }
        
        // Build argv array for exec
        char *argv[MAX_TOKENS];
        int argc = 0;
        
        // First arg is command name
        argv[argc++] = cmd;
        
        // Add remaining arguments
        for (int i = arg_start + 1; i < token_count; i++) {
            if (tokens[i]) argv[argc++] = tokens[i];
        }
        argv[argc] = NULL;
        
        execvp(cmd, argv);
        fprintf(stderr, "Command not found!\n");
        exit(1);
    } else {
        // Parent
        if (!background) {
            int status;
            waitpid(pid, &status, 0);
        } else {
            add_process_activity(pid, cmd, PROCESS_RUNNING);
        }
    }

    return 0;
}

// =====================
// Execute sequence of commands (semicolon-separated)
// =====================
int execute_command_sequence(char *command_string) {
    // Validate command syntax using the grammar parser
    init_shell_parser(command_string);
    if (!parse_cmd_shell()) {
        fprintf(stderr, "Invalid Syntax!\n");
        return 0;
    }
    
    char *cmd_start = command_string;
    char *semicolon;
    char temp_char;

    // Check for initial semicolon
    while (*cmd_start && isspace(*cmd_start)) cmd_start++;
    if (*cmd_start == ';') {
        fprintf(stderr, "Invalid Syntax!\n");
        return 0;
    }

    while (cmd_start && *cmd_start) {
        semicolon = strchr(cmd_start, ';');
        
        if (semicolon) {
            temp_char = *semicolon;
            *semicolon = '\0';
        }
        
        execute_single_command(cmd_start);
        
        if (semicolon) {
            *semicolon = temp_char;
            cmd_start = semicolon + 1;
            
            // Check for empty command between semicolons (like cmd1;;cmd2)
            char *tmp = cmd_start;
            while (*tmp && isspace(*tmp)) tmp++;
            if (*tmp == ';') {
                fprintf(stderr, "Invalid Syntax!\n");
                return 0;
            }
        } else {
            break;
        }
    }

    return 0;
}
