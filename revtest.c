#include "types.h"
#include "user.h"
#include "stat.h"
#include "fcntl.h"

char message[13] = "\0!dlrow olleH";

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
  void (*revfn)() = (void(*)())rv;
  char* stack = (char*) malloc(512);
  revfn(message, 13, stack+512);
  printf(1, "message= %s\n", message);
  free(stack);

  exit();
  return 0;
}
