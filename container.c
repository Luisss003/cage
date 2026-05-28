#include <sys/mount.h>
#include <sys/ptrace.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "utils.h"

void
container_setup(void *arg)
{
  ptrace(PTRACE_TRACEME, 0, NULL, NULL);
  raise(SIGSTOP);
  mode_t mode;
  //setup container junk
  //First, create a new directory to act as new root
  
  char template[] = "/tmp/jail.XXXXXX";
  char * root = mkdtemp(template);

  //Then make the new dir a mount point itself to later be used by pivot
  if(mount(


  if(mkdirat(dir, "bin", mode) == -1){
    die("mkdirat");
  }

  if(mkdirat(dir, "lib", mode) == -1){
    die("mkdirat");
  }

  if(mkdirat(dir, "lib64", mode) == -1){
    die("mkdirat");
  }

  if(mkdirat(dir, "proc", mode) == -1){
    die("mkdirat");
  }

  if(mkdirat(dir, "usr", mode) == -1){
    die("mkdirat");
  }

  if(mkdirat(dir, "oldroot", mode) == -1){
    die("mkdirat");
  }


}
