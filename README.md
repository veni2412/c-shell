# c-shell

This project implements a Unix-like shell in C, strictly using POSIX-compliant system calls and libraries. The shell supports command parsing based on a formally defined context-free grammar, execution of external commands, and a set of built-in shell intrinsics.

Core features include input parsing with syntax validation, foreground and background process execution, I/O redirection, command pipelines, persistent command history, and job control. The implementation adheres to POSIX standards (`_POSIX_C_SOURCE=200809L`, `_XOPEN_SOURCE=700`) and is modularized across multiple `.c` and `.h` files based on functionality.

The shell was built incrementally, starting from prompt handling and parsing, and extending to process control, signal handling, and job management.

Features
- Custom shell prompt with home-directory expansion (`~`)
- Grammar-based input parsing with syntax validation
- Built-in commands: `hop`, `reveal`, `log`, `activities`, `ping`, `fg`, `bg`
- Execution of external commands using `fork`, `exec`, and `wait`
- Input/output redirection (`<`, `>`, `>>`) and command pipelines (`|`)
- Foreground, background (`&`), and sequential (`;`) execution
- Persistent command history across shell sessions
- Job control with signal handling (`SIGINT`, `SIGTSTP`, `SIGCONT`, `SIGKILL`)

Build Instructions
```
bash
make all
```


This generates the executable `shell.out` inside the `shell/` directory.
