#include "shell.h"

typedef struct proc {
  pid_t pid;    /* process identifier */
  int state;    /* RUNNING or STOPPED or FINISHED */
  int exitcode; /* -1 if exit status not yet received */
} proc_t;

typedef struct job {
  pid_t pgid;            /* 0 if slot is free */
  proc_t *proc;          /* array of processes running in as a job */
  struct termios tmodes; /* saved terminal modes */
  int nproc;             /* number of processes */
  int state;             /* changes when live processes have same state */
  char *command;         /* textual representation of command line */
} job_t;

static job_t *jobs = NULL;          /* array of all jobs */
static int njobmax = 1;             /* number of slots in jobs array */
static int tty_fd = -1;             /* controlling terminal file descriptor */
static struct termios shell_tmodes; /* saved shell terminal modes */

static void sigchld_handler(int sig) {
  int old_errno = errno;
  pid_t pid;
  int status;
  /* TODO: Change state (FINISHED, RUNNING, STOPPED) of processes and jobs.
   * Bury all children that finished saving their status in jobs. */
#ifdef STUDENT

  while(true)
  {
    pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED);

    if(pid <= 0) break;

    for (int j = 0; j < njobmax; j++)
    {

      bool is_job_touched = false;

      for (int i = 0; i < jobs[j].nproc; i++) // ma sie od 0 zaczynac zawsze
      {
        if (jobs[j].proc[i].pid != pid)
          continue;

        //fprintf(stderr, "proces '%s' get the signal \n", jobcmd(j));

        //if(WIFSIGNALED(status)) fprintf(stderr, "the signal is %d\n", WTERMSIG(status));

        if (WIFSIGNALED(status) || WIFEXITED(status))
        {
            jobs[j].proc[i].state = FINISHED;
        }
        else if (WIFSTOPPED(status))
          jobs[j].proc[i].state = STOPPED;
        else if(WIFCONTINUED(status))
          jobs[j].proc[i].state = RUNNING;

        jobs[j].proc[i].exitcode = status;
        is_job_touched = true;
      }

      if (is_job_touched)
      {
        //fprintf(stderr, "jobs-pgid: %d \n", jobs[j].pgid);
        int kuku = jobs[j].nproc - 1;
        //fprintf(stderr, "kuku: %d \n", kuku);
        int akuku = jobs[j].proc[kuku].state;
        //fprintf(stderr, "akuku: %d \n", akuku);
        jobs[j].state = akuku;
      }
    }
  }

  (void)status;
  (void)pid;
#endif /* !STUDENT */
  errno = old_errno;
}

/* When pipeline is done, its exitcode is fetched from the last process. */
static int exitcode(job_t *job) {
  return job->proc[job->nproc - 1].exitcode;
}

static int allocjob(void) {
  /* Find empty slot for background job. */
  for (int j = BG; j < njobmax; j++)
    if (jobs[j].pgid == 0)
      return j;

  /* If none found, allocate new one. */
  jobs = realloc(jobs, sizeof(job_t) * (njobmax + 1));
  memset(&jobs[njobmax], 0, sizeof(job_t));
  return njobmax++;
}

static int allocproc(int j) {
  job_t *job = &jobs[j];
  job->proc = realloc(job->proc, sizeof(proc_t) * (job->nproc + 1));
  return job->nproc++;
}

int addjob(pid_t pgid, int bg) {
  int j = bg ? allocjob() : FG;
  job_t *job = &jobs[j];
  /* Initial state of a job. */
  job->pgid = pgid;
  job->state = RUNNING;
  job->command = NULL;
  job->proc = NULL;
  job->nproc = 0;
  job->tmodes = shell_tmodes;
  return j;
}

static void deljob(job_t *job) {
  assert(job->state == FINISHED);
  free(job->command);
  free(job->proc);
  job->pgid = 0;
  job->command = NULL;
  job->proc = NULL;
  job->nproc = 0;
}

static void movejob(int from, int to) {
  assert(jobs[to].pgid == 0);
  memcpy(&jobs[to], &jobs[from], sizeof(job_t));
  memset(&jobs[from], 0, sizeof(job_t));
}

