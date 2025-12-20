#include "reveal.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <linux/limits.h>
#include "prompt.h"

static char prev_cwd[PATH_MAX] = {0};

int cmp_strings(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void update_reveal_prev_cwd() {
    char directory[PATH_MAX];
    if (getcwd(directory, sizeof(directory)) != NULL) {
        strncpy(prev_cwd, directory, sizeof(prev_cwd));
        prev_cwd[sizeof(prev_cwd) - 1] = '\0';
    }
}

void reveal_command(int a, int l, const char *arg) {
    char directory[PATH_MAX];

    if (!arg || strcmp(arg, ".") == 0) {
        if (!getcwd(directory, sizeof(directory))) {
            perror("getcwd failed");
            return;
        }
    }
    else if (strcmp(arg, "~") == 0) {
        strncpy(directory, get_shell_home(), sizeof(directory));
        directory[sizeof(directory)-1] = '\0';
    }
    else if (strcmp(arg, "..") == 0) {
        strcpy(directory, "..");
    }
    else if (strcmp(arg, "-") == 0) {
        if (prev_cwd[0] == '\0') {
            printf("No such directory!\n");
            return;
        }
        strncpy(directory, prev_cwd, sizeof(directory));
        directory[sizeof(directory)-1] = '\0';
    }
    else {
        strncpy(directory, arg, sizeof(directory));
        directory[sizeof(directory)-1] = '\0';
    }

    DIR *dir = opendir(directory);
    if (!dir) {
        printf("No such directory!\n");
        return;
    }

    char *entries[1024];
    int count = 0;
    struct dirent *de;
    while ((de = readdir(dir))) {
        if (!a && de->d_name[0] == '.') continue;
        entries[count++] = strdup(de->d_name);
    }
    closedir(dir);

    qsort(entries, count, sizeof(char *), cmp_strings);

    for (int i = 0; i < count; i++) {
        if (l) printf("%s\n", entries[i]);
        else printf("%s ", entries[i]);
        free(entries[i]);
    }
    if (!l) printf("\n");
}
