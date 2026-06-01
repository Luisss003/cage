#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mount.h>
#include <stdio.h>
#include <sys/ptrace.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "utils.h"

void
container_setup(void *arg)
{

  //Prevent mount propogation
  if(mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) == -1){
    die("mount");
  }

  //Create a temp dir to use as mount point
  char template[] = "/tmp/jail.XXXXXX";
  char * newroot = mkdtemp(template);

  //Then make the new dir a mount point itself to later be used by pivot
  if(mount(newroot, newroot, NULL, MS_BIND | MS_REC, NULL) == -1){
    die("bind newroot to itself");
  }

  //Safely bind mount required libraries
  char *bind_dir[] = {"/bin", "/usr", "/lib", "/lib64"};
  for (int i = 0; i < 4; i++){
    char *temp_dir = malloc(1024);
    snprintf(temp_dir, 1024, "%s%s", newroot, bind_dir[i]);
    if(mkdir(temp_dir, 0755) == -1){
      die("mkdir");
    }

    if(mount(bind_dir[i], temp_dir, NULL, MS_BIND | MS_REC, NULL) == -1){
      die("mount bind");
    }

    //Remount for readonly perms
    if(mount(NULL, temp_dir, NULL, MS_BIND | MS_REC | MS_RDONLY | MS_REMOUNT, NULL) == -1) die("mount rebind");
  }

  //Create oldroot mount point
  char oldroot[1024];
  snprintf(oldroot, sizeof(oldroot), "%s/oldroot", newroot);
  if(mkdir(oldroot, 0755) == -1) die("mkdir oldroot");
  fprintf(stderr, "Oldroot: %s\n", oldroot);

  //After creating bind mounts, isolate file system via pivot
  if(syscall(SYS_pivot_root, newroot, oldroot) == -1) die("pivot");
  chdir("/");
  
  //After pivoting, delete oldroot
  if(umount2("oldroot", MNT_DETACH) == -1) die("umount2");
  if(rmdir("oldroot") == -1) die("rmdir");

  //Now, need to remount proc, since we are going to isolate PID namespace
  if(mkdir("/proc", 0555) == -1) die("mkdir proc");
  if(mount("proc", "/proc", "proc", 0, NULL) == -1) die("mount proc");

  if(sethostname("cage", 4) == -1) die("sethostname");
  execlp("/bin/sh", "sh", NULL);
  die("execve");

}
