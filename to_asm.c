#include "types.h"
#include "sume_tokens.h"
#include "from_ir.h"
#include "data_s.h"
#include "populate_fns.h"
#include "instrs.h"

asm_fn_info fn_infos[64];
asm_blk_info blk_infos[256];
uint fn_info_n;
int blk_info_n;

uint goodret_addr = 0;
uint trapret_addr = 0;
uint start_addr = 0;


uint cur_blk_stk[64];

int
prelude(
  char* asm_loc,
  uint  asm_i_start,
  uint  base_addr)
{
  uint asm_i = asm_i_start;
  int nil;
  asm_i += push_r32(EBP, &nil, asm_loc+asm_i);
  asm_i += mov_r32_r32(EBP, ESP, asm_loc+asm_i);
  asm_i += push_r32(EBX, &nil, asm_loc+asm_i);
  asm_i += push_r32(ESI, &nil, asm_loc+asm_i);
  asm_i += push_r32(EDI, &nil, asm_loc+asm_i);
  asm_i += mov_r32_dis_ebp(EDI, 8 + 0, asm_loc+asm_i);
  asm_i += mov_r32_dis_ebp(ESI, 8 + 4, asm_loc+asm_i);
  asm_i += mov_r32_dis_ebp(EAX, 8 + 8, asm_loc+asm_i);
  asm_i += sub_r32_imm(EAX, 4, asm_loc+asm_i);
  asm_i += mov_r32_r32(EBP, EAX, asm_loc+asm_i);
  asm_i += mov_ar32_r32(EAX, ESP, asm_loc+asm_i);
  asm_i += mov_r32_r32(ESP, EAX, asm_loc+asm_i);
  asm_i += mov_r32_r32(EBP, ESP, asm_loc+asm_i);
  asm_i += sub_r32_imm(ESP, 4*fn_infos[0].local_n, asm_loc+asm_i);
  asm_i += push_r32(EBP, &nil, asm_loc+asm_i);
  asm_i += mov_r32_r32(EBP, ESP, asm_loc+asm_i);
  asm_i += sub_r32_imm(EBP, 4, asm_loc+asm_i);
  asm_i += call_rel(base_addr+asm_i, start_addr, asm_loc+asm_i);
  return asm_i - asm_i_start;
}

int
goodret(
  char* asm_loc,
  uint  asm_i_start)
{
  int nil;
  uint asm_i = asm_i_start;
  asm_i += pop_r32(ESP, &nil, asm_loc+asm_i);
  asm_i += pop_r32(ESP, &nil, asm_loc+asm_i);
  asm_i += pop_r32(EDI, &nil, asm_loc+asm_i);
  asm_i += pop_r32(ESI, &nil, asm_loc+asm_i);
  asm_i += pop_r32(EBX, &nil, asm_loc+asm_i);
  asm_i += pop_r32(EBP, &nil, asm_loc+asm_i);
  asm_i += xor_r32_r32(EAX, EAX, asm_loc+asm_i);
  asm_i += ret_fn(asm_loc+asm_i);
  return asm_i - asm_i_start;
}

int
trapret(
  char* asm_loc,
  uint  asm_i_start)
{
  int nil;
  uint asm_i = asm_i_start;
  asm_i += mov_r32_r32(ESP, EBP, asm_loc+asm_i);
  asm_i += mov_r32_imm(EBX, goodret_addr, asm_loc+asm_i);
  // label unwind
  asm_i += pop_r32(EAX, &nil, asm_loc+asm_i);
  asm_i += pop_r32(ESP, &nil, asm_loc+asm_i);
  asm_i += cmp_r32_r32(EAX, EBX, asm_loc+asm_i);
  asm_i += jne_rel(0, -4, asm_loc+asm_i); // 1 + 1 + 2
  asm_i += pop_r32(ESP, &nil, asm_loc+asm_i);
  asm_i += pop_r32(EDI, &nil, asm_loc+asm_i);
  asm_i += pop_r32(ESI, &nil, asm_loc+asm_i);
  asm_i += pop_r32(EBX, &nil, asm_loc+asm_i);
  asm_i += pop_r32(EBP, &nil, asm_loc+asm_i);
  asm_i += mov_r32_imm(EAX, -1, asm_loc+asm_i);
  asm_i += ret_fn(asm_loc+asm_i);
  return asm_i - asm_i_start;
}

