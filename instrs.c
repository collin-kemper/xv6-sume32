#include "types.h"
#include "user.h"
#include "instrs.h"

/**Notation:
 * r32   == 32-bit register
 * r8    == 8-bit register
 * ar32  == [r32]
 * imm32 == 32-bit immediate
 *
 * all functions return the length, and return the instruction through instr
 */

char
modrm(
  int mod,
  int regop,
  int rm)
{
  return ((mod&3)<<6) | ((regop&7)<<3) | (rm&7);
}

char
sib(
  int ss,
  int index,
  int base)
{
  return modrm(ss, index, base);
}

int
op_imm32(
  char  op,
  int   imm,
  char* instr)
{
  instr[0] = op;
  memmove(instr+1, &imm, 4);
  return 5;
}

int
op_modrm_imm32(
  char  op,
  char  mod,
  int   imm,
  char* instr)
{
  instr[0] = op;
  instr[1] = mod;
  memmove(instr+2, &imm, 4);
  return 6;
}

int
op_modrm_ib(
  char  op,
  char  mod,
  int   imm,
  char* instr)
{
  instr[0] = op;
  instr[1] = mod;
  instr[2] = imm;
  return 3;
}

int
op_modrm(
  char  op,
  char  modrm,
  char* instr)
{
  instr[0] = op;
  instr[1] = modrm;
  return 2;
}

int
op_op_modrm(
  char  op0,
  char  op1,
  char  modrm,
  char* instr)
{
  instr[0] = op0;
  instr[1] = op1;
  instr[2] = modrm;
  return 3;
}

int
op_rel32(
  char  op,
  int   rel,
  char* instr)
{
  instr[0] = op;
  rel -= 5;
  memmove(instr+1, &rel, 4);
  return 5;
}

int
op_op_rel32(
  char  op0,
  char  op1,
  int   rel,
  char* instr)
{
  instr[0] = op0;
  instr[1] = op1;
  rel -= 6;
  memmove(instr+2, &rel, 4);
  return 6;
}

int
add_r32_imm(
  int   r0,
  int   imm,
  char* instr)
{
  if (r0 == EAX) {
    return op_imm32(0x05, imm, instr);
  }
  else {
    return op_modrm_imm32(0x81, modrm(3, 0, r0), imm, instr);
  }
}

int
add_r32_r32(
  int   r0,
  int   r1,
  char* instr)
{
  return op_modrm(0x03, modrm(3, r0, r1), instr);
}

int
and_r32_r32(
  int   r0,
  int   r1,
  char* instr)
{
  return op_modrm(0x23, modrm(3, r0, r1), instr);
}

int
bsf_r32_r32(
  int   r0,
  int   r1,
  char* instr)
{
  return op_op_modrm(0x0f, 0xbc, modrm(3, r0, r1), instr);
}

int
bsr_r32_r32(
  int   r0,
  int   r1,
  char* instr)
{
  return op_op_modrm(0x0f, 0xbd, modrm(3, r0, r1), instr);
}

int
call_rel(
  int   start,
  int   end,
  char* instr)
{
  return op_rel32(0xe8, end - start, instr);
}

int
cmp_r32_r32(
  int   r0,
  int   r1,
  char* instr)
{
  return op_modrm(0x39, modrm(3, r1, r0), instr);
}

int
div_r32(
  int   r0,
  char* instr)
{
  return op_modrm(0xf7, modrm(3, 6, r0), instr);
}

int
idiv_r32(
  int   r0,
  char* instr)
{
  return op_modrm(0xf7, modrm(3, 7, r0), instr);
}

int
jae_rel(
  int   start,
  int   end,
  char* instr)
{
  return op_op_rel32(0x0f, 0x83, end - start, instr);
}

int
jb_rel(
  int   start,
  int   end,
  char* instr)
{
  return op_op_rel32(0x0f, 0x82, end - start, instr);
}

int
jz_rel(
  int   start,
  int   end,
  char* instr)
{
  return op_op_rel32(0x0f, 0x84, end - start, instr);
}

int
jne_rel(
  int   start,
  int   end,
  char* instr)
{
  return op_op_rel32(0x0f, 0x85, end - start, instr);
}

int
jmp_rel(
  int   start,
  int   end,
  char* instr)
{
  return op_rel32(0xe9, end - start, instr);
}

int
mov_r8_r8(
  int   r0,
  int   r1,
  char* instr)
{
  return op_modrm(0x88, modrm(3, r1, r0), instr);
}

int
mov_r32_r32(
  int   r0,
  int   r1,
  char* instr)
{
  return op_modrm(0x89, modrm(3, r1, r0), instr);
}

int
mov_dis_ebp_r32(
  int   dis,
  int   r1,
  char* instr)
{
  return op_modrm_imm32(0x89, modrm(2, r1, EBP), dis, instr);
}

int
mov_r32_dis_ebp(
  int   r0,
  int   dis,
  char* instr)
{
  return op_modrm_imm32(0x8b, modrm(2, r0, EBP), dis, instr);
}

int
mov_r32_ar32(
  int   r0,
  int   r1,
  char* instr)
{
  return op_modrm(0x8b, modrm(0, r0, r1), instr);
}

