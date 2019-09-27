#include "types.h"
#include "user.h"
#include "stat.h"
#include "fcntl.h"

int  fibs[10]    = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

int
main(int argc, char* argv[])
{
  if (argc != 2)
    exit();
  int fd = open(argv[1], O_RDONLY);
  struct stat fds;
  fstat(fd, &fds);
  void* buf = malloc(fds.size);
  read(fd, buf, fds.size);
  close(fd);
  int rv = (int)sbrkx(buf, fds.size);;

  void (*fibfn)() = (void(*)())rv;
  char* stack = (char*) malloc(4096);
  fibfn(fibs, 10*sizeof(int), stack+4096);
  printf(1, "The first ten fibs are:\n");
  for (int i = 0; i < 10; ++i) {
    printf(1, "  fib(%d) = %d\n", i, fibs[i]);
  }
  free(stack);

  exit();
  return 0;
}
