#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <argp.h>
#include <sys/ptrace.h>
#include <linux/ptrace.h>
#include "utils.h"
#include <string.h>
#include "cage.h"
#include "utils.h"
#include "container.h"

//Eventually replace with cmd line arg
#define STACK_SIZE (1024 * 1024)

int
main(int argc, char **argv)
{

  struct child_args args;
  if(pipe(args.pipe_fd) == -1) die("pipe");
  char *exec_args[] = {"kitten", "testfile", NULL};
  args.exec_args = exec_args;

  char *stack;
  char *stackTop;
  int s, flags;

  flags = CLONE_NEWNET | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUTS | SIGCHLD | CLONE_NEWUSER;

  stack = malloc(STACK_SIZE);
  if(stack == NULL){
    die("malloc");
  }
  stackTop = stack + STACK_SIZE;

  int child;
  

  child = clone(container_setup, stackTop, flags, &args);
  if (child == -1) die("clone");
  close(args.pipe_fd[0]);

  write_user_files(child);

  if(write(args.pipe_fd[1], "x", 1) != 1) die("write sync");
  close(args.pipe_fd[1]);

  //trace_syscalls(child, args);
  waitpid(child, NULL, 0);  
  return 0;
}
void 
trace_syscalls(pid_t child, char *args[])
{
  int status;
  //Waits until child is stopped
  waitpid(child, &status, 0);

  //This flag allows for us to distinguish stops from syscalls and usual stops
  ptrace(PTRACE_SETOPTIONS, child, NULL, PTRACE_O_TRACESYSGOOD);

  //This sends SIGCONT to the child, then stops upon the next syscall enter/exit
  ptrace(PTRACE_SYSCALL, child, NULL, NULL);

  //Waits until child exits normally, gets killed by a signal, or stops (either due to a normal signal, or a syscall related signal)
  while(waitpid(child, &status, 0) != -1){
    if(WIFEXITED(status)){
      printf("The child exited\n");
      fflush(stdout);
      exit(EXIT_SUCCESS);
    }
    if(WIFSIGNALED(status)){
      printf("Killed by a signal\n");
      fflush(stdout);
      exit(EXIT_SUCCESS);
    }
    if(WIFSTOPPED(status)){
      int sig = WSTOPSIG(status);
      if(sig == (SIGTRAP | 0x80)){
        struct ptrace_syscall_info sc_info = {0};
        ptrace(PTRACE_GET_SYSCALL_INFO, child,sizeof(sc_info), &sc_info);

        print_syscall(sc_info.entry.nr);
        if (sc_info.entry.nr == 0 || sc_info.entry.nr == 1 || sc_info.entry.nr == 3){
          detect_file_io(&sc_info); 
        }
        ptrace(PTRACE_SYSCALL, child, NULL, NULL);
      }
      else{
        printf("Something stopped\n");
        fflush(stdout);
        ptrace(PTRACE_SYSCALL, child, NULL, NULL);
      }
    }
  }
}

void 
detect_file_io(struct ptrace_syscall_info* sc_info)
{
  if (sc_info->entry.nr == 0) {
    //TODO: write func that reads /proc/ of child to see file name assocaited with fd
    
    fprintf(stderr, "[rdi] File Desriptor: %llu\n", sc_info->entry.args[0]); 
    fprintf(stderr, "[rsi] Read bytes to: %p\n", sc_info->entry.args[1]);
    fprintf(stderr, "[rdx] Read %llu bytes\n", sc_info->entry.args[2]);
 }
  else if(sc_info->entry.nr == 1){
    if (!(sc_info->entry.args[0] == 0 || sc_info->entry.args[0] == 1 ||
       sc_info->entry.args[0] == 1)){
      fprintf(stderr, "A FILE WAS CHANGED\n");
    }
    fprintf(stderr, "[rdi] Wrote to fd: %llu\n", sc_info->entry.args[0]); 
    fprintf(stderr, "[rsi] Wrote bytes from: %p\n", sc_info->entry.args[1]);
    fprintf(stderr, "[rdx] Wrote %llu bytes\n", sc_info->entry.args[2]);
 
  }
}

