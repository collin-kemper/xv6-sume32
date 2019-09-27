#define VOD_T 0
#define INT_T 1
#define FLT_T 2
typedef struct _fn_info {
  char  rt;
  char* param_t;
  uint  param_n;
  char* local_t;
  uint  local_n;
} fn_info;

extern char heap[4096];
extern int heap_pos;

int get_int(char* prog, uint prog_len, uint* place, int* rv);
int get_float(char* prog, uint prog_len, uint* place, float* rv);
int get_type_list(char* prog, uint prog_len, uint* place, char* type_l, uint* len);
int add_fn_params(fn_info* fn, char* param_t, int param_n);
int add_fn_locals(fn_info* fn, char* local_t, int local_n);
