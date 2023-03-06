#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PRIME_UP_LIMIT 10
#define PIPE_LIMIT 100
#define PROC_LIMIT 10

typedef struct {     
  int ptocpipe[2];
  int ctoppipe[2];
} PIPE;

typedef struct {
  int read_cnt;
  int proc_first_num;
} PROC;

PIPE pipe_ary[PIPE_LIMIT];
PROC proc_ary[PROC_LIMIT];

int
main() {
  int i;
  int p;
  int ret;
  int pid;
  int current_proc_lv = 0;
  proc_ary[current_proc_lv].read_cnt = 0;
  proc_ary[current_proc_lv].proc_first_num = 2;

  for (i = 2; i <= PRIME_UP_LIMIT; i++) {
    if (current_proc_lv == 0) {
      if (proc_ary[current_proc_lv].read_cnt == 0) {
        fprintf(1, "prime: %d\n", i);
        proc_ary[current_proc_lv].proc_first_num = i;
        proc_ary[current_proc_lv].read_cnt++;
        continue;
      } else {
        if (i % proc_ary[current_proc_lv].proc_first_num == 0) {
          proc_ary[current_proc_lv].read_cnt++;
          continue;
        } else {
          // this pipe can pass info from parent to child
          pipe(pipe_ary[current_proc_lv].ptocpipe);
          write(pipe_ary[current_proc_lv].ptocpipe[1], &i, 4);
          close(pipe_ary[current_proc_lv].ptocpipe[1]);
          proc_ary[current_proc_lv].read_cnt++;
          // this pipe can pass info from child to parent
          pipe(pipe_ary[current_proc_lv].ctoppipe);
          pid = fork();
        }
      }
    } else {
      ret = read(pipe_ary[current_proc_lv - 1].ptocpipe[0], &p, 4);
      close(pipe_ary[current_proc_lv - 1].ptocpipe[0]);
     
      if (ret <= 0) {
        //current_proc_lv--;
        exit(0);
      }

      if (proc_ary[current_proc_lv].read_cnt == 0) {
        fprintf(1, "prime: %d\n", p);
        proc_ary[current_proc_lv].proc_first_num = p;
        proc_ary[current_proc_lv].read_cnt++;
        close(pipe_ary[current_proc_lv - 1].ctoppipe[0]);
        write(pipe_ary[current_proc_lv - 1].ctoppipe[1], &proc_ary[current_proc_lv].read_cnt, 4);
        close(pipe_ary[current_proc_lv - 1].ctoppipe[1]);
        continue;
      } else {
        if (p % proc_ary[current_proc_lv].proc_first_num == 0) {
          proc_ary[current_proc_lv].read_cnt++;
          close(pipe_ary[current_proc_lv - 1].ctoppipe[0]);
          write(pipe_ary[current_proc_lv - 1].ctoppipe[1], &proc_ary[current_proc_lv].read_cnt, 4);
          close(pipe_ary[current_proc_lv - 1].ctoppipe[1]);
          continue;
        } else {
          pipe(pipe_ary[current_proc_lv].ptocpipe);
          write(pipe_ary[current_proc_lv].ptocpipe[1], &i, 4);
          close(pipe_ary[current_proc_lv].ptocpipe[1]);
          proc_ary[current_proc_lv].read_cnt++;
          // this pipe can pass info from child to parent
          pipe(pipe_ary[current_proc_lv].ctoppipe);
          pid = fork();
        }
      }
    }

    if (pid < 0) {
      fprintf(1, "fork error\n");
      exit(1);
    }
    else if (pid == 0) {
      current_proc_lv++;
      close(pipe_ary[current_proc_lv - 1].ctoppipe[0]);
      write(pipe_ary[current_proc_lv - 1].ctoppipe[1], &proc_ary[current_proc_lv].read_cnt, 4);
      close(pipe_ary[current_proc_lv - 1].ctoppipe[1]);
      continue;
    }
    else {
      //fprintf(1, "i= %d\n", i);
      close(pipe_ary[current_proc_lv].ptocpipe[0]);
      wait(&pid);
      close(pipe_ary[current_proc_lv].ctoppipe[1]);
      read(pipe_ary[current_proc_lv].ctoppipe[0], &proc_ary[current_proc_lv + 1].read_cnt, 4);
      close(pipe_ary[current_proc_lv].ctoppipe[0]);
    }
  }
  exit(0);

}