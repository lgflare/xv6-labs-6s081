#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int ptoc[2], ctop[2]; // [0] for reading and [1] for writing
char buf[10];

int
mian() {
  int pid;

  if (pipe(ptoc) < 0) {
    fprintf(2, "pipe error\n");
    exit(1);
  }
  if (pipe(ctop) < 0) {
    fprintf(2, "pipe error\n");
    exit(1);
  }

  pid = fork();
  if (pid < 0) {
    fprintf(2, "fork error\n");
    exit(1);
  }  
  else if (pid == 0) {
    // child process
    // child recieve ping
    close(ptoc[1]);
    read(ptoc[0], buf, 1);
    fprintf(1, "%d: received ping\n", getpid());
    // child send pong
    close(ctop[0]);
    write(ctop[1], "c", 1);
  }
  else if (pid > 0) {
    // parent process
    // parent send ping
    close(ptoc[0]);
    write(ptoc[1], "p", 1);
    wait(&pid);
    // parent recieve pong
    close(ctop[1]);
    read(ctop[0], buf, 1);
    fprintf(1, "%d: received pong\n", getpid());
  }
  exit(0);
}
