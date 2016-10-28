#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int value_st, value_ed;

  if(argc <= 1){
    printf(1, "usage: syscallptest N\n");        
    exit();
  }
  value_st = getnumsyscallp();
  int i;
  for (i = 0; i < atoi(argv[1]); i ++) {
    getpid();
  }
  value_ed = getnumsyscallp();
  printf(1, "%d %d", value_st, value_ed); 
  exit();
}
