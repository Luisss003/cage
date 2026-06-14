#include <stdio.h>
#include <seccomp.h>
#include "utils.h"
#include "seccomp.h"
#include <errno.h>
void
setup_seccomp(int allow_list[], size_t size) 
{
  scmp_filter_ctx ctx;

  //Default Action: If unallowed syscall, kill container.
  if((ctx =  seccomp_init(SCMP_ACT_ALLOW)) == NULL) die("seccomp_init");

  
  for(size_t i = 0; i < size; i++){
    if(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, allow_list[i], 0) == -1) die("seccomp_rule_add");
  }
  int status;
  if((status = seccomp_load(ctx)) != 0) die("seccomp_load");
}
