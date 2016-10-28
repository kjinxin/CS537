#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#define check(exp, msg) if(exp) {} else {\
  printf(1, "%s:%d check (" #exp ") failed: %s\n", __FILE__, __LINE__, msg);\
  exit();}

int long_workload(int n) {
  int i, j = 0;
  for(i = 0; i < n; i++) {
    j += i * j + 1;
  }
  return j;
}

int
main(int argc, char *argv[])
{
  struct pstat st;

  sleep(10);

  printf(1, "begin testing\n");

  int i = long_workload(50000000), j;
  check(getpinfo(&st) == 0, "getpinfo");

  printf(1, "end testing\n");

  int pid = getpid(); 
  for(i = 0; i < NPROC; i++) {
    if (st.inuse[i]) {
      printf(1, "pid: %d\n", st.pid[i]);
      if (pid == st.pid[i]) {
        // printf(1, "pid: %d priority: %d\n ", st.pid[i], st.priority[i]);
        for (j = 0; j < 4; j++) {
          printf(1, "\t level %d ticks used %d\n", j, st.ticks[i][j]);  
        }

        check(st.ticks[i][0] == 5,
            "current process should run for 5 ticks in the first level");
        check(st.ticks[i][1] == 5,
            "current process should run for 5 ticks in the second level");
        check(st.ticks[i][2] == 10,
            "current process should run for 10 ticks in the third level");
        check(st.ticks[i][3] > 0,
            "current process should run for at least 1 tick in the lowest level");
        check(st.priority[i] == 3, "current process should have the lowest priority");
      } else {
        for (j = 0; j < 4; j++) {
          printf(1, "\t level %d ticks used %d\n", j, st.ticks[i][j]);  
        }
      }
    }
  }

  printf(1, "TEST PASSED");
  exit();
}
