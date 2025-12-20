// LLM GENERATED CODE

#define _POSIX_C_SOURCE 200809L   // enables POSIX APIs like sigaction

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "shortcuts.h"
#include "background.h"
#include "activities.h"

void init_shortcuts(void) {
    struct sigaction sa_int;
    struct sigaction sa_tstp;

    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("sigaction(SIGINT)");
        exit(EXIT_FAILURE);
    }

    // Set up SIGTSTP (Ctrl-Z) handler
    sa_tstp.sa_handler = sigtstp_handler;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART;
    if (sigaction(SIGTSTP, &sa_tstp, NULL) == -1) {
        perror("sigaction(SIGTSTP)");
        exit(EXIT_FAILURE);
    }
}


void sigint_handler(int sig) {
    pid_t fg_pid = get_foreground_pid();

    if (fg_pid > 0) {
        // Send SIGINT to the foreground process group
        kill(-fg_pid, SIGINT);
    } else {
        // No foreground process, just print a new line
        printf("\n");
        fflush(stdout);
    }
    (void)sig; // suppress unused parameter warning
}


void sigtstp_handler(int sig) {
    pid_t fg_pid = get_foreground_pid();

    if (fg_pid > 0) {
        const char *cmd = get_foreground_command();
        
        // If we don't have the command name, use a placeholder
        if (!cmd) {
            cmd = "(foreground process)";
        }

        // Send SIGTSTP to the foreground process group
        kill(-fg_pid, SIGTSTP);

        // Stop and move to background
        stop_foreground_process(cmd);
        
        // Ensure shell prompt is redisplayed
        printf("\n");
        fflush(stdout);
    }
    (void)sig; // suppress unused parameter warning
}

/**
 * Handle EOF (Ctrl-D) condition
 */
void handle_eof(void) {
    printf("\nlogout\n");
    fflush(stdout);

    kill_all_processes();
    cleanup_jobs();
    cleanup_activities();

    exit(EXIT_SUCCESS);
}

// LLM GENERATED CODE