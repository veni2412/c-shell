#ifndef SHORTCUTS_H
#define SHORTCUTS_H

#include <sys/types.h>

void init_shortcuts(void);
void sigint_handler(int sig);
void sigtstp_handler(int sig);
void handle_eof(void);

#endif