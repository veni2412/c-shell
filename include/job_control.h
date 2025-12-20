#ifndef JOB_CONTROL_H
#define JOB_CONTROL_H

/**
 * Bring a background or stopped job to the foreground
 * 
 * If the job is stopped, send SIGCONT to resume it
 * If no job number is provided, use the most recently created background/stopped job
 * 
 * @param job_number_str String representation of the job number, or NULL for most recent
 * @return 0 on success, -1 on failure
 */
int fg_command(const char *job_number_str);

/**
 * Resume a stopped background job
 * 
 * Send SIGCONT to the job and let it continue running in the background
 * 
 * @param job_number_str String representation of the job number, or NULL for most recent
 * @return 0 on success, -1 on failure
 */
int bg_command(const char *job_number_str);

#endif /* JOB_CONTROL_H */
