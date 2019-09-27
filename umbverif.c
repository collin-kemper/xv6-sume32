#include "types.h"
#include "fcntl.h"
#include "stat.h"
#include "user.h"
#include "dyn_arr.h"
#include "verif.h"

int
main(
  int argn,
  char** argv)
{
  if (argn != 2) {
    printf(2, "takes a single argument\n");
    return -1;
  }

  int in_fd = open(argv[1], O_RDONLY);
  if (in_fd == -1) {
    printf(2, "could not open file: %s\n", argv[1]);
    return -1;
  }

  struct stat fd_stat;
  if (fstat(in_fd, &fd_stat) == -1) {
    close(in_fd);
    printf(2, "could not stat file: %s\n", argv[1]);
    return -1;
  }

  uint f_sz = fd_stat.size;
  char* f_cont = malloc(f_sz);
  if (f_cont == 0) {
    close(in_fd);
    printf(2, "could not malloc space for file: %s\n", argv[1]);
    return -1;
  }
  if (read(in_fd, f_cont, f_sz) == -1) {
    close(in_fd);
    free(f_cont);
    printf(2, "could not read file: %s\n", argv[1]);
    return -1;
  }

  if (verify(f_cont, f_sz) < 0)
    printf(2, "verification failed\n");
  else
    printf(2, "verification succeeded!\n");
  close(in_fd);
  free(f_cont);
  exit();
  return 0;
}