uint
populate_infos(
  char* prog,
  uint  prog_len,
  char* buf,
  uint  base_addr,
  int   gen_info)
{
  uint place = 0;
  uint asm_i = 0;
  int cur_fn = -1;
  int cur_blk = -1;
  int cur_blk_h = -1;
  int stkh = 0;

  asm_i += prelude(buf, asm_i, base_addr);
  goodret_addr = asm_i;
  asm_i += goodret(buf, asm_i);
  trapret_addr = asm_i;
  asm_i += trapret(buf, asm_i);
  start_addr = asm_i;

  while (place < prog_len) {
  int iv;
  uint uiv;
  int fnrt;
  int fnaddr;
  int fnlocal_n;
  int fnparam_n;
  int blkstk_h;
  int blkaddr;
  int blk_n;
  int nil;
  char empty_b[64];

  instr_token token = prog[place];
  place += 1;
  switch (token) {
  case i_const:
    get_int(prog, prog_len, &place, &iv);
    asm_i += mov_r32_imm(EAX, iv, buf+asm_i);
    asm_i += push_r32(EAX, &stkh, buf+asm_i);
    break;
  case i_clz:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += bsf_r32_r32(EAX, EAX, buf+asm_i);
    asm_i += push_r32(EAX, &stkh, buf+asm_i);
    break;
  case i_ctz:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += bsr_r32_r32(EAX, EAX, buf+asm_i);
    asm_i += push_r32(EAX, &stkh, buf+asm_i);
    break;
  case i_popcnt:
    break;
  case i_add:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += add_r32_r32(EAX, EBX, buf+asm_i);
    asm_i += push_r32(EAX, &stkh, buf+asm_i);
    break;
  case i_and:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += and_r32_r32(EAX, EBX, buf+asm_i);
    asm_i += push_r32(EAX, &stkh, buf+asm_i);
    break;
  case i_div_s:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += xor_r32_r32(EDX, EDX, buf+asm_i);
    asm_i += idiv_r32(EBX, buf+asm_i);
    asm_i += push_r32(EAX, &stkh, buf+asm_i);
    break;
  case i_div_u:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += xor_r32_r32(EDX, EDX, buf+asm_i);
    asm_i += div_r32(EBX, buf+asm_i);
    asm_i += push_r32(EAX, &stkh, buf+asm_i);
    break;
  case i_mul:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += mul_r32(EBX, buf+asm_i);
    asm_i += push_r32(EAX, &stkh, buf+asm_i);
    break;
  case i_or:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += or_r32_r32(EAX, EBX, buf+asm_i);
    asm_i += push_r32(EAX, &stkh, buf+asm_i);
    break;
  case i_rem_s:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += xor_r32_r32(EDX, EDX, buf+asm_i);
    asm_i += idiv_r32(EBX, buf+asm_i);
    asm_i += push_r32(EDX, &stkh, buf+asm_i);
    break;
  case i_rem_u:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += xor_r32_r32(EDX, EDX, buf+asm_i);
    asm_i += div_r32(EBX, buf+asm_i);
    asm_i += push_r32(EDX, &stkh, buf+asm_i);
    break;
  case i_rotl:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += mov_r32_r32(ECX, EBX, buf+asm_i);
    asm_i += rol_r32(EAX, buf+asm_i);
    asm_i += push_r32(EAX, &stkh, buf+asm_i);
    break;
  case i_rotr:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += mov_r32_r32(ECX, EBX, buf+asm_i);
    asm_i += ror_r32(EAX, buf+asm_i);
    asm_i += push_r32(EAX, &stkh, buf+asm_i);
    break;
  case i_shl:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += mov_r32_r32(ECX, EBX, buf+asm_i);
    asm_i += shl_r32(EAX, buf+asm_i);
    asm_i += push_r32(EAX, &stkh, buf+asm_i);
    break;
  case i_shr_s:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += mov_r32_r32(ECX, EBX, buf+asm_i);
    asm_i += sar_r32(EAX, buf+asm_i);
    asm_i += push_r32(EAX, &stkh, buf+asm_i);
    break;
  case i_shr_u:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += mov_r32_r32(ECX, EBX, buf+asm_i);
    asm_i += shr_r32(EAX, buf+asm_i);
    asm_i += push_r32(EAX, &stkh, buf+asm_i);
    break;
  case i_sub:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += sub_r32_r32(EAX, EBX, buf+asm_i);
    asm_i += push_r32(EAX, &stkh, buf+asm_i);
    break;
  case i_xor:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += xor_r32_r32(EAX, EBX, buf+asm_i);
    asm_i += push_r32(EAX, &stkh, buf+asm_i);
    break;
  case i_eqz:
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += xor_r32_r32(ECX, ECX, buf+asm_i);
    asm_i += test_r32_r32(EAX, EAX, buf+asm_i);
    asm_i += setz_r8(CL, buf+asm_i);
    asm_i += push_r32(ECX, &stkh, buf+asm_i);
    break;
  case i_eq:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += xor_r32_r32(ECX, ECX, buf+asm_i);
    asm_i += cmp_r32_r32(EAX, EBX, buf+asm_i);
    asm_i += setz_r8(CL, buf+asm_i);
    asm_i += push_r32(ECX, &stkh, buf+asm_i);
    break;
  case i_ge_s:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += xor_r32_r32(ECX, ECX, buf+asm_i);
    asm_i += cmp_r32_r32(EAX, EBX, buf+asm_i);
    asm_i += setge_r8(CL, buf+asm_i);
    asm_i += push_r32(ECX, &stkh, buf+asm_i);
    break;
  case i_ge_u:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += xor_r32_r32(ECX, ECX, buf+asm_i);
    asm_i += cmp_r32_r32(EAX, EBX, buf+asm_i);
    asm_i += setae_r8(CL, buf+asm_i);
    asm_i += push_r32(ECX, &stkh, buf+asm_i);
    break;
  case i_gt_s:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += xor_r32_r32(ECX, ECX, buf+asm_i);
    asm_i += cmp_r32_r32(EAX, EBX, buf+asm_i);
    asm_i += setg_r8(CL, buf+asm_i);
    asm_i += push_r32(ECX, &stkh, buf+asm_i);
    break;
  case i_gt_u:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += xor_r32_r32(ECX, ECX, buf+asm_i);
    asm_i += cmp_r32_r32(EAX, EBX, buf+asm_i);
    asm_i += seta_r8(CL, buf+asm_i);
    asm_i += push_r32(ECX, &stkh, buf+asm_i);
    break;
  case i_le_s:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += xor_r32_r32(ECX, ECX, buf+asm_i);
    asm_i += cmp_r32_r32(EAX, EBX, buf+asm_i);
    asm_i += setle_r8(CL, buf+asm_i);
    asm_i += push_r32(ECX, &stkh, buf+asm_i);
    break;
  case i_le_u:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += xor_r32_r32(ECX, ECX, buf+asm_i);
    asm_i += cmp_r32_r32(EAX, EBX, buf+asm_i);
    asm_i += setbe_r8(CL, buf+asm_i);
    asm_i += push_r32(ECX, &stkh, buf+asm_i);
    break;
  case i_lt_s:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += xor_r32_r32(ECX, ECX, buf+asm_i);
    asm_i += cmp_r32_r32(EAX, EBX, buf+asm_i);
    asm_i += setl_r8(CL, buf+asm_i);
    asm_i += push_r32(ECX, &stkh, buf+asm_i);
    break;
  case i_lt_u:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += xor_r32_r32(ECX, ECX, buf+asm_i);
    asm_i += cmp_r32_r32(EAX, EBX, buf+asm_i);
    asm_i += setb_r8(CL, buf+asm_i);
    asm_i += push_r32(ECX, &stkh, buf+asm_i);
    break;
  case i_ne:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += xor_r32_r32(ECX, ECX, buf+asm_i);
    asm_i += cmp_r32_r32(EAX, EBX, buf+asm_i);
    asm_i += setne_r8(CL, buf+asm_i);
    asm_i += push_r32(ECX, &stkh, buf+asm_i);
    break;
  case i_ld32:
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += shl_r32_imm(EAX, 2, buf+asm_i);
    asm_i += cmp_r32_r32(EAX, ESI, buf+asm_i);
    asm_i += jae_rel(base_addr+asm_i, trapret_addr, buf+asm_i);
    asm_i += add_r32_r32(EAX, EDI, buf+asm_i);
    asm_i += mov_r32_ar32(EAX, EAX, buf+asm_i);
    asm_i += push_r32(EAX, &stkh, buf+asm_i);
    break;
  case i_ld16_s:
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += shl_r32_imm(EAX, 1, buf+asm_i);
    asm_i += cmp_r32_r32(EAX, ESI, buf+asm_i);
    asm_i += jae_rel(base_addr+asm_i, trapret_addr, buf+asm_i);
    asm_i += add_r32_r32(EAX, EDI, buf+asm_i);
    asm_i += movsx_r32_ar16(EAX, EAX, buf+asm_i);
    asm_i += push_r32(EAX, &stkh, buf+asm_i);
    break;
  case i_ld16_u:
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += shl_r32_imm(EAX, 1, buf+asm_i);
    asm_i += cmp_r32_r32(EAX, ESI, buf+asm_i);
    asm_i += jae_rel(base_addr+asm_i, trapret_addr, buf+asm_i);
    asm_i += add_r32_r32(EAX, EDI, buf+asm_i);
    asm_i += movzx_r32_ar16(EAX, EAX, buf+asm_i);
    asm_i += push_r32(EAX, &stkh, buf+asm_i);
    break;
  case i_ld8_s:
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += cmp_r32_r32(EAX, ESI, buf+asm_i);
    asm_i += jae_rel(base_addr+asm_i, trapret_addr, buf+asm_i);
    asm_i += add_r32_r32(EAX, EDI, buf+asm_i);
    asm_i += movsx_r32_ar8(EAX, EAX, buf+asm_i);
    asm_i += push_r32(EAX, &stkh, buf+asm_i);
    break;
  case i_ld8_u:
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += cmp_r32_r32(EAX, ESI, buf+asm_i);
    asm_i += jae_rel(base_addr+asm_i, trapret_addr, buf+asm_i);
    asm_i += add_r32_r32(EAX, EDI, buf+asm_i);
    asm_i += movzx_r32_ar8(EAX, EAX, buf+asm_i);
    asm_i += push_r32(EAX, &stkh, buf+asm_i);
    break;
  case i_memsz:
    asm_i += push_r32(ESI, &stkh, buf+asm_i);
    break;
  case str32: 
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += shl_r32_imm(EAX, 2, buf+asm_i);
    asm_i += cmp_r32_r32(EAX, ESI, buf+asm_i);
    asm_i += jae_rel(base_addr+asm_i, trapret_addr, buf+asm_i);
    asm_i += add_r32_r32(EAX, EDI, buf+asm_i);
    asm_i += mov_ar32_r32(EAX, EBX, buf+asm_i);
    break;
  case str16:
    break;// TEMPORARY
  case str8:
    asm_i += pop_r32(EBX, &stkh, buf+asm_i);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += cmp_r32_r32(EAX, ESI, buf+asm_i);
    asm_i += jae_rel(base_addr+asm_i, trapret_addr, buf+asm_i);
    asm_i += add_r32_r32(EAX, EDI, buf+asm_i);
    asm_i += mov_ar32_r8(EAX, BL, buf+asm_i);
    break;
  case get_l:
    get_int(prog, prog_len, &place, &iv);
    fnparam_n = fn_infos[cur_fn].param_n;
    fnlocal_n = fn_infos[cur_fn].local_n;
    iv = (fnparam_n + fnlocal_n - iv + 1);
    asm_i += mov_r32_dis_ebp(EAX, 4*iv, buf+asm_i);
    asm_i += push_r32(EAX, &stkh, buf+asm_i);
    break;
  case set_l:
    get_int(prog, prog_len, &place, &iv);
    fnparam_n = fn_infos[cur_fn].param_n;
    fnlocal_n = fn_infos[cur_fn].local_n;
    iv = (fnparam_n + fnlocal_n - iv + 1);
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += mov_dis_ebp_r32(4*iv, EAX, buf+asm_i);
    break;
  case trap:
    asm_i += jmp_rel(base_addr+asm_i, trapret_addr, buf+asm_i);
    break;
  case block:
    cur_blk += 1;
    cur_blk_h += 1;
    cur_blk_stk[cur_blk_h] = cur_blk;
    if (gen_info) {
      blk_infos[cur_blk].br_addr = 0;
      blk_infos[cur_blk].stk_height = stkh;
    }
    break;
  case loop:
    cur_blk += 1;
    cur_blk_h += 1;
    cur_blk_stk[cur_blk_h] = cur_blk;
    if (gen_info) {
      blk_infos[cur_blk].br_addr = base_addr+asm_i;
      blk_infos[cur_blk].stk_height = stkh;
    }
    break;
  case br:
    get_int(prog, prog_len, &place, &iv);
    blk_n = cur_blk_stk[cur_blk_h-iv];
    blkaddr = blk_infos[blk_n].br_addr;
    asm_i += add_r32_imm(ESP, 4*(stkh), buf+asm_i);
    asm_i += jmp_rel(base_addr+asm_i, blkaddr, buf+asm_i);
    break;
  case br_if:
    get_int(prog, prog_len, &place, &iv);
    blk_n = cur_blk_stk[cur_blk_h-iv];
    blkaddr = blk_infos[blk_n].br_addr;
    asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    asm_i += test_r32_r32(EAX, EAX, buf+asm_i);
    asm_i += jz_rel(0, 17, buf+asm_i);
    asm_i += add_r32_imm(ESP, 4*(stkh), buf+asm_i);
    asm_i += jmp_rel(base_addr+asm_i, blkaddr, buf+asm_i);
    //next = 6 + 6 + 5
    break;
  case ret:
    if (fn_infos[cur_fn].rt != 0) {
      asm_i += pop_r32(EAX, &stkh, buf+asm_i);
    }
    asm_i += mov_r32_r32(ESP, EBP, buf+asm_i);
    asm_i += ret_fn(buf+asm_i);
    break;
  case end:
    if (cur_blk_h > 0) {
      blk_n = cur_blk_stk[cur_blk_h];
      blkstk_h = blk_infos[blk_n].stk_height;
      blk_n = cur_blk_stk[cur_blk_h-1];
      stkh = blk_infos[blk_n].stk_height;
      blkstk_h -= stkh;
      asm_i += add_r32_imm(ESP, 4*blkstk_h, buf+asm_i);

      blk_n = cur_blk_stk[cur_blk_h];
      if (gen_info && blk_infos[blk_n].br_addr == 0) {
        blk_infos[blk_n].br_addr = base_addr+asm_i;
      }
    }
    cur_blk_h -= 1;
    break;
  case call:
    get_int(prog, prog_len, &place, &iv);
    fnrt      = fn_infos[iv].rt;
    fnaddr    = fn_infos[iv].fn_addr;
    fnparam_n = fn_infos[iv].param_n;
    fnlocal_n = fn_infos[iv].local_n;
    asm_i += sub_r32_imm(ESP, fnlocal_n, buf+asm_i);
    asm_i += push_r32(EBP, &nil, buf+asm_i);
    asm_i += mov_r32_r32(EBP, ESP, buf+asm_i);
    asm_i += sub_r32_imm(EBP, 4, buf+asm_i);
    asm_i += call_rel(base_addr+asm_i, fnaddr, buf+asm_i);
    asm_i += pop_r32(EBP, &nil, buf+asm_i);
    asm_i += add_r32_imm(ESP, 4*(fnparam_n+fnlocal_n), buf+asm_i);
    stkh -= fnparam_n;
    if (fnrt != 0)
      asm_i += push_r32(EAX, &stkh, buf+asm_i);
    break;
  case v_declfn:
  case i_declfn:
    cur_fn += 1;
    cur_blk += 1;
    cur_blk_h += 1;
    cur_blk_stk[cur_blk_h] = cur_blk;
    get_type_list(prog, prog_len, &place, empty_b, &uiv);
    get_type_list(prog, prog_len, &place, empty_b, &uiv);
    if (gen_info) {
      fn_infos[cur_fn].fn_addr = base_addr+asm_i;
      blk_infos[cur_blk].br_addr = -1;
      blk_infos[cur_blk].stk_height = 0;
    }
    break;
  default:
    break;
  }
  }
  return asm_i;
}

void
to_asm(
  char* prog,
  uint  prog_len,
  char* asm_loc,
  uint* asm_sz,
  uint  base_addr)
{
  asm_populate_fns(prog, prog_len, fn_infos, &fn_info_n);
  populate_infos(prog, prog_len, asm_loc, base_addr, 1);
  *asm_sz = populate_infos(prog, prog_len, asm_loc, base_addr, 0);
}