int
mov_ar32_r8(
  int   r0,
  int   r1,
  char* instr)
{
  return op_modrm(0x88, modrm(0, r1, r0), instr);
}

int
mov_ar32_r32(
  int   r0,
  int   r1,
  char* instr)
{
  return op_modrm(0x89, modrm(0, r1, r0), instr);
}

int
mov_r32_imm(
  int   r0,
  int   imm,
  char* instr)
{
  return op_imm32(0xb8+r0, imm, instr);
}

int
movsx_r32_ar8(
  int   r0,
  int   r1,
  char* instr)
{
  return op_op_modrm(0x0f, 0xbe, modrm(0, r0, r1), instr);
}
int
movsx_r32_ar16(
  int   r0,
  int   r1,
  char* instr)
{
  return op_op_modrm(0x0f, 0xbf, modrm(0, r0, r1), instr);
}
int
movzx_r32_ar8(
  int   r0,
  int   r1,
  char* instr)
{
  return op_op_modrm(0x0f, 0xb6, modrm(0, r0, r1), instr);
}
int
movzx_r32_ar16(
  int   r0,
  int   r1,
  char* instr)
{
  return op_op_modrm(0x0f, 0xb7, modrm(0, r0, r1), instr);
}

int
mul_r32(
  int   r0,
  char* instr)
{
  return op_modrm(0xf7, modrm(3, 4, r0), instr);
}

int
or_r32_r32(
  int   r0,
  int   r1,
  char* instr)
{
  return op_modrm(0x0b, modrm(3, r0, r1), instr);
}

int
push_r32(
  int   r0,
  int*  stkh,
  char* instr)
{
  *stkh += 1;
  instr[0] = 0x50 + r0;
  return 1;
}

int
pop_r32(
  int   r0,
  int*  stkh,
  char* instr)
{
  *stkh -= 1;
  instr[0] = 0x58 + r0;
  return 1;
}

int
ret_fn(
  char* instr)
{
  instr[0] = 0xc3;
  return 1;
}

int
rol_r32(
  int   r0,
  char* instr)
{
  return op_modrm(0xd3, modrm(3, 0, r0), instr);
}

int
ror_r32(
  int   r0,
  char* instr)
{
  return op_modrm(0xd3, modrm(3, 1, r0), instr);
}

int
sar_r32(
  int   r0,
  char* instr)
{
  return op_modrm(0xd3, modrm(3, 7, r0), instr);
}

int
shr_r32(
  int   r0,
  char* instr)
{
  return op_modrm(0xd3, modrm(3, 5, r0), instr);
}

int
shl_r32_imm(
  int   r0,
  int   imm,
  char* instr)
{
  return op_modrm_ib(0xc1, modrm(3, 4, r0), imm, instr);
}

int
shl_r32(
  int   r0,
  char* instr)
{
  return op_modrm(0xd3, modrm(3, 4, r0), instr);
}

int
seta_r8(
  int   r0,
  char* instr)
{
  return op_op_modrm(0x0f, 0x97, modrm(3, 0, r0), instr);
}
int
setae_r8(
  int   r0,
  char* instr)
{
  return op_op_modrm(0x0f, 0x93, modrm(3, 0, r0), instr);
}

int
setb_r8(
  int   r0,
  char* instr)
{
  return op_op_modrm(0x0f, 0x92, modrm(3, 0, r0), instr);
}
int
setbe_r8(
  int   r0,
  char* instr)
{
  return op_op_modrm(0x0f, 0x96, modrm(3, 0, r0), instr);
}
int
setg_r8(
  int   r0,
  char* instr)
{
  return op_op_modrm(0x0f, 0x9f, modrm(3, 0, r0), instr);
}
int
setge_r8(
  int   r0,
  char* instr)
{
  return op_op_modrm(0x0f, 0x9d, modrm(3, 0, r0), instr);
}
int
setl_r8(
  int   r0,
  char* instr)
{
  return op_op_modrm(0x0f, 0x9c, modrm(3, 0, r0), instr);
}
int
setle_r8(
  int   r0,
  char* instr)
{
  return op_op_modrm(0x0f, 0x9e, modrm(3, 0, r0), instr);
}
int
setne_r8(
  int   r0,
  char* instr)
{
  return op_op_modrm(0x0f, 0x95, modrm(3, 0, r0), instr);
}
int
setz_r8(
  int   r0,
  char* instr)
{
  return op_op_modrm(0x0f, 0x94, modrm(3, 0, r0), instr);
}

int
sub_r32_imm(
  int   r0,
  int   imm,
  char* instr)
{
  if (r0 == EAX) {
    return op_imm32(0x2d, imm, instr);
    return 5;
  }
  else {
    return op_modrm_imm32(0x81, modrm(3, 5, r0), imm, instr);
  }
}

int
sub_r32_r32(
  int   r0,
  int   r1,
  char* instr)
{
  return op_modrm(0x2b, modrm(3, r0, r1), instr);
}

int
test_r32_r32(
  int   r0,
  int   r1,
  char* instr)
{
  return op_modrm(0x85, modrm(3, r0, r1), instr);
}

int
xor_r32_r32(
  int   r0,
  int   r1,
  char* instr)
{
  return op_modrm(0x33, modrm(3, r0, r1), instr);
}
