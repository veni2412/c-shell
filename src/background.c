// LLM GENERATED CODE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "background.h"
#include "activities.h"

static job_t *jobs_list = NULL;
static int next_job_id = 1;
static pid_t foreground_pid = -1;
static char foreground_command[256] = "";

void init_jobs() {
    jobs_list = NULL;
    next_job_id = 1;
}

void add_job(pid_t pid, const char *command) {
    job_t *new_job = (job_t *)malloc(sizeof(job_t));
    if (!new_job) {
        perror("malloc");
        return;
    }

    new_job->job_id = next_job_id++;
    new_job->pid = pid;
    new_job->command = strdup(command);
    new_job->state = JOB_RUNNING;
    new_job->next = NULL;

    // Add to the end of the list
    if (jobs_list == NULL) {
        jobs_list = new_job;
    } else {
        job_t *current = jobs_list;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_job;
    }

    // Print job information
    printf("[%d] %d\n", new_job->job_id, new_job->pid);
    
    // Add to activities list
    add_process_activity(pid, command, PROCESS_RUNNING);
}

static void remove_job_by_pid(pid_t pid) {
    job_t *current = jobs_list;
    job_t *prev = NULL;

    while (current != NULL) {
        if (current->pid == pid) {
            if (prev == NULL) {
                // Removing the first job
                jobs_list = current->next;
            } else {
                // Removing a job from the middle or end
                prev->next = current->next;
            }
            
            free(current->command);
            free(current);
            return;
        }
        
        prev = current;
        current = current->next;
    }
}


static job_t* find_job(pid_t pid) {
    job_t *current = jobs_list;
    
    while (current != NULL) {
        if (current->pid == pid) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

void check_jobs() {
    pid_t pid;
    int status;
    
    // Non-blocking wait to check for completed children
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        job_t *job = find_job(pid);
        
        if (job != NULL) {
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                printf("%s with pid %d exited normally\n", job->command, job->pid);
            } else {
                printf("%s with pid %d exited abnormally\n", job->command, job->pid);
            }
            
            remove_job_by_pid(pid);
        }
    }
}

void cleanup_jobs() {
    job_t *current = jobs_list;
    job_t *next;
    
    while (current != NULL) {
        next = current->next;
        free(current->command);
        free(current);
        current = next;
    }
    
    jobs_list = NULL;
}

int redirect_background_stdin() {
    int fd = open("/dev/null", O_RDONLY);
    if (fd == -1) {
        perror("open /dev/null");
        return -1;
    }
    
    // Redirect stdin to /dev/null
    if (dup2(fd, STDIN_FILENO) == -1) {
        perror("dup2");
        close(fd);
        return -1;
    }
    
    close(fd);
    return 0;
}

int is_background_command(char *command) {
    int len = strlen(command);
    
    // Command must have at least one character
    if (len == 0) {
        return 0;
    }
    
    // Check if the last character is &
    if (command[len - 1] == '&') {
        // Remove the & from the command
        command[len - 1] = '\0';
        
        // Remove any trailing whitespace
        len = strlen(command);
        while (len > 0 && (command[len - 1] == ' ' || command[len - 1] == '\t')) {
            command[len - 1] = '\0';
            len--;
        }
        
        return 1;
    }
    
    return 0;
}

void set_foreground_pid(pid_t pid) {
    foreground_pid = pid;
}

void set_foreground_command(const char *command) {
    if (command) {
        strncpy(foreground_command, command, sizeof(foreground_command) - 1);
        foreground_command[sizeof(foreground_command) - 1] = '\0';
    } else {
        foreground_command[0] = '\0';
    }
}

const char *get_foreground_command() {
    if (foreground_command[0] == '\0') {
        return NULL;
    }
    return foreground_command;
}

pid_t get_foreground_pid() {
    return foreground_pid;
}

int stop_foreground_process(const char *command) {
    if (foreground_pid <= 0) {
        return -1; // No foreground process
    }
    
    // Move the process to the jobs list
    add_job(foreground_pid, command);
    
    // Update the job state to stopped
    int job_id = get_most_recent_job_id();
    if (job_id > 0) {
        update_job_state(job_id, JOB_STOPPED);
    }
    
    // Update the process state in activities list
    update_process_state(foreground_pid, PROCESS_STOPPED);
    
    // Print the job information
    job_t *job = find_job(foreground_pid);
    if (job) {
        printf("[%d] Stopped %s\n", job->job_id, job->command);
    }
    
    // Reset foreground PID
    foreground_pid = -1;
    
    return 0;
}

