#include "types.h"
#include "sume_tokens.h"
#include "from_ir.h"
#include "data_s.h"
#include "populate_fns.h"

int add_fn(
  char*    prog,
  uint     prog_len,
  uint*    place,
  fn_info* fn_list,
  uint*    fn_num,
  char     rt)
{
  char type_buf[64];
  uint type_len;
  fn_info* cur_fn = &fn_list[*fn_num];
  cur_fn->rt = rt;

  if (get_type_list(
      prog,
      prog_len,
      place,
      type_buf,
      &type_len) < 0)
    return -1;
  if (add_fn_params(cur_fn, type_buf, type_len) < 0)
    return -1;

  if (get_type_list(
      prog,
      prog_len,
      place,
      type_buf,
      &type_len) < 0)
    return -1;
  if (add_fn_locals(cur_fn, type_buf, type_len) < 0)
    return -1;
  *fn_num += 1;
  return 0;
}

int
populate_fns(
  char*    prog,
  uint     prog_len,
  fn_info* fn_list,
  uint*    fn_num)
{
  uint place = 0;
  *fn_num = 0;

  while (place < prog_len) {
    instr_token token = prog[place];
    place += 1;
    switch (token) {
      case i_clz:
      case i_ctz:
      case i_popcnt:
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
      case i_eqz:
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
      case i_ld32:
      case i_ld16_s:
      case i_ld16_u:
      case i_ld8_s:
      case i_ld8_u:
      case i_memsz:
      case str32:
      case str16:
      case str8:
      case trap:
      case block:
      case loop:
      case ret:
      case end:
        break;
      case i_const:
      case get_l:
      case set_l:
      case br:
      case br_if:
      case call:
        if (get_int(prog, prog_len, &place, 0) < 0)
          return -1;
        break;
      case v_declfn:
        if (add_fn(prog,
            prog_len,
            &place,
            fn_list,
            fn_num, 0) < 0)
          return -1;
        break;
      case i_declfn: ;
        if (add_fn(prog,
            prog_len,
            &place,
            fn_list,
            fn_num, INT_T) < 0)
          return -1;
        break;
      default:
        return -1;
    }
  }
  return 0;
}


int
asm_add_fn(
  char*    prog,
  uint     prog_len,
  uint*    place,
  asm_fn_info* fn_list,
  uint*    fn_num,
  char     rt)
{
  char type_buf[64];
  uint type_len;
  asm_fn_info* cur_fn = &fn_list[*fn_num];
  cur_fn->rt = rt;

  if (get_type_list(
      prog,
      prog_len,
      place,
      type_buf,
      &type_len) < 0)
    return -1;
  cur_fn->param_n = type_len;

  if (get_type_list(
      prog,
      prog_len,
      place,
      type_buf,
      &type_len) < 0)
    return -1;
  cur_fn->local_n = type_len;

  *fn_num += 1;
  return 0;
}

int
asm_populate_fns(
  char*    prog,
  uint     prog_len,
  asm_fn_info* fn_list,
  uint*    fn_num)
{
  uint place = 0;
  *fn_num = 0;

  while (place < prog_len) {
    instr_token token = prog[place];
    place += 1;
    switch (token) {
      case i_clz:
      case i_ctz:
      case i_popcnt:
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
      case i_eqz:
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
      case i_ld32:
      case i_ld16_s:
      case i_ld16_u:
      case i_ld8_s:
      case i_ld8_u:
      case i_memsz:
      case str32:
      case str16:
      case str8:
      case trap:
      case block:
      case loop:
      case ret:
      case end:
        break;
      case i_const:
      case get_l:
      case set_l:
      case br:
      case br_if:
      case call:
        if (get_int(prog, prog_len, &place, 0) < 0)
          return -1;
        break;
      case v_declfn:
        if (asm_add_fn(prog,
            prog_len,
            &place,
            fn_list,
            fn_num, 0) < 0)
          return -1;
        break;
      case i_declfn: ;
        if (asm_add_fn(prog,
            prog_len,
            &place,
            fn_list,
            fn_num, INT_T) < 0)
          return -1;
        break;
      default:
        return -1;
    }
  }
  return 0;
}
