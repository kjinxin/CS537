#include "mysh.h"
/*
 * initialize the job queue
 */
void initJobList() {
  int i;
  for (i = 0; i < JOB_LEN; i++) {
    jobqueue[i].valid = JOB_INVALID;
    jobqueue[i].jid = jobid;
  }
}

/*
* qsort compare function
*/
int cmp(const void *a, const void *b) {
  return ((job) a)->jid - ((job) b)->jid;
}

/*
 * to do the command J to show the running jobs
 */
void showJobList() {
  int i;
  removeJob();
  qsort(jobqueue, JOB_LEN, sizeof(Job), cmp);
  for (i = 0; i < JOB_LEN; i++) {
    if (jobqueue[i].valid == JOB_VALID) {
      char buffer[MAX_LEN + 2];
      sprintf(buffer, "%d : %s\n", jobqueue[i].jid, jobqueue[i].name);
      write(1, buffer, strlen(buffer));
    }
  }
}

/*
 * wait for a job to terminate
 */
void waitJob(char *jobidstr) {
  int jid = atoi(jobidstr);
  int i, status;
  char buffer[MAX_LEN + 2];
  for (i = 0; i < JOB_LEN; i++) {
    if (jobqueue[i].jid == jid) {
      if (jobqueue[i].valid == JOB_VALID) {
        // calculate waiting time;
        struct timeval tvs, tvf;
        struct timezone tzs, tzf;
        gettimeofday(&tvs, &tzs);
        waitpid(jobqueue[i].pid, &status, 0);
        gettimeofday(&tvf, &tzf);
        sprintf(buffer, "%ld : Job %s terminated\n",
                (tvf.tv_sec - tvs.tv_sec) * 1000000 + tvf.tv_usec -
                    tvs.tv_usec, jobidstr);
      } else {
        sprintf(buffer, "%d : Job %s terminated\n", 0, jobidstr);
      }
      write(1, buffer, strlen(buffer));
      return;
    }
  }
  if (jid >= 1 && jid < jobid) {
    sprintf(buffer, "%d : Job %s terminated\n", 0, jobidstr);
    write(1, buffer, strlen(buffer));
    return;
  }
  // invalid Jid
  sprintf(buffer, "Invalid jid %s\n", jobidstr);
  write(2, buffer, strlen(buffer));
}

/*
 * to add a new process
 */
void addProcess(char *argv[], int model, int argc) {
  int jobspace = -1;
  int i;
  for (i = 0; i < JOB_LEN; i++) {
    if (jobqueue[i].valid == JOB_INVALID) {
      jobspace = i;
      break;
    }
  }
  if (jobspace == -1 && model == MODEL_BACK) {
    return;
  }
  int rc = fork();
  if (rc == 0) {
    execvp(argv[0], argv);
    // error handling
    char buffer[MAX_LEN + 2];
    sprintf(buffer, "%s: Command not found\n", argv[0]);
    write(2, buffer, strlen(buffer));
    exit(1);
  } else if (rc > 0) {
    if (model == MODEL_FORE) {
      int status;
      jobid++;
      waitpid(rc, &status, 0);
    } else {
      //  add to the job queue
      jobqueue[jobspace].pid = rc;
      jobqueue[jobspace].jid = jobid++;
      int count = 0;
      for (i = 0; i < strlen(argv[0]); i++) {
        jobqueue[jobspace].name[count++] = argv[0][i];
      }
      for (i = 1; i < argc; i++) {
        int j = 0;
        jobqueue[jobspace].name[count++] = ' ';
        for (j = 0; j < strlen(argv[i]); j++) {
          jobqueue[jobspace].name[count++] = argv[i][j];
        }
      }
      jobqueue[jobspace].name[count] = '\0';
      jobqueue[jobspace].valid = JOB_VALID;
    }
  } else {
    // todo error handling
  }
}
/*
*remove the job that is not running
*/
void removeJob() {
  int status = 0, i, rc;
  for (i = 0; i < JOB_LEN; i++) {
    if (jobqueue[i].valid == JOB_VALID) {
      rc = waitpid(jobqueue[i].pid, &status, WNOHANG);
      if (rc != 0) {
        jobqueue[i].valid = JOB_INVALID;
      }
    }
  }
}
int main(int argc, char *argv[]) {
  FILE *fp = stdin;
  int model = MODEL_INTER;
  char str[MAX_LEN + 2];
  if (argc > 2) {
    // todo error handling
    sprintf(str, "Usage: mysh [batchFile]\n");
    write(2, str, strlen(str));
    exit(1);
  }
  if (argc > 1) {
    model = MODEL_BATCH;
    fp = fopen(argv[1], "r");
    if (fp == NULL) {
      free(fp);
      sprintf(str, "Error: Cannot open file %s\n", argv[1]);
      write(2, str, strlen(str));
      exit(1);
    }
  }
  initJobList();
  while (1) {
    removeJob();
    if (model == MODEL_INTER) {
      write(1, "mysh> ", 6);
    }
    if (fgets(str, MAX_LEN + 2, fp) != NULL) {
      if (strlen(str) > MAX_LEN && str[MAX_LEN] != '\n') {
         if (model == MODEL_BATCH) {
           write(1, str, 512);
           write(1, "\n", 1);
         }
         while (strlen(str) >= MAX_LEN) {
           if (str[MAX_LEN - 1] == '\n') break;
           if (fgets(str, MAX_LEN + 1, fp) == NULL) {
             fclose(fp);
             exit(0);
           }
         }
         continue;
      } else {
        if (model == MODEL_BATCH) {
          write(1, str, strlen(str));
        }
      }
      if (model == MODEL_INTER && str[strlen(str) - 1] != '\n') {
        continue;
      }
      if (str[strlen(str) - 1] == '\n') {
        str[strlen(str) - 1] = '\0';
      }
      if (strlen(str) == 0)
        continue;
      char *token = strtok(str, " \t");
      char *jobargv[MAX_LEN + 2];
      int modelbf = MODEL_FORE;
      int paranum = 0;
      while (token != NULL) {
        if (!strcmp(token, "&")) {
            modelbf = MODEL_BACK;
            break;
        }
        if (token[strlen(token) - 1] == '&') {
          modelbf = MODEL_BACK;
          token[strlen(token) - 1] = '\0';
          jobargv[paranum++] = strdup(token);
          break;
        }
        jobargv[paranum++] = strdup(token);
        token = strtok(NULL, " \t");
      }
      jobargv[paranum] = NULL;
      if (paranum == 2 && !strcmp(jobargv[0], "myw")) {
        waitJob(jobargv[1]);
      } else if (paranum == 1 && !strcmp(jobargv[0], "j")) {
        showJobList();
      } else if (paranum == 1 && !strcmp(jobargv[0], "exit")) {
        break;
      } else if (paranum > 0) {
        addProcess(jobargv, modelbf, paranum);
      }
      int i = 0;
      for (i = 0; i < paranum; i++) {
        free(jobargv[i]);
      }

    } else {
      // todo error handling
      fclose(fp);
      exit(0);  // write(1, "\n", 1);
    }
  }
  fclose(fp);
  return 0;
}
