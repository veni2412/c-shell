#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // Added for strdup()
#include <unistd.h>
#include <fcntl.h>
#include "redirection.h"

// Global pointers that should be freed between redirections
static char *last_input_file = NULL;
static char *last_output_file = NULL;

int handle_input_redirection(const char *input_file) {
    // Free the previous input file if it exists
    if (last_input_file) {
        free(last_input_file);
        last_input_file = NULL;
    }
    
    // Store a copy of the new input file for later cleanup
    if (input_file) {
        last_input_file = strdup(input_file);
    }
    
    if (!input_file) return 0;

    int fd = open(input_file, O_RDONLY);
    if (fd < 0) {
        perror(input_file);
        return -1;
    }

    if (dup2(fd, STDIN_FILENO) < 0) {
        perror("dup2 failed for input redirection");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

int handle_output_redirection(const char *output_file, int append_mode) {
    // Free the previous output file if it exists
    if (last_output_file) {
        free(last_output_file);
        last_output_file = NULL;
    }
    
    // Store a copy of the new output file for later cleanup
    if (output_file) {
        last_output_file = strdup(output_file);
    }
    
    if (!output_file) return 0;

    int flags = O_WRONLY | O_CREAT;
    if (append_mode) flags |= O_APPEND;
    else flags |= O_TRUNC;

    int fd = open(output_file, flags, 0644);
    if (fd < 0) {
        perror(output_file);
        fprintf(stderr, "Unable to create file for writing\n");
        return -1;
    }

    if (dup2(fd, STDOUT_FILENO) < 0) {
        perror("dup2 failed for output redirection");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

// Function to create output file without redirection
// Used for Bash-like behavior with multiple redirections
int create_output_file(const char *output_file, int append_mode) {
    if (!output_file) return 0;

    int flags = O_WRONLY | O_CREAT;
    if (append_mode) flags |= O_APPEND;
    else flags |= O_TRUNC;

    int fd = open(output_file, flags, 0644);
    if (fd < 0) {
        perror(output_file);
        fprintf(stderr, "Unable to create file for writing\n");
        return -1;
    }
    
    // Close immediately, we just want to create the file
    close(fd);
    return 0;
}

// Function to cleanup redirection memory when the shell exits
void cleanup_redirection() {
    if (last_input_file) {
        free(last_input_file);
        last_input_file = NULL;
    }
    
    if (last_output_file) {
        free(last_output_file);
        last_output_file = NULL;
    }
}