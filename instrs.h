#define AL  0
#define CL  1
#define DL  2
#define BL  3
#define AH  4
#define CH  5
#define DH  6
#define BH  7

#define AX  0
#define CX  1
#define DX  2
#define BX  3
#define SP  4
#define BP  5
#define SI  6
#define DI  7

#define EAX 0
#define ECX 1
#define EDX 2
#define EBX 3
#define ESP 4
#define EBP 5
#define ESI 6
#define EDI 7

int add_r32_imm    (int r0,  int imm, char* instr);
int add_r32_r32    (int r0,  int r1,  char* instr);
int and_r32_r32    (int r0,  int r1,  char* instr);
int bsf_r32_r32    (int r0,  int r1,  char* instr);
int bsr_r32_r32    (int r0,  int r1,  char* instr);
int call_rel       (int start, int end, char* instr);
int cmp_r32_r32    (int r0,  int r1,  char* instr);
int div_r32        (int r0,  char* instr);
int idiv_r32       (int r0,  char* instr);
int jae_rel        (int start, int end, char* instr);
int jb_rel         (int start, int end, char* instr);
int jz_rel         (int start, int end, char* instr);
int jne_rel        (int start, int end, char* instr);
int jmp_rel        (int start, int end, char* instr);
int mov_r8_r8      (int r0,  int r1,  char* instr);
int mov_r32_r32    (int r0,  int r1,  char* instr);
int mov_dis_ebp_r32(int dis, int r1,  char* instr);
int mov_r32_dis_ebp(int r0,  int dis, char* instr);
int mov_r32_ar32   (int r0,  int r1,  char* instr);
int mov_ar32_r8    (int r0,  int r1,  char* instr);
int mov_ar32_r32   (int r0,  int r1,  char* instr);
int mov_r32_imm    (int r0,  int imm, char* instr);
int movsx_r32_ar8  (int r0,  int r1,  char* instr);
int movsx_r32_ar16 (int r0,  int r1,  char* instr);
int movzx_r32_ar8  (int r0,  int r1,  char* instr);
int movzx_r32_ar16 (int r0,  int r1,  char* instr);
int mul_r32        (int r0,  char* instr);
int or_r32_r32     (int r0,  int r1,  char* instr);
int push_r32       (int r0,  int* stkh, char* instr);
int pop_r32        (int r0,  int* stkh, char* instr);
int ret_fn         (char* instr);
int rol_r32        (int r0,  char* instr);
int ror_r32        (int r0,  char* instr);
int sar_r32        (int r0,  char* instr);
int shr_r32        (int r0,  char* instr);
int shl_r32_imm    (int r0,  int imm, char* instr);
int shl_r32        (int r0,  char* instr);
int seta_r8        (int r0,  char* instr);
int setae_r8       (int r0,  char* instr);
int setb_r8        (int r0,  char* instr);
int setbe_r8       (int r0,  char* instr);
int setg_r8        (int r0,  char* instr);
int setge_r8       (int r0,  char* instr);
int setl_r8        (int r0,  char* instr);
int setle_r8       (int r0,  char* instr);
int setne_r8       (int r0,  char* instr);
int setz_r8        (int r0,  char* instr);
int sub_r32_imm    (int r0,  int imm, char* instr);
int sub_r32_r32    (int r0,  int r1,  char* instr);
int test_r32_r32   (int r0,  int r1,  char* instr);
int xor_r32_r32    (int r0,  int r1,  char* instr);
