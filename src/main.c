#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include "prompt.h"
#include "parser.h"
#include "hop.h"
#include "reveal.h"
#include "log.h"
#include "redirection.h"
#include "pipe.h"
#include "sequence.h"
#include "background.h"
#include "activities.h"
#include "ping.h"
#include "shortcuts.h"

int main() {
    char input[4096];

    // Initialize signal handlers for keyboard shortcuts
    init_shortcuts();

    // Initialize the shell's home directory
    initialize_shell_home();
    load_log();
    
    // Initialize background jobs and activities
    init_jobs();
    init_activities();

    while (1) {
        check_jobs();
        print_prompt();

        if (!fgets(input, sizeof(input), stdin)) {
            handle_eof();
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0) continue;

        if (strncmp(input, "log", 3) != 0 || 
            (strncmp(input, "log", 3) == 0 && strncmp(input, "log execute", 11) != 0)) {
            add_log_entry(input);
        }
        
        execute_command_sequence(input);
    }

    cleanup_jobs();
    cleanup_activities();
    cleanup_redirection(); // Clean up any allocated redirection memory
    return 0;
}