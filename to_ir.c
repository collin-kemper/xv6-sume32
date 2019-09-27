#include "types.h"
#include "user.h"
#include "dyn_arr.h"
#include "sume_strs.h"
#include "sume_tokens.h"
#include "to_ir.h"

int
is_whitespace(
  char c)
{
  return c == ' '  || c == '\t' || c == '\n'
      || c == '\v' || c == '\f' || c == '\r';
}

int
is_prefix(
  const char* prefix,
  uint  prefix_len,
  char* word,
  uint  word_len)
{
  if (prefix_len > word_len) return 0;
  int i;
  for(i = 0; i < prefix_len; ++i) {
    if (prefix[i] != word[i]) return 0;
  }
  if (prefix_len == word_len) return 1;
  if (word[i] ==  ';'
      || is_whitespace(word[i]))
    return 1;
  return 0;
}

instr_token
get_next_instr_token(
  char* prog, 
  uint  prog_len,
  uint* place)
{
  char* cur_place = prog + *place;
  uint  rem_len = prog_len - *place;
  uint instr_prefixes_len = sizeof(instrs)/sizeof(instrs[0]);
  for(int i = 0; i < instr_prefixes_len; ++i) {
    uint pre_sz = strlen(instrs[i]);
    if (is_prefix(instrs[i], pre_sz, cur_place, rem_len)) {
      *place += pre_sz;
      return i;
    }
  }
  return -1;
}

/**Moves place to start of next line in prog
 */
void
get_line(
  char* prog,
  uint  prog_len,
  uint* place)
{
  while(*place < prog_len && prog[*place] != '\n')
    *place += 1;
  *place += 1;
}

/**moves place to the first non-whitespace character in prog
 */
void
consume_whitespace(
  char* prog,
  uint  prog_len,
  uint* place)
{
  while (*place < prog_len) {
    if (prog[*place] == ';') {
      *place += 1;
      get_line(prog, prog_len, place);
      continue;
    }
    if (is_whitespace(prog[*place])) {
      *place += 1;
      continue;
    }
    return;
  }
}


int
get_char(
  char* prog,
  uint  prog_len,
  uint* place,
  char* ret)
{
  if (*place >= prog_len)
    return -1;
  *ret = prog[*place];
  *place += 1;
  return 0;
}

int
char_to_int(
  char c,
  int* ret)
{
  int val;
  val = c - '0';
  if (val < 0)
    return -1;
  if (val < 10) {
    *ret = val;
    return 0;
  }
  val = c - 'A' + 10;
  if (val < 10)
    return -1;
  if (val < 36) {
    *ret = val;
    return 0;
  }
  val = c - 'a' + 10;
  if (val < 10)
    return -1;
  if (val < 36) {
    *ret = val;
    return 0;
  }
  return -1;
}

/**character to decimal
 */
int
char_to_dec(
  char c,
  int* ret)
{
  int val;
  val = c - '0';
  if (val < 0)
    return -1;
  if (val < 10) {
    *ret = val;
    return 0;
  }
  return -1;
}

int
get_int(
  char* prog,
  uint  prog_len,
  uint* place,
  uint* ret)
{
  *ret = 0;
  char cur;
  int neg = 0;
  int radix = 10;
  if (get_char(prog, prog_len, place, &cur) < 0)
    return -1;

  if (cur == '-') {
    neg = 1;
    if (get_char(prog, prog_len, place, &cur) < 0)
      return -1;
  }

  if (cur == 'b') {
    if (get_char(prog, prog_len, place, &cur) < 0)
      return -1;
    radix = 2;
  } else if (cur == 'o') {
    if (get_char(prog, prog_len, place, &cur) < 0)
      return -1;
    radix = 8;
  }else if (cur == 'x' || cur == 'h') {
    if (get_char(prog, prog_len, place, &cur) < 0)
      return -1;
    radix = 16;
  }

  int v = 0;
  if (char_to_int(cur, &v) < 0)
    return -1;

  *ret *= radix;
  *ret += v;

  while (1) {
    if (get_char(prog, prog_len, place, &cur) < 0)
      break;
    if (is_whitespace(cur))
      break;
    if (char_to_int(cur, &v) < 0)
      return -1;
    if (v >= radix)
      return -1;
    *ret *= radix;
    *ret += v;
  }

  if (neg)
    *ret = -*ret;
  return 0;
}

