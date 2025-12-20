#include "builtins.h"
#include "hop.h"
#include "reveal.h"
#include "log.h"
#include "job_control.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

bool is_builtin(const char *cmd) {
    if (!cmd) return false;
    
    return (strcmp(cmd, "hop") == 0 ||
            strcmp(cmd, "reveal") == 0 ||
            strcmp(cmd, "log") == 0 ||
            strcmp(cmd, "fg") == 0 ||
            strcmp(cmd, "bg") == 0 ||
            strcmp(cmd, "exit") == 0);
}

int run_builtin(int argc, char **argv) {
    if (!argv || !argv[0]) return 1;
    
    if (strcmp(argv[0], "hop") == 0) {
        // hop [dir]
        if (argc > 1) {
            hop_command(argv[1]);
        } else {
            hop_command(NULL);
        }
        return 0;
    } 
    else if (strcmp(argv[0], "reveal") == 0) {
        int a = 0, l = 0;
        char *path = NULL;
        
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-a") == 0) {
                a = 1;
            } else if (strcmp(argv[i], "-l") == 0) {
                l = 1;
            } else if (!path) {
                path = argv[i];
            } else {
                fprintf(stderr, "reveal: too many arguments\n");
                return 1;
            }
        }
        
        // Update previous working directory
        update_reveal_prev_cwd();
        
        // Call reveal with arguments
        reveal_command(a, l, path);
        return 0;
    }
    else if (strcmp(argv[0], "log") == 0) {
        if (argc == 1) {
            print_log();
            return 0;
        }
        else if (argc >= 2 && strcmp(argv[1], "purge") == 0) {
            purge_log();
            return 0;
        }
        else if (argc >= 3 && strcmp(argv[1], "execute") == 0) {
            int index_int = atoi(argv[2]);
            char *log_cmd = get_log_command(index_int);
            
            if (!log_cmd) {
                fprintf(stderr, "log: Invalid index!\n");
                return 1;
            }
            
            printf("%s\n", log_cmd);
            return 0;
        } else {
            fprintf(stderr, "log: Unknown subcommand: %s\n", argv[1]);
            return 1;
        }
    }
    else if (strcmp(argv[0], "fg") == 0) {
        char *job_number = (argc > 1) ? argv[1] : NULL;
        return fg_command(job_number);
    }
    else if (strcmp(argv[0], "bg") == 0) {
        char *job_number = (argc > 1) ? argv[1] : NULL;
        return bg_command(job_number);
    }
    else if (strcmp(argv[0], "exit") == 0) {
        exit(0);
    }
    
    return 1;
}
