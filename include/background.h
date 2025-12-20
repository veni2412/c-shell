#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
 * Structure to represent a background job
 */
typedef struct job {
    int job_id;           // Job ID number
    pid_t pid;            // Process ID
    char *command;        // Command string
    int state;            // Job state (running, stopped)
    struct job *next;     // Pointer to next job in list
} job_t;

// Job states
#define JOB_RUNNING 0
#define JOB_STOPPED 1

void init_jobs();
void add_job(pid_t pid, const char *command);
void check_jobs();
void cleanup_jobs();
int redirect_background_stdin();
int is_background_command(char *command);

/**
 * Set the current foreground process PID
 * 
 * @param pid Process ID of the foreground process
 */
void set_foreground_pid(pid_t pid);

/**
 * Set the current foreground command string
 * 
 * @param command Command string of the foreground process
 */
void set_foreground_command(const char *command);

/**
 * Get the current foreground command string
 * 
 * @return Command string of the current foreground process, or NULL if none
 */
const char *get_foreground_command();

/**
 * Get the current foreground process PID
 * 
 * @return Process ID of the current foreground process, or -1 if none
 */
pid_t get_foreground_pid();

/**
 * Stop the foreground process and move it to the background
 * 
 * @param command Command string of the process
 * @return 0 on success, -1 on failure
 */
int stop_foreground_process(const char *command);

/**
 * Kill all child processes spawned by the shell
 */
void kill_all_processes();

/**
 * Get the most recent job ID
 * 
 * @return The most recent job ID, or -1 if no jobs exist
 */
int get_most_recent_job_id();

/**
 * Get the most recent stopped job ID
 * 
 * @return The most recent stopped job ID, or -1 if no stopped jobs exist
 */
int get_most_recent_stopped_job_id();

/**
 * Check if a job with the given ID exists
 * 
 * @param job_id Job ID to check
 * @return 1 if job exists, 0 otherwise
 */
int job_exists(int job_id);

/**
 * Get the details of a job by ID
 * 
 * @param job_id Job ID to look up
 * @param pid Pointer to store the process ID
 * @param command Pointer to store the command string
 * @param state Pointer to store the job state
 * @return 0 on success, -1 if job not found
 */
int get_job_details(int job_id, pid_t *pid, char **command, int *state);

/**
 * Update the state of a job
 * 
 * @param job_id Job ID to update
 * @param state New state (JOB_RUNNING or JOB_STOPPED)
 * @return 0 on success, -1 if job not found
 */
int update_job_state(int job_id, int state);

/**
 * Remove a job from the background jobs list
 * 
 * @param job_id Job ID to remove
 * @return 0 on success, -1 if job not found
 */
int remove_job(int job_id);

/**
 * Add a job with a specific ID to the background jobs list
 * 
 * @param job_id Job ID to use
 * @param pid Process ID
 * @param command Command string
 * @return 0 on success, -1 on failure
 */
int add_job_with_id(int job_id, pid_t pid, const char *command);

#endif