/**
 * Kill all child processes spawned by the shell
 */
void kill_all_processes() {
    // Kill all processes in the job list
    job_t *current = jobs_list;
    
    while (current != NULL) {
        kill(current->pid, SIGKILL);
        current = current->next;
    }
    
    // Kill foreground process if exists
    if (foreground_pid > 0) {
        kill(foreground_pid, SIGKILL);
    }
}

/**
 * Get the most recent job ID
 * 
 * @return The most recent job ID, or -1 if no jobs exist
 */
int get_most_recent_job_id() {
    if (jobs_list == NULL) {
        return -1;
    }
    
    // The most recent job is the one with the highest job ID
    int highest_id = -1;
    job_t *current = jobs_list;
    
    while (current != NULL) {
        if (current->job_id > highest_id) {
            highest_id = current->job_id;
        }
        current = current->next;
    }
    
    return highest_id;
}

/**
 * Get the most recent stopped job ID
 * 
 * @return The most recent stopped job ID, or -1 if no stopped jobs exist
 */
int get_most_recent_stopped_job_id() {
    if (jobs_list == NULL) {
        return -1;
    }
    
    // The most recent stopped job is the one with the highest job ID that is stopped
    int highest_id = -1;
    job_t *current = jobs_list;
    
    while (current != NULL) {
        if (current->state == JOB_STOPPED && current->job_id > highest_id) {
            highest_id = current->job_id;
        }
        current = current->next;
    }
    
    return highest_id;
}

/**
 * Check if a job with the given ID exists
 * 
 * @param job_id Job ID to check
 * @return 1 if job exists, 0 otherwise
 */
int job_exists(int job_id) {
    job_t *current = jobs_list;
    
    while (current != NULL) {
        if (current->job_id == job_id) {
            return 1;
        }
        current = current->next;
    }
    
    return 0;
}

/**
 * Get the details of a job by ID
 * 
 * @param job_id Job ID to look up
 * @param pid Pointer to store the process ID
 * @param command Pointer to store the command string
 * @param state Pointer to store the job state
 * @return 0 on success, -1 if job not found
 */
int get_job_details(int job_id, pid_t *pid, char **command, int *state) {
    job_t *current = jobs_list;
    
    while (current != NULL) {
        if (current->job_id == job_id) {
            *pid = current->pid;
            *command = current->command;
            *state = current->state;
            return 0;
        }
        current = current->next;
    }
    
    return -1;
}

/**
 * Update the state of a job
 * 
 * @param job_id Job ID to update
 * @param state New state (JOB_RUNNING or JOB_STOPPED)
 * @return 0 on success, -1 if job not found
 */
int update_job_state(int job_id, int state) {
    job_t *current = jobs_list;
    
    while (current != NULL) {
        if (current->job_id == job_id) {
            current->state = state;
            return 0;
        }
        current = current->next;
    }
    
    return -1;
}

/**
 * Remove a job from the background jobs list
 * 
 * @param job_id Job ID to remove
 * @return 0 on success, -1 if job not found
 */
int remove_job(int job_id) {
    job_t *current = jobs_list;
    job_t *prev = NULL;
    
    while (current != NULL) {
        if (current->job_id == job_id) {
            if (prev == NULL) {
                // Removing the first job
                jobs_list = current->next;
            } else {
                // Removing a job from the middle or end
                prev->next = current->next;
            }
            
            // Don't free the command string since it might still be used
            // by the calling function
            free(current);
            return 0;
        }
        
        prev = current;
        current = current->next;
    }
    
    return -1;
}

/**
 * Add a job with a specific ID to the background jobs list
 * 
 * @param job_id Job ID to use
 * @param pid Process ID
 * @param command Command string
 * @return 0 on success, -1 on failure
 */
int add_job_with_id(int job_id, pid_t pid, const char *command) {
    job_t *new_job = (job_t *)malloc(sizeof(job_t));
    if (!new_job) {
        perror("malloc");
        return -1;
    }

    new_job->job_id = job_id;
    new_job->pid = pid;
    new_job->command = strdup(command);
    new_job->state = JOB_STOPPED;  // Default to stopped when re-adding
    new_job->next = NULL;

    // Add to the end of the list
    if (jobs_list == NULL) {
        jobs_list = new_job;
    } else {
        job_t *current = jobs_list;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_job;
    }

    return 0;
}
