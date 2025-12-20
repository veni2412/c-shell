#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include "../include/job_control.h"
#include "../include/background.h"
#include "../include/activities.h"

/**
 * Check if a string is a valid job number
 * 
 * @param job_number_str String to check
 * @return 1 if valid, 0 otherwise
 */
static int is_valid_job_number(const char *job_number_str) {
    if (job_number_str == NULL || *job_number_str == '\0') {
        return 0;
    }
    
    // Check that all characters are digits
    for (int i = 0; job_number_str[i] != '\0'; i++) {
        if (!isdigit(job_number_str[i])) {
            return 0;
        }
    }
    
    return 1;
}

/**
 * Bring a background or stopped job to the foreground
 * 
 * If the job is stopped, send SIGCONT to resume it
 * If no job number is provided, use the most recently created background/stopped job
 * 
 * @param job_number_str String representation of the job number, or NULL for most recent
 * @return 0 on success, -1 on failure
 */
int fg_command(const char *job_number_str) {
    int job_id;
    
    if (job_number_str == NULL || *job_number_str == '\0') {
        // No job number provided, use the most recent job
        job_id = get_most_recent_job_id();
        if (job_id < 0) {
            fprintf(stderr, "No such job!\n");
            return -1;
        }
    } else {
        // Job number provided, validate it
        if (!is_valid_job_number(job_number_str)) {
            fprintf(stderr, "Invalid job number\n");
            return -1;
        }
        
        job_id = atoi(job_number_str);
        
        // Check if the job exists
        if (!job_exists(job_id)) {
            fprintf(stderr, "No such job!\n");
            return -1;
        }
    }
    
    // Get the job details
    pid_t pid;
    char *command;
    int state;
    
    if (get_job_details(job_id, &pid, &command, &state) < 0) {
        fprintf(stderr, "Error retrieving job details\n");
        return -1;
    }
    
    // Print the command
    printf("%s\n", command);
    
    // If the job is stopped, send SIGCONT to resume it
    if (state == JOB_STOPPED) {
        if (kill(pid, SIGCONT) < 0) {
            perror("kill");
            return -1;
        }
        
        // Update job state
        update_job_state(job_id, JOB_RUNNING);
        
        // Update activities list
        update_process_state(pid, PROCESS_RUNNING);
    }
    
    // Set as foreground process
    set_foreground_pid(pid);
    
    // Remove from background jobs list
    remove_job(job_id);
    
    // Wait for the process to complete or stop again
    int status;
    waitpid(pid, &status, WUNTRACED);
    
    // Reset foreground pid
    set_foreground_pid(-1);
    
    // If the process was stopped, add it back to the job list
    if (WIFSTOPPED(status)) {
        add_job_with_id(job_id, pid, command);
        update_job_state(job_id, JOB_STOPPED);
        update_process_state(pid, PROCESS_STOPPED);
        printf("[%d] Stopped %s\n", job_id, command);
    }
    
    return 0;
}

/**
 * Resume a stopped background job
 * 
 * Send SIGCONT to the job and let it continue running in the background
 * 
 * @param job_number_str String representation of the job number, or NULL for most recent
 * @return 0 on success, -1 on failure
 */
int bg_command(const char *job_number_str) {
    int job_id;
    
    if (job_number_str == NULL || *job_number_str == '\0') {
        // No job number provided, use the most recent stopped job
        job_id = get_most_recent_stopped_job_id();
        if (job_id < 0) {
            fprintf(stderr, "No stopped jobs\n");
            return -1;
        }
    } else {
        // Job number provided, validate it
        if (!is_valid_job_number(job_number_str)) {
            fprintf(stderr, "Invalid job number\n");
            return -1;
        }
        
        job_id = atoi(job_number_str);
        
        // Check if the job exists
        if (!job_exists(job_id)) {
            fprintf(stderr, "No such job!\n");
            return -1;
        }
    }
    
    // Get the job details
    pid_t pid;
    char *command;
    int state;
    
    if (get_job_details(job_id, &pid, &command, &state) < 0) {
        fprintf(stderr, "Error retrieving job details\n");
        return -1;
    }
    
    // Check if the job is already running
    if (state == JOB_RUNNING) {
        fprintf(stderr, "Job already running\n");
        return -1;
    }
    
    // Send SIGCONT to resume the job
    if (kill(pid, SIGCONT) < 0) {
        perror("kill");
        return -1;
    }
    
    // Update job state
    update_job_state(job_id, JOB_RUNNING);
    
    // Update activities list
    update_process_state(pid, PROCESS_RUNNING);
    
    // Print the job information
    printf("[%d] %s &\n", job_id, command);
    
    return 0;
}
