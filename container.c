#define _GNU_SOURCE
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mount.h>
#include <stdio.h>
#include <sys/ptrace.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "container.h"
#include "cage.h"
#include "utils.h"
#include "seccomp.h"

#define BUF_MAX 1024

int
container_setup(void *arg)
{
  struct child_args *args = (struct child_args*)arg;
  close(args[0].pipe_fd[1]);
  char buf;
  if(read(args[0].pipe_fd[0], &buf, 1) != 1) die("read sync");
  close(args[0].pipe_fd[0]);


  if(setgid(0) == -1) die("setgid");
  if(setuid(0) == -1) die("setuid");

  if(mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) == -1){
    die("mount");
  }

  char template[] = "/tmp/jail.XXXXXX";
  char * newroot = mkdtemp(template);

  if(mount(newroot, newroot, NULL, MS_BIND | MS_REC, NULL) == -1){
    die("bind newroot to itself");
  }

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

  //Copy executable to container, then within container, make it executable
  char *c_exec_file = malloc(1024);
  snprintf(c_exec_file, 1024, "%s/%s", newroot, args->exec_args[0]);
  copy_file(args->exec_args[0], c_exec_file); 

  //Create oldroot mount point
  char oldroot[1024];
  snprintf(oldroot, sizeof(oldroot), "%s/oldroot", newroot);
  if(mkdir(oldroot, 0755) == -1) die("mkdir oldroot");
  fprintf(stderr, "Oldroot: %s\n", oldroot);
  //After creating bind mounts, isolate file system via pivot
  if(syscall(SYS_pivot_root, newroot, oldroot) == -1) die("pivot");
  chdir("/");
   fprintf(stderr, "My new PID=%d\n", getpid());

  //Now, need to remount proc, since we are going to isolate PID namespace
  if(mkdir("/proc", 0555) == -1) die("mkdir proc");
  if(mount("proc", "/proc", "proc", 0, NULL) == -1) die("mount proc");
 
  //After pivoting, delete oldroot
  if(umount2("oldroot", MNT_DETACH) == -1) die("umount2");
  if(rmdir("oldroot") == -1) die("rmdir");


  if(sethostname("cage", 4) == -1) die("sethostname");

  int allow_list[] = {
    59,   // execve
    12,   // brk
    9,    // mmap
    21,   // access
    257,  // openat
    5,    // fstat
    3,    // close
    0,    // read
    17,   // pread6
    158,  // arch_prctl
    218,  // set_tid_address
    273,  // set_robust_list
    334,  // rseq
    10,   // mprotect
    302,  // prlimit64
    318,  // getrandom
    11,   // munmap
    1,    // write
    231   // exit_group
  };
  setup_seccomp(allow_list, 19 );
  execlp("/bin/sh", "sh" , NULL);
  die("execve");

}

void
copy_file(char *src, char *dst)
{
  fprintf(stderr, "source: %s\ndest: %s\n", src, dst);
  int src_fd, dst_fd;

  if((src_fd = open(src, O_RDONLY)) == -1) die("open original executable");
  if((dst_fd = open(dst, O_CREAT | O_TRUNC | O_RDWR,0111 )) == -1) die("creat container executable");
 
  char buf[1024];
  ssize_t n;

  while((n = read(src_fd, buf, BUF_MAX-1)) > 0){
    ssize_t written = 0;
    while(written < n) {
      ssize_t w = write(dst_fd, buf+written, n-written);
      if(w == -1) die("write exec bytes to container");
      written += w;
    }
  }
  if(n == -1) die("read executable bytes");

  if(close(src_fd) == -1) die("close src_fd");
  if(close(dst_fd) == -1) die("close dst_fd");
}
