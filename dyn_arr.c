#include "types.h"
#include "user.h"
#include "types.h"
#include "dyn_arr.h"

dyn_arr
make_dyn_arr(
  uint sz)
{
  dyn_arr ret = { malloc(sz), 0, sz };
  return ret;
}

int
append_dyn_arr(
  dyn_arr* arr,
  char* dat,
  uint sz)
{
  if (arr->size + sz > arr->max_sz) {
    char* new_data = malloc(arr->max_sz + PAGE_SZ);
    if (new_data == 0) return -1;
    memmove(new_data, arr->data, arr->size);
  }
  memmove(arr->data + arr->size, dat, sz);
  arr->size += sz;
  return 0;
}

void
free_dyn_arr(
  dyn_arr* arr)
{
  free(arr->data);
}
