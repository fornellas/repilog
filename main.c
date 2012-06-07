#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

// Failsafe in case of non GNU Libc
#ifndef TEMP_FAILURE_RETRY
# define TEMP_FAILURE_RETRY(expression) \
  (__extension__                                                              \
    ({ long int __result;                                                     \
       do __result = (long int) (expression);                                 \
       while (__result == -1L && errno == EINTR);                             \
       __result; }))
#endif

#define BUFFSIZE 4096

int logfd;
char *logpath;

// open log file
void
openlog(){
  if(-1==(logfd=TEMP_FAILURE_RETRY(open(logpath, O_WRONLY|O_APPEND|O_CREAT, 0644)))){
    perror("Unable to open log file");
    exit(1);
  }
}

// Reopen log file on SIGUSR1
void
reopenlog (int sig) {
  if(TEMP_FAILURE_RETRY(close(logfd))){
    perror("Error closing log file");
    exit(1);
  }
  openlog();
}

// main
int
main (int argc, char ** argv) {
  char buffer[BUFFSIZE];
  int r;
  sigset_t s;

  // command line
  if(2!=argc){
    fprintf(stderr, "Usage: %s [log file]\n", argv[0]);
    exit(1);
  }
  logpath=argv[1];

  // open log
  openlog();

  // setup signal
  if(SIG_ERR==signal(SIGUSR1, reopenlog)){
    fprintf(stderr, "Error caling signal(2).\n");
    exit(1);
  };
  sigemptyset(&s);
  sigaddset(&s, SIGUSR1);

  // read data
  for(;;) {
    int r;
    r=TEMP_FAILURE_RETRY(read(0, (void *)buffer, BUFFSIZE));
    // data read
    if(r>0){
      int w;

      // write with signal delayed
      sigprocmask(SIG_BLOCK, &s, NULL);
      w=TEMP_FAILURE_RETRY(write(logfd, (const void *)buffer, r));
      sigprocmask(SIG_UNBLOCK, &s, NULL);
      if(-1==w){
        perror("Error writing to log file");
        exit(1);
      }
      if(r!=w){
        fprintf(stderr, "Warning: Error writing to log file: expected %d size write, got only %d.\n", w, r);
      }
    // EOF
    } else if(0==r) {
      exit(0);
    // error reading
    } else if(-1==r){
      perror("Error reading stdin");
      exit(1);
    }
  }
}
