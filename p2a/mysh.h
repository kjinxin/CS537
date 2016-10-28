#ifndef __MYSH_H__
#define __MYSH_H__
#include "mysh.h"
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#define MODEL_INTER 0
#define MODEL_BATCH 1
#define MODEL_FORE 2
#define MODEL_BACK 3
#define JOB_VALID 1
#define JOB_INVALID 0
#define MAX_LEN 512
#define JOB_LEN 32

int jobid = 1;

typedef struct _job {
  unsigned int pid;
  unsigned int jid;
  char name[514];
  unsigned int valid;
} Job, *job;

Job jobqueue[32];

void initJobList();
void showJobList();
void waitJob(char *jobid);
void addProcess(char *argv[], int model, int argc);
void removeJob();
#endif
