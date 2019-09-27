#include "types.h"
#include "from_ir.h"
#include "sume_tokens.h"
#include "data_s.h"
#include "populate_fns.h"
#include "verif.h"

typedef struct _stack_frame {
  int bottom;
  int top;
} stack_frame;

fn_info fn_infos[64];
int cur_fn = -1;

char stack[4096];
int stk_pos = -1;

stack_frame frames[64];
int cur_frame = -1;

int
push_t(
  char type)
{
  if (stk_pos == -1)
    return -1;
  if (stk_pos >= 4096)
    return -1;
  stack[stk_pos] = type;
  stk_pos += 1;
  return 0;
}

int
pop_t()
{
  if (cur_frame == -1)
    return -1;
  if (stk_pos == frames[cur_frame].bottom)
    return -1;
  stk_pos -= 1;
  return stack[stk_pos];
}


int
push_frame()
{
  if (cur_frame == 63)
    return -1;
  if (cur_frame >= 0) {
    frames[cur_frame].top = stk_pos;
    frames[cur_frame + 1].bottom = stk_pos;
  }
  cur_frame += 1;
  return 0;
}

int
pop_frame()
{
  if (cur_frame == -1)
    return -1;
  cur_frame -= 1;
  if (cur_frame != -1) {
    stk_pos = frames[cur_frame].top;
  }
  else {
    stk_pos = -1;
  }
  return 0;
}

int
is_in_fn()
{
  return cur_frame >= 0;
}

int
get_local_t(
  int lnum)
{
  if (lnum < 0) return -1;
  if (lnum < fn_infos[cur_fn].param_n) {
    return fn_infos[cur_fn].param_t[lnum];
  }
  lnum -= fn_infos[cur_fn].param_n;
  if (lnum >= fn_infos[cur_fn].local_n)           return -1;
  return fn_infos[cur_fn].local_t[lnum];
}


int
verify(
  char* prog,
  uint  prog_len)
{
  uint place = 0;
  uint num_fns;
  if (populate_fns(prog, prog_len, fn_infos, &num_fns) < 0)
    return -1;
  if (fn_infos[0].param_n != 0)
    return -1;
  int prev_fn_num = -1;

  while (place < prog_len) {
    int lnum;
    int type;
    int fnum;
    int fn_rt;
    instr_token token = prog[place];
    place += 1;
    switch (token) {
      case i_const:
        if (!is_in_fn())                                return -1;
        if (get_int(prog, prog_len, &place, 0) < 0)     return -1;
        if (push_t(INT_T) < 0)                          return -1;
        break;
      case i_clz:
      case i_ctz:
      case i_popcnt:
      case i_eqz:
      case i_ld32:
      case i_ld16_s:
      case i_ld16_u:
      case i_ld8_s:
      case i_ld8_u:
        if (!is_in_fn())                                return -1;
        if (pop_t() != INT_T)                           return -1;
        if (push_t(INT_T) < 0)                          return -1;
        break;
      case i_add:
      case i_and:
      case i_div_s:
      case i_div_u:
      case i_mul:
      case i_or:
      case i_rem_s:
      case i_rem_u:
      case i_rotl:
      case i_rotr:
      case i_shl:
      case i_shr_s:
      case i_shr_u:
      case i_sub:
      case i_xor:
      case i_eq:
      case i_ge_s:
      case i_ge_u:
      case i_gt_s:
      case i_gt_u:
      case i_le_s:
      case i_le_u:
      case i_lt_s:
      case i_lt_u:
      case i_ne:
        if (!is_in_fn())                                return -1;
        if (pop_t() != INT_T)                           return -1;
        if (pop_t() != INT_T)                           return -1;
        if (push_t(INT_T) < 0)                          return -1;
        break;
      case i_memsz:
        if (!is_in_fn())                                return -1;
        if (push_t(INT_T) < 0)                          return -1;
        break;
      case str32:
      case str16:
      case str8:
        if (!is_in_fn())                                return -1;
        if (pop_t() < 0)                                return -1;
        if (pop_t() != INT_T)                           return -1;
        break;
      case trap:
        if (!is_in_fn())                                return -1;
        if (place >= prog_len)                          return -1;
        if (prog[place] != end)                         return -1;
        break;
      case block:
      case loop:
        if (!is_in_fn())                                return -1;
        if (push_frame() < 0)                           return -1;
        break;
      case ret:
        if (!is_in_fn())                                return -1;
        if (place >= prog_len)                          return -1;
        if (prog[place] != end)                         return -1;
        fn_rt = fn_infos[fnum].rt;
        if (fn_rt != 0) {
          if (pop_t() != fn_rt)                         return -1;
        }
        if (place >= prog_len)                          return -1;
        if (prog[place] != end)                         return -1;
        break;
      case end:
        if (pop_frame() < 0)                            return -1;
        break;
      case get_l:
        if (!is_in_fn())                                return -1;
        if (get_int(prog, prog_len, &place, &lnum) < 0) return -1;
        type = get_local_t(lnum);
        if (type < 0)                                   return -1;
        if (push_t(type) < 0)                           return -1;
        break;
      case call:
        if (!is_in_fn())                                return -1;
        if (get_int(prog, prog_len, &place, &fnum) < 0) return -1;
        if (fnum < 0 || fnum >= num_fns)                return -1;
        for (int i = fn_infos[fnum].param_n-1; i >= 0; --i) {
          if (pop_t() != fn_infos[fnum].param_t[i])     return -1;
        }
        fn_rt = fn_infos[fnum].rt;
        if (fn_rt != 0) {
          if (push_t(fn_rt) < 0)                        return -1;
        }
        break;
      case set_l:
        if (!is_in_fn())                                return -1;
        if (get_int(prog, prog_len, &place, &lnum) < 0) return -1;
        type = pop_t();
        if (type < 0)                                   return -1;
        if (type != get_local_t(lnum))                  return -1;
        break;
      case br:
        if (!is_in_fn())                                return -1;
        if (get_int(prog, prog_len, &place, &lnum) < 0) return -1;
        if (lnum < 0)                                   return -1;
        if (lnum >= cur_frame)                          return -1;
        if (place >= prog_len)                          return -1;
        if (prog[place] != end)                         return -1;
        break;
      case br_if:
        if (!is_in_fn())                                return -1;
        if (get_int(prog, prog_len, &place, &lnum) < 0) return -1;
        if (lnum < 0)                                   return -1;
        if (lnum >= cur_frame)                          return -1;
        if (pop_t() != INT_T)                           return -1;
        break;
      case v_declfn:
      case i_declfn: ;
        char unused_buf[64];
        uint unused_len;
        if (is_in_fn())                                 return -1;
        if (push_frame() < 0)                           return -1;
        prev_fn_num += 1;
        stk_pos = 0;
        cur_fn = prev_fn_num;
        get_type_list(
          prog,
          prog_len,
          &place,
          unused_buf,
          &unused_len);
        get_type_list(
          prog,
          prog_len,
          &place,
          unused_buf,
          &unused_len);
        break;
      default:
        return -1;
    }
  }
  return 0;
}
