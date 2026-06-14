#include <stdio.h>
#include <sys/syscall.h>
#include <string.h>
#include <fcntl.h>
#include "utils.h"
#include <unistd.h>
#include <stdlib.h>

void 
print_syscall(int n)
{
  fprintf(stderr, "--------------------------\n"); 
  switch(n){
    case 0:
      fprintf(stderr, "SYSCALL: read\n");
      break;
    case 1:
      fprintf(stderr, "SYSCALL: write\n");
      break;
    case 2:
      fprintf(stderr, "SYSCALL: open\n");
      break;
    default:
      fprintf(stderr, "UNKNOWN SYSCALL\n");
      break;
  }
  fprintf(stderr, "--------------------------\n"); 

}

long
syscall_lookup(char *s)
{
  if(strcmp(s, "read") == 0) return SYS_read; 
}

void
die(const char *msg)
{
  perror(msg);
  exit(EXIT_FAILURE);
}

void
print_usage()
{
  fprintf(stderr, "USAGE: WRONG");
}

void
write_file(char *path, char *str)
{
  int fd;
  if((fd = open(path, O_WRONLY)) == -1) die("open");
  if(write(fd, str, strlen(str)) != (ssize_t)strlen(str)) die("write");
  close(fd);
}

void
write_user_files(pid_t pid)
{
    char path[256];
    char map[256];

    uid_t outside_uid = getuid();
    gid_t outside_gid = getgid();

    char *sudo_uid = getenv("SUDO_UID");
    char *sudo_gid = getenv("SUDO_GID");

    if (sudo_uid != NULL)
        outside_uid = atoi(sudo_uid);

    if (sudo_gid != NULL)
        outside_gid = atoi(sudo_gid);

    snprintf(path, sizeof(path), "/proc/%d/setgroups", pid);
    write_file(path, "deny\n");

    snprintf(path, sizeof(path), "/proc/%d/uid_map", pid);
    snprintf(map, sizeof(map), "0 %d 1\n", outside_uid);
    write_file(path, map);

    snprintf(path, sizeof(path), "/proc/%d/gid_map", pid);
    snprintf(map, sizeof(map), "0 %d 1\n", outside_gid);
    write_file(path, map);
}

