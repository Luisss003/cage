#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <argp.h>
#include <sys/ptrace.h>
int
main(int argc, char **argv)
{
  
  pid_t child_pid;
  int status;
    

  switch(child_pid = fork()) {
    case -1:
      return -1;
    case 0:
      const char* args[] = {"./kitten", "testfile", NULL};
      ptrace(PTRACE_TRACEME,0, NULL, NULL);
      //Stop child so that parent can set up ptrace options
      raise(SIGSTOP);
      execve("./kitten", args, NULL);
      _exit(EXIT_SUCCESS);
    default:
      break;
  }

  //Now the parent must wait for that sigstop from child

  ptrace(PTRACE_ATTACH, child_pid, NULL, NULL);
    
  //Now, we must continue the child UNTIL the next syscall boundary (entering/exiting a syscall).
  waitpid(child_pid, &status, 0);

  if(!WIFSTOPPED(status)){
    fprintf(stderr, "child did not stop\n");
  }

  ptrace(PTRACE_SETOPTIONS, child_pid, NULL, PTRACE_O_TRACESYSGOOD);

  //After resuming, the parent needs to wait again until the next boundary. 
  //Its possible that the parent being unblocked, it wasnt caused by a syscall, but rather exit, signal, or just stopping. Therefore we want ot account for all 3 possibilities.
    
    
}

