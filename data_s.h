typedef struct _asm_fn_info {
  int  rt;
  int fn_addr;
  int param_n;
  int local_n;
} asm_fn_info;

typedef struct _asm_blk_info {
  int br_addr;
  int stk_height;
} asm_blk_info;
