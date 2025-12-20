# Compiler and flags
CC = gcc
CFLAGS = -std=c99 \
         -D_POSIX_C_SOURCE=200809L \
         -D_XOPEN_SOURCE=700 \
         -Wall -Wextra -Werror \
         -Wno-unused-parameter \
         -fno-asm \
         -Iinclude

# Source files
SRCS = src/main.c src/prompt.c src/hop.c src/reveal.c src/log.c src/redirection.c src/pipe.c src/sequence.c src/background.c src/activities.c src/ping.c src/shortcuts.c src/job_control.c src/builtins.c src/parser.c

# Output binary
TARGET = shell.out

# Default rule
all: $(TARGET)

# Compile sources into binary
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

# Clean up
clean:
	rm -f $(TARGET)
