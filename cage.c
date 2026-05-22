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
int
main(int argc, char **argv)
{
  pid_t child_pid;
  int status;
  char* args[] = {"./kitten", "testfile", NULL};

  switch(child_pid = fork()) {
    case -1:
      return -1;
    /* We make the child process traceable, pause it to give time for the 
       parent to set up tracking, then start ./kitten */
    case 0:
      ptrace(PTRACE_TRACEME,0, NULL, NULL);
      raise(SIGSTOP);
      execve("./kitten", args, NULL);
      _exit(EXIT_SUCCESS);
    default:
      break;
  }

  //Waits until child is stopped
  waitpid(child_pid, &status, 0);

  //This flag allows for us to distinguish stops from syscalls and usual stops
  ptrace(PTRACE_SETOPTIONS, child_pid, NULL, PTRACE_O_TRACESYSGOOD);

  //This sends SIGCONT to the child, then stops upon the next syscall enter/exit
  ptrace(PTRACE_SYSCALL, child_pid, NULL, NULL);

  //Waits until child exits normally, gets killed by a signal, or stops (either due to a normal signal, or a syscall related signal)
  while(waitpid(child_pid, &status, 0) != -1){
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
        ptrace(PTRACE_GET_SYSCALL_INFO, child_pid,sizeof(sc_info), &sc_info);

        fprintf(stderr, "SYSCALL: %llu\n", sc_info.entry.nr);
        ptrace(PTRACE_SYSCALL, child_pid, NULL, NULL);
      }
      else{
        printf("Something stopped\n");
        fflush(stdout);
        ptrace(PTRACE_SYSCALL, child_pid, NULL, NULL);
      }
    }
  }
}
