#include "prompt.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include <linux/limits.h>

// ############## LLM Generated Code Begins ##############

static char shell_home_dir[PATH_MAX];   // now static, private to this file

void initialize_shell_home(void) {
    if (!getcwd(shell_home_dir, sizeof(shell_home_dir))) {
        struct passwd *pw = getpwuid(getuid());
        if (pw && pw->pw_dir) {
            strncpy(shell_home_dir, pw->pw_dir, sizeof(shell_home_dir) - 1);
            shell_home_dir[sizeof(shell_home_dir) - 1] = '\0';
        } else {
            shell_home_dir[0] = '\0';
        }
    }
}

const char *get_shell_home(void) {
    return shell_home_dir;
}

// ############## LLM Generated Code Ends ################

void print_prompt(void) {
    char hostname[1024];
    char cwd[PATH_MAX];
    char display_path[PATH_MAX];

    struct passwd *pw = getpwuid(getuid());
    const char *username;

    if (pw && pw->pw_name) {
        username = pw->pw_name;
    } 
    else {
        username = "unknown";
    }

    if (gethostname(hostname, sizeof(hostname)) != 0) {
        strcpy(hostname, "unknown");
    }

    if (!getcwd(cwd, sizeof(cwd))) {
        strcpy(cwd, "?");
    }

    if (shell_home_dir[0] != '\0' && strncmp(cwd, shell_home_dir, strlen(shell_home_dir)) == 0) {
        snprintf(display_path, sizeof(display_path), "~%s", cwd + strlen(shell_home_dir));
    } 
    else {
        snprintf(display_path, sizeof(display_path), "%s", cwd);
    }

    printf("<%s@%s:%s> ", username, hostname, display_path);
    fflush(stdout);
}