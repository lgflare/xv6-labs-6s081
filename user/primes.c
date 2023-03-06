#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PRIME_UP_LIMIT 35
#define PROC_LIMIT 35

typedef struct {     
  int ptocpipe[2];
  int ctoppipe[2];
} PIPE;

typedef struct {
  int lv;
  int base;
} PROC;

PIPE pipes;
PROC proc_ary[PROC_LIMIT];

int
main() {
  int pid;
  int p;
  int send_lv;
  int current_lv = 0;
  int i = 2;
  proc_ary[0].lv = 0;
  proc_ary[0].base = 0;
  pipe(pipes.ptocpipe);
  pipe(pipes.ctoppipe);

  while(1) {
    pid = fork();
    if (pid < 0) 
      exit(1);
    else if (pid == 0) {
      // child process
      int ret;
      current_lv++;
      ret = read(pipes.ptocpipe[0], &p, sizeof(int));
      if (ret < 0)
        exit(0);
      else {
        if (proc_ary[current_lv].base == 0) {
          // this process is created for the first time
          proc_ary[current_lv].base = p;
          proc_ary[current_lv].lv = current_lv;
          fprintf(1, "prime %d\n", p);
          write(pipes.ctoppipe[1], &proc_ary[current_lv].lv, sizeof(int));
          write(pipes.ctoppipe[1], &proc_ary[current_lv].base, sizeof(int));
          exit(0);
        } else {
          if (p % proc_ary[current_lv].base == 0) {
            write(pipes.ctoppipe[1], &proc_ary[current_lv].lv, sizeof(int));
            write(pipes.ctoppipe[1], &proc_ary[current_lv].base, sizeof(int));
            exit(0);
          } else
            continue;
        }
      }
    } else {
      // pid > 0, parent
      if (current_lv == 0) {
        while (i <= PRIME_UP_LIMIT) {
          if (proc_ary[0].base == 0) {
            proc_ary[0].base = i;
            fprintf(1, "prime %d\n", i);
            i++;
          } else {
            if (i % proc_ary[0].base != 0) {
              write(pipes.ptocpipe[1], &i, sizeof(int));
              wait(&pid);
              read(pipes.ctoppipe[0], &send_lv, sizeof(int));
              proc_ary[send_lv].lv = send_lv;
              read(pipes.ctoppipe[0], &proc_ary[send_lv].base, sizeof(int));
              i++;
              break;
            } 
            i++;
          }
        }
      } else if (current_lv > 0) {
        write(pipes.ptocpipe[1], &p, sizeof(int));
        wait(&pid);
        exit(0);
      }

      if (i <= PRIME_UP_LIMIT)
        continue;

      exit(0);
    }
  }
}