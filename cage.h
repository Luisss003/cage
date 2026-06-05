#ifndef CAGE_H
#define CAGE_H
#include <unistd.h>
void trace_syscalls(pid_t, char **);
void detect_file_io(struct ptrace_syscall_info*);

struct child_args{
  int pipe_fd[2];
  char **exec_args;
};
#endif
