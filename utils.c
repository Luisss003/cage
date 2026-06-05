#include <stdio.h>
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
  printf("attempting to write\n");
  if(write(fd, str, strlen(str)) != (ssize_t)strlen(str)) die("write");
  close(fd);
}

void
write_user_files(pid_t pid)
{
  char path[256];
  char map[256];

  snprintf(path, sizeof(path), "/proc/%d/setgroups", pid);
  write_file(path, "deny\n");
  fprintf(stderr, "GOT PAST FIRST WRITE\n");
  snprintf(path, sizeof(path), "/proc/%d/uid_map", pid);
  snprintf(map, sizeof(map), "0 %d 1\n", getuid());
  write_file(path, map);

  snprintf(path, sizeof(path), "/proc/%d/gid_map", pid);
  snprintf(map, sizeof(map), "0 %d 1\n", getgid());
  write_file(path, map);
}

