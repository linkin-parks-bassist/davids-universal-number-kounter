#include <stdint.h>

#ifndef DUNKPROG_H
#define DUNKPROG_H

#define N_REGISTERS 16
#define N_SPECIAL_REGS 5

#define N_PINS 16

#define N_ALU_OPS 8

#define MIX_IN				0b0000000010000000

#define BINSIZE 2 * 64 * 1024
#define NWORDS 64 * 1024

#define code_sequence_for(X) dr_data[X] = (uint16_t)current_position;

#define ALU_ADD 	0x0
#define ALU_SUB 	0x1
#define ALU_MUL 	0x2
#define ALU_INC 	0x3
#define ALU_DEC 	0x4
#define ALU_NEG 	0x5
#define ALU_UCMP 	0x6
#define ALU_CMP 	0x7
#define ALU_SGN 	0x8
#define ALU_AND 	0x9
#define ALU_OR 		0xa
#define ALU_XOR 	0xb
#define ALU_NOT 	0xc
#define ALU_LSHIFT 	0xd
#define ALU_RSHIFT 	0xe
#define ALU_SHIFT  	0xf

#define ALUa_RISING_EDGE 0x0100
#define ALUb_RISING_EDGE 0x0200

#define ALU_R(N) \
	append_mc1(ALUop(1, 1, N)); \
	append_mc1(datatoreg(1));
#define ALU_R_C(N) \
	append_mc1(tmpctodata); \
	append_mc2(datatoALUb, ALUop(1, 0, N)); \
	append_mc1(datatoreg(1));
#define ALU_R_R(N)\
	append_mc1(ALUop(1, 2, N)); \
	append_mc1(datatoreg(1));
#define ALU_R_R_R(N)\
	append_mc1(ALUop(1, 2, N)); \
	append_mc1(datatoreg(11));
#define ALU_R_R_C(N) \
	append_mc1(tmpctodata); \
	append_mc2(datatoALUb, ALUop(1, 2, N)); \
	append_mc1(datatoreg(11));

#define COND_GOTO_R_C(jmp) \
	append_mc2(regtodata(1), incrementpk); \
	append_mc2(tmpctosreg, jmp);
	
#define COND_GOTO_CMP_R_C_C(jmp) \
	append_mc1(incrementpk); \
	append_mc1(pkptroutinc); \
	append_mc2(datatoALUb, ALUop(1, 0, ALU_CMP) & ALUb_RISING_EDGE); \
	append_mc2(holddata, incrementpk); \
	append_mc2(tmpctosreg, jmp);
	
#define COND_GOTO_CMP_UNSGN_R_C_C(jmp) \
	append_mc1(incrementpk); \
	append_mc1(pkptroutinc); \
	append_mc2(datatoALUb, ALUop(1, 0, ALU_CMP) & ALUb_RISING_EDGE); \
	append_mc2(holddata, incrementpk); \
	append_mc2(tmpctosreg, jmp);
	
#define COND_GOTO_CMP_R_R_C(jmp) \
	append_mc1(ALUop(1, 2, ALU_CMP)); \
	append_mc2(jmp, incrementpk);
	
#define COND_GOTO_CMP_UNSGN_R_R_C(jmp) \
	append_mc1(ALUop(1, 2, ALU_UCMP)); \
	append_mc2(jmp, incrementpk);

#endif
