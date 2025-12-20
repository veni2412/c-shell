#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include "ping.h"

/**
 * Check if a string represents a valid integer
 * 
 * @param str The string to check
 * @return 1 if valid integer, 0 otherwise
 */
static int is_valid_number(const char *str) {
    if (str == NULL || *str == '\0') {
        return 0;
    }
    
    // Check for negative numbers
    if (*str == '-') {
        str++;
        if (*str == '\0') {
            return 0; // Just a minus sign
        }
    }
    
    // Check that all remaining characters are digits
    while (*str) {
        if (!isdigit(*str)) {
            return 0;
        }
        str++;
    }
    
    return 1;
}

/**
 * Send a signal to a process with the specified PID
 * 
 * Format: ping <pid> <signal_number>
 * 
 * - Takes signal_number modulo 32 before sending
 * - Prints "No such process found" if process doesn't exist
 * - Prints "Sent signal <signal_number> to process with pid <pid>" on success
 * - Prints "Invalid syntax!" if signal_number is not a valid number
 * 
 * @param pid_str String representation of the process ID
 * @param signal_str String representation of the signal number
 * @return 0 on success, -1 on failure
 */
int ping_command(const char *pid_str, const char *signal_str) {
    // Check if pid_str is a valid number
    if (!is_valid_number(pid_str)) {
        fprintf(stderr, "Invalid syntax!\n");
        return -1;
    }
    
    // Check if signal_str is a valid number
    if (!is_valid_number(signal_str)) {
        fprintf(stderr, "Invalid syntax!\n");
        return -1;
    }
    
    // Convert strings to integers
    pid_t pid = (pid_t)atoi(pid_str);
    int signal_number = atoi(signal_str);
    
    // Take signal number modulo 32
    int actual_signal = signal_number % 32;
    
    // Send the signal
    if (kill(pid, actual_signal) < 0) {
        if (errno == ESRCH) {
            // Process does not exist
            fprintf(stderr, "No such process found\n");
        } else {
            // Other errors
            perror("kill");
        }
        return -1;
    }
    
    // Signal sent successfully
    printf("Sent signal %d to process with pid %d\n", signal_number, pid);
    return 0;
}
