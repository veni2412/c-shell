#include "hop.h"
#include "prompt.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>

// ############## LLM Generated Code Begins ##############

static char prev_cwd[PATH_MAX] = "";

void hop_command(const char *arg) {
    char temp_cwd[PATH_MAX];

    // Get current directory
    if (getcwd(temp_cwd, sizeof(temp_cwd)) == NULL) {
        perror("getcwd");
        return;
    }

    // Case 1: "~" or NULL
    if (arg == NULL || strcmp(arg, "~") == 0) {
        if (chdir(get_shell_home()) != 0) {
            fprintf(stderr, "No such directory!\n");
            return;
        }
        // Update previous directory only on successful directory change
        strncpy(prev_cwd, temp_cwd, sizeof(prev_cwd));
        prev_cwd[sizeof(prev_cwd) - 1] = '\0';
    }

    // ############## LLM Generated Code Ends ################

    // Case 2: "."
    else if (strcmp(arg, ".") == 0) {
        // Stay where you are, no need to update prev_cwd
    }

    // Case 3: ".."
    else if (strcmp(arg, "..") == 0) {
        if (chdir("..") != 0) {
            fprintf(stderr, "No such directory!\n");
            return;
        }
        // Update previous directory only on successful directory change
        strncpy(prev_cwd, temp_cwd, sizeof(prev_cwd));
        prev_cwd[sizeof(prev_cwd) - 1] = '\0';
    }

    // Case 4: "-"
    else if (strcmp(arg, "-") == 0) {
        if (strlen(prev_cwd) > 0) {
            char temp_cwd[PATH_MAX];
            
            // Save current dir before changing
            if (getcwd(temp_cwd, sizeof(temp_cwd)) != NULL) {
                // Change to previous directory
                if (chdir(prev_cwd) != 0) {
                    fprintf(stderr, "No such directory!\n");
                } else {
                    // Only update prev_cwd if directory change was successful
                    strncpy(prev_cwd, temp_cwd, sizeof(prev_cwd));
                    prev_cwd[sizeof(prev_cwd) - 1] = '\0';
                }
            }
        } else {
            // No previous directory yet, do nothing
            fprintf(stderr, "No previous directory!\n");
        }
    }

    // Case 5: any other name/path
    else {
        if (chdir(arg) != 0) {
            fprintf(stderr, "No such directory!\n");
            return;
        }
        // Update previous directory only on successful directory change
        strncpy(prev_cwd, temp_cwd, sizeof(prev_cwd));
        prev_cwd[sizeof(prev_cwd) - 1] = '\0';
    }
}

// New function to handle multiple hop arguments
void hop_command_multiple(char **args, int count) {
    // Save original directory to update prev_cwd only once at the end
    char original_cwd[PATH_MAX];
    if (getcwd(original_cwd, sizeof(original_cwd)) == NULL) {
        perror("getcwd");
        return;
    }
    
    // Process each argument in sequence
    for (int i = 0; i < count; i++) {
        // Apply each hop command
        hop_command(args[i]);
    }
    
    // Get final directory
    char final_cwd[PATH_MAX];
    if (getcwd(final_cwd, sizeof(final_cwd)) == NULL) {
        perror("getcwd");
        return;
    }
    
    // If we actually changed directories, update prev_cwd
    if (strcmp(original_cwd, final_cwd) != 0) {
        strncpy(prev_cwd, original_cwd, sizeof(prev_cwd));
        prev_cwd[sizeof(prev_cwd) - 1] = '\0';
    }
}