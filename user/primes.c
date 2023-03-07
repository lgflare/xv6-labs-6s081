#include "kernel/types.h"
#include "user/user.h"

#define PRIME_LIMIT 35

void
handle(int pfd) {
  int j, k;
  int spid;
  int sp[2];
  if (read(pfd, &j, sizeof(int)) > 0) {
    fprintf(1, "prime %d\n", j);
    pipe(sp);
    spid = fork();
    if (spid < 0) {
      fprintf(2, "fork error\n");
      exit(1);
    } else if (spid == 0) {
      close(sp[1]);
      handle(sp[0]);
    } else {
      close(sp[0]);
      while (read(pfd, &k, sizeof(int)) > 0) {
        if (k % j != 0)
          write(sp[1], &k, sizeof(int));
      }
      close(sp[1]);
      close(pfd);
      wait(0);
    }
  } else
    close(pfd);
}

int
main() {
  int i;
  int pid;
  int p[2];
  pipe(p);
  pid = fork();

  if (pid < 0) {
    fprintf(2, "fork error\n");
    exit(1);
  } else if (pid == 0) {
    close(p[1]);
    handle(p[0]);
  } else {
    close(p[0]);
    for (i = 2; i <= PRIME_LIMIT; i++)
      write(p[1], &i, sizeof(int));
    close(p[1]);
    wait(0);
  }

  exit(0);
}