int
get_float(
  char*  prog,
  uint   prog_len,
  uint*  place,
  float* ret)
{
  *ret = 0.0;
  char cur;
  int neg = 0;
  if (get_char(prog, prog_len, place, &cur) < 0)
    return -1;

  if (cur == '-') {
    neg = 1;
    if (get_char(prog, prog_len, place, &cur) < 0)
      return -1;
  }

  int v;
  if (char_to_dec(cur, &v) < 0)
    return -1;

  *ret *= 10.0;
  *ret += (float) v;

  while (1) {
    if (get_char(prog, prog_len, place, &cur) < 0)
      return -1;
    if (cur == '.')
      break;
    if (char_to_dec(cur, &v) < 0)
      return -1;
    *ret *= 10.0;
    *ret += (float) v;
  }
  float dec_pl = 0.1;
  while (1) {
    if (get_char(prog, prog_len, place, &cur) < 0)
      break;
    if (is_whitespace(cur))
      break;
    if (char_to_dec(cur, &v) < 0)
      return -1;
    *ret += ((float) v) * dec_pl;
    dec_pl /= 10;
  }

  if (neg)
    *ret -= *ret;
  return 0;
}

int
get_type_list(
  char* prog,
  uint  prog_len,
  uint* place,
  char* ret,
  uint* ret_len)
{
  char cur;
  uint ret_pl = 0;
  if (get_char(prog, prog_len, place, &cur) < 0)
    return -1;
  if (cur != '(')
    return -1;
  consume_whitespace(prog, prog_len, place);
  while (1) {
    if (prog[*place] == ')')
      break;
    if (*place + 3 >= prog_len)
      return -1;
    if (prog[*place] == 'i'
        && prog[(*place)+1] == 'n'
        && prog[(*place)+2] == 't') {
      int ind = ret_pl / 4;
      int off = ret_pl % 4;
      ret[ind] &= ~(3<<(off*2)); // zero position
      ret[ind] |= (1<<(off*2)); // set equal to 1;
      ret_pl += 1;
    }
    else if(prog[*place] == 'f'
        && prog[(*place)+1] == 'l'
        && prog[(*place)+2] == 't') {
      int ind = ret_pl / 4;
      int off = ret_pl % 4;
      ret[ind] &= ~(3<<(off*2)); // zero position
      ret[ind] |= (2<<(off*2)); // set equal to 2;
      ret_pl += 1;
    }
    else {
      return -1;
    }
    *place += 3;
    consume_whitespace(prog, prog_len, place);
  }
  *place += 1;

  int ind = ret_pl / 4;
  int off = ret_pl % 4;
  ret[ind] &= ~(3<<(off*2)); // zero position
  ret_pl += 1;

  int ret_end = 4*((ret_pl-1)/4 + 1);
  for (; ret_pl < ret_end; ++ret_pl) {
    ind = ret_pl / 4;
    off = ret_pl % 4;
    ret[ind] &= ~(3<<(off*2)); // zero position
  }
  *ret_len = ret_end/4;
  return 0;
}

int
parse_sume(
  dyn_arr* result,
  char*    prog,
  uint     prog_len)
{
  uint place = 0;
  char instr_buf[100];
  uint instr_len;
  while (1) {
    consume_whitespace(prog, prog_len, &place);
    if (place >= prog_len)
      break;
    instr_token tkn = get_next_instr_token(prog, prog_len, &place);
    if (tkn == -1)
      return -1;
    instr_buf[0] = (char) tkn;
    instr_len = 1;
    switch (tkn) {
      case i_const:
      case get_l:
      case set_l:
      case br:
      case br_if:
      case call:
        consume_whitespace(prog, prog_len, &place);
        uint iv;
        if (get_int(prog, prog_len, &place, &iv) < 0)
          return -1;
        memmove(instr_buf+1, (char*) &iv, sizeof(iv));
        instr_len += sizeof(iv);
        break;
      case v_declfn:
      case i_declfn:
        consume_whitespace(prog, prog_len, &place);
        uint type_l_len = 0;
        if (get_type_list(prog,
            prog_len,
            &place,
            instr_buf+instr_len,
            &type_l_len) < 0)
          return -1;
        instr_len += type_l_len;
        consume_whitespace(prog, prog_len, &place);
        if (get_type_list(prog,
            prog_len,
            &place,
            instr_buf+instr_len,
            &type_l_len) < 0)
          return -1;
        instr_len += type_l_len;
        break;
      default:
        break;
    }
    if (append_dyn_arr(result, instr_buf, instr_len) < 0)
      return -1;
  }
  return 0;
}