static void mkcommand(char **cmdp, char **argv) {
  if (*cmdp)
    strapp(cmdp, " | ");

  for (strapp(cmdp, *argv++); *argv; argv++) {
    strapp(cmdp, " ");
    strapp(cmdp, *argv);
  }
}

void addproc(int j, pid_t pid, char **argv) {
  assert(j < njobmax);
  job_t *job = &jobs[j];

  int p = allocproc(j);
  proc_t *proc = &job->proc[p];
  /* Initial state of a process. */
  proc->pid = pid;
  proc->state = RUNNING;
  proc->exitcode = -1;
  mkcommand(&job->command, argv);
}

/* Returns job's state.
 * If it's finished, delete it and return exitcode through statusp. */
static int jobstate(int j, int *statusp) {
  assert(j < njobmax);
  job_t *job = &jobs[j];
  int state = job->state;

  /* TODO: Handle case where job has finished. */
#ifdef STUDENT


  if(state == FINISHED)
  {
    *statusp = exitcode(job);
    //deljob(job);
  }
  
  (void)exitcode;
#endif /* !STUDENT */

  return state;
}

char *jobcmd(int j) {
  assert(j < njobmax);
  job_t *job = &jobs[j];
  return job->command;
}

/* Continues a job that has been stopped. If move to foreground was requested,
 * then move the job to foreground and start monitoring it. */
bool resumejob(int j, int bg, sigset_t *mask) {
  if (j < 0) {
    for (j = njobmax - 1; j > 0 && jobs[j].state == FINISHED; j--)
      continue;
  }

  if (j >= njobmax || jobs[j].state == FINISHED)
    return false;

    /* TODO: Continue stopped job. Possibly move job to foreground slot. */
#ifdef STUDENT
  if(!bg)
  {
    //fprintf(stderr, "DEBUG - resume to fg\n");
    if(j!=0) movejob(j,0);

    kill(-jobs[0].pgid, SIGCONT);
    // while(true)
    // {
    //   Sigsuspend(mask);

    //   if(jobs[0].state == RUNNING)break;
    // }

    //watchjobs(RUNNING);
    
    monitorjob(mask);
  }
  else 
  {
    fprintf(stderr, "DEBUG - resume to bg\n");
    kill(-jobs[0].pgid, SIGCONT);
  }

  (void)movejob;
#endif /* !STUDENT */

  return true;
}

/* Kill the job by sending it a SIGTERM. */
bool killjob(int j) {
  if (j >= njobmax || jobs[j].state == FINISHED)
    return false;
  debug("[%d] killing '%s'\n", j, jobs[j].command);

  /* TODO: I love the smell of napalm in the morning. */
#ifdef STUDENT
  kill(-jobs[j].pgid, SIGCONT);
  kill(-jobs[j].pgid, SIGTERM);

#endif /* !STUDENT */

  return true;
}

/* Report state of requested background jobs. Clean up finished jobs. */
void watchjobs(int which) {
  for (int j = BG; j < njobmax; j++) {
    if (jobs[j].pgid == 0)
      continue;

      /* TODO: Report job number, state, command and exit code or signal. */
#ifdef STUDENT

    if(jobs[j].state == which || which == ALL)
    {
      printf("[%d] ", j);

      //printf(stderr, "PGID: %d\n", jobs[j].pgid);
      
      int status = exitcode(&jobs[j]);

      if (jobs[j].state == RUNNING)
      {
        if(WIFCONTINUED(status))
        {
          printf("continue '%s'", jobcmd(j));

        }
        else printf("running '%s'", jobcmd(j));
      }
      else if (jobs[j].state == STOPPED)
      {
        printf("suspended '%s'", jobcmd(j));
      }
      else if (jobs[j].state == FINISHED)
      {
        if (WIFEXITED(status))
          printf("exited '%s', status=%d", jobcmd(j), WEXITSTATUS(status));

        else if (WIFSIGNALED(status))
          printf("killed '%s' by signal %d", jobcmd(j), WTERMSIG(status));
      }

      printf("\n");
    }

    if (jobs[j].state == FINISHED)
    {
      //fprintf(stderr, "deljob: %d \n", j);

      deljob(&jobs[j]);
    }

    (void)deljob;
#endif /* !STUDENT */
  }
}

