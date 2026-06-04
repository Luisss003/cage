#include <stdio.h>
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

