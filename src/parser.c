// llm generated code

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "parser.h"

#define MAX_TOKENS 1024
#define MAX_TOKEN_LENGTH 256

static const char *input;
static int pos;

void init_shell_parser(const char *str) {
    input = str;
    pos = 0;
}

static void skip_whitespace() {
    while (input[pos] && isspace(input[pos])) {
        pos++;
    }
}

// Tokenize the input string into an array of tokens
char **tokenize_command(const char *input_str, int *count) {
    const char *special_chars = "|&<>;";
    char **result = malloc(MAX_TOKENS * sizeof(char*));
    *count = 0;
    
    const char *p = input_str;
    char token[MAX_TOKEN_LENGTH];
    int token_pos = 0;
    
    while (*p != '\0' && *count < MAX_TOKENS - 1) {
        // Skip whitespace
        while (isspace(*p)) p++;
        if (*p == '\0') break;
        
        // Handle quoted strings
        if (*p == '\"' || *p == '\'') {
            char quote = *p;
            p++; // Skip the opening quote
            
            token_pos = 0;
            while (*p != '\0' && *p != quote) {
                token[token_pos++] = *p++;
                if (token_pos >= MAX_TOKEN_LENGTH - 1) break;
            }
            if (*p == quote) p++; // Skip the closing quote
            
            token[token_pos] = '\0';
            result[*count] = strdup(token);
            (*count)++;
            continue;
        }
        
        // Handle special operators
        if (strchr(special_chars, *p) != NULL) {
            token_pos = 0;
            token[token_pos++] = *p++;
            
            // Handle >> as a single token
            if (token[0] == '>' && *p == '>') {
                token[token_pos++] = *p++;
            }
            
            token[token_pos] = '\0';
            result[*count] = strdup(token);
            (*count)++;
            continue;
        }
        
        // Handle regular words
        token_pos = 0;
        while (*p != '\0' && !isspace(*p) && strchr(special_chars, *p) == NULL) {
            token[token_pos++] = *p++;
            if (token_pos >= MAX_TOKEN_LENGTH - 1) break;
        }
        
        token[token_pos] = '\0';
        result[*count] = strdup(token);
        (*count)++;
    }
    
    result[*count] = NULL;
    return result;
}

// Free the memory allocated for tokens
void free_tokens(char **tokens, int count) {
    for (int i = 0; i < count; i++) {
        free(tokens[i]);
    }
    free(tokens);
}

static int match_char(char c) {
    skip_whitespace();
    if (input[pos] == c) {
        pos++;
        return 1;
    }
    return 0;
}

static int parse_name() {
    skip_whitespace();

    int start = pos;
    while (input[pos] && strchr("|&<>;", input[pos]) == NULL && !isspace(input[pos])) {
        pos++;
    }

    return pos > start;
}

static int parse_input() {
    skip_whitespace();

    if (match_char('<')) {
        return parse_name();
    }

    return 0;
}

static int parse_output() {
    skip_whitespace();

    if (match_char('>')) {
        if (match_char('>')) {}
        return parse_name();
    }

    return 0;
}

static int parse_atomic() {
    if (!parse_name()) {
        return 0;
    }

    while (1) {
        if (parse_name() || parse_input() || parse_output()) {
            continue;
        }
        break;
    }

    return 1;
}

static int parse_cmd_group() {
    if (!parse_atomic()) {
        return 0;
    }

    while (1) {
        skip_whitespace();
        if (match_char('|')) {
            if (!parse_atomic()) {
                return 0;
            }
        }
        else {
            break;
        }
    }

    return 1;
}

int parse_cmd_shell() {
    if (!parse_cmd_group()) {
        return 0;
    }

    while (1) {
        skip_whitespace();
        if (match_char('&') || match_char(';')) {
            if (!parse_cmd_group()) {
                return 0;
            }
        }
        else {
            break;
        }
    }

    skip_whitespace();
    match_char('&');

    return input[pos] == '\0';
}

// Split a command string into pipeline stages
int split_pipeline_commands(const char *cmd, char **parts) {
    int count = 0;
    char *cmd_copy = strdup(cmd);
    char *token, *saveptr;
    
    // Split on pipe character
    token = strtok_r(cmd_copy, "|", &saveptr);
    while (token != NULL && count < 127) {
        // Trim leading whitespace
        while (*token && isspace(*token)) token++;
        
        // Trim trailing whitespace
        int len = strlen(token);
        while (len > 0 && isspace(token[len - 1])) {
            token[--len] = '\0';
        }
        
        // Add to parts if non-empty
        if (*token) {
            parts[count++] = strdup(token);
        }
        
        token = strtok_r(NULL, "|", &saveptr);
    }
    
    parts[count] = NULL;
    free(cmd_copy);
    return count;
}

// llm generated code ends here