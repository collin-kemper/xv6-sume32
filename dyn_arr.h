#define PAGE_SZ 4096

typedef struct _dyn_arr {
  char* data;
  uint  size;
  uint  max_sz;
} dyn_arr;

dyn_arr make_dyn_arr(uint sz);
int     append_dyn_arr(dyn_arr* arr, char* dat, uint sz);
void    free_dyn_arr(dyn_arr* arr);
