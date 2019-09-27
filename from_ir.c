#include "types.h"
#include "user.h"
#include "from_ir.h"

char heap[4096];
int  heap_pos = 0;

int
get_int(
  char* prog,
  uint  prog_len,
  uint* place,
  int*  rv)
{
  if (*place + 4 >= prog_len)
    return -1;
  if (rv != 0) {
    char* start = prog + *place;
    memmove(rv, start, sizeof(int));
  }
  *place += 4;
  return 0;
}

int
get_float(
  char* prog,
  uint  prog_len,
  uint* place,
  float* rv)
{
  if (*place + 4 >= prog_len)
    return -1;
  if (rv != 0) {
    char* start = prog + *place;
    memmove(rv, start, sizeof(float));
  }
  *place += 4;
  return 0;
}

int
get_type_list(
  char* prog,
  uint  prog_len,
  uint* place,
  char* type_l,
  uint* len)
{
  *len = 0;
  int pos = 0;
  char* start = prog + *place;
  while (1) {
    int ind = pos/4;
    int off = pos%4;
    if (ind + *place >= prog_len)
      return -1;
    int val = (start[ind]>>(off*2)) & 3;
    if (val == 0)
      break;
    type_l[pos] = (char) val;
    pos += 1;
  }
  *place += (pos)/4 + 1;
  *len = pos;
  return 0;
}

int
add_fn_params(
  fn_info* fn,
  char*    param_t,
  int      param_n)
{
  if (fn->param_t != 0) {
    if (param_n != fn->param_n)
      return -1;
    if (memcmp(fn->param_t, param_t, param_n) != 0)
      return -1;
  }
  else {
    fn->param_t = heap+heap_pos;
    fn->param_n = param_n;
    memmove(fn->param_t, param_t, param_n);
    heap_pos += param_n;
  }
  return 0;
}

int
add_fn_locals(
  fn_info* fn,
  char*    local_t,
  int      local_n)
{
  if (fn->local_t != 0) {
    if (local_n != fn->local_n)
      return -1;
    if (memcmp(fn->local_t, local_t, local_n) != 0)
      return -1;
  }
  else {
    fn->local_t = heap+heap_pos;
    fn->local_n = local_n;
    memmove(fn->local_t, local_t, local_n);
    heap_pos += local_n;
  }
  return 0;
}
