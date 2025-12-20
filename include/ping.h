#ifndef PING_H
#define PING_H

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
int ping_command(const char *pid_str, const char *signal_str);

#endif /* PING_H */