/* Monitor job execution. If it gets stopped move it to background.
 * When a job has finished or has been stopped move shell to foreground. */
int monitorjob(sigset_t *mask) {
  int exitcode = 0, state;

  /* TODO: Following code requires use of Tcsetpgrp of tty_fd. */
#ifdef STUDENT

  //printf("DEBUG monitorjob\n");
  setfgpgrp(jobs[0].pgid);


  struct termios term;

  Tcgetattr(tty_fd, &term);

  Tcsetattr(tty_fd, TCSADRAIN, &jobs[0].tmodes);


  while(true)
  {

    //fprintf(stderr, "DEBUG monitor while1\n");


    Sigsuspend(mask);

    //fprintf(stderr, "DEBUG monitor while2\n");

    if(jobs[0].state != RUNNING) break;

  }
  
  int st = jobstate(0, &state);

  //pid_t pid = 0;

  if(st == STOPPED) 
  {
    //fprintf(stderr, "DEBUG monitor while");

    //resumejob(-1, BG, mask);
    int i = BG;
    for(; i < njobmax; i++)
    {
      if(jobs[i].pgid == 0)break;
    }
    movejob(0, i); //TODO
    //pid = i;

    exitcode = -1;
  }
  else if(st == FINISHED)
  {
    exitcode = WEXITSTATUS(state);
  }



  Tcgetattr(tty_fd, &jobs[0].tmodes);

  Tcsetattr(tty_fd, TCSADRAIN, &term);

  setfgpgrp(getpgid(0));

  (void)jobstate;
  (void)exitcode;
  (void)state;
#endif /* !STUDENT */

  return exitcode;
}

/* Called just at the beginning of shell's life. */
void initjobs(void) {
  struct sigaction act = {
    .sa_flags = SA_RESTART,
    .sa_handler = sigchld_handler,
  };

  /* Block SIGINT for the duration of `sigchld_handler`
   * in case `sigint_handler` does something crazy like `longjmp`. */
  sigemptyset(&act.sa_mask);
  sigaddset(&act.sa_mask, SIGINT);
  Sigaction(SIGCHLD, &act, NULL);

  jobs = calloc(sizeof(job_t), 1);

  /* Assume we're running in interactive mode, so move us to foreground.
   * Duplicate terminal fd, but do not leak it to subprocesses that execve. */
  assert(isatty(STDIN_FILENO));
  tty_fd = Dup(STDIN_FILENO);
  fcntl(tty_fd, F_SETFD, FD_CLOEXEC);

  /* Take control of the terminal. */
  Tcsetpgrp(tty_fd, getpgrp());

  /* Save default terminal attributes for the shell. */
  Tcgetattr(tty_fd, &shell_tmodes);
}

/* Called just before the shell finishes. */
void shutdownjobs(void) {
  sigset_t mask;
  Sigprocmask(SIG_BLOCK, &sigchld_mask, &mask);

  /* TODO: Kill remaining jobs and wait for them to finish. */
#ifdef STUDENT

//fprintf(stderr, "rzeznia hahah\n");

for (int j = BG; j < njobmax; j++)
{
    if (jobs[j].pgid == 0)
      continue;

    killjob(j);

    Sigprocmask(SIG_SETMASK, &mask, NULL);
    while (true)
    {
      Sigsuspend(&mask);

      //fprintf(stderr, "HAH zlapalem sygnal\n");

      if (jobs[j].state == FINISHED)
        break;
    }
    Sigprocmask(SIG_BLOCK, &sigchld_mask, &mask);
}

//watchjobs(ALL);
#endif /* !STUDENT */

  watchjobs(FINISHED);

  Sigprocmask(SIG_SETMASK, &mask, NULL);

  Close(tty_fd);
}

/* Sets foreground process group to `pgid`. */
void setfgpgrp(pid_t pgid) {
  Tcsetpgrp(tty_fd, pgid);
}
