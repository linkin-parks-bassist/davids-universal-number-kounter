#include <stdint.h>

#ifndef DUNKPROG_H
#define DUNKPROG_H

#include "microops.h"
#include "opcodes.h"
#include "instr.h"

extern dunk_instr instrs[256];

#define POINTER 	4

#define FIRST_NIBBLE 		0b10000000000000
#define SECOND_NIBBLE 		0b01000000000000
#define THIRD_NIBBLE 		0b00100000000000
#define FOURTH_NIBBLE 		0b00010000000000

#define FOLLOWING(N) 		0b00000000100000 + N

#define OFFSET_FOLLOWING(N) 0b00000000010000 + N

#define FOLLOWING_MASK 		0b00001111111111
#define FOLLOWING_POS_MASK  0b00000000001111

/** Macros for classifying parameter types **/
#define CONSTANT 	1
#define REGISTER 	2
#define S_REGISTER 	3
#define STRING_P	16
#define LABEL_P		17
#define INVALID 	0xffff

#define MAX_PARAMS 3

#define N_REGISTERS 16
#define N_SPECIAL_REGS 5

#define N_PINS 16

#define N_ALU_OPS 8

#define MIX_IN				0b0000000010000000

#define BINSIZE 2 * 64 * 1024
#define NWORDS 64 * 1024

#define code_sequence_for(X) dr_data[X] = (uint16_t)current_position;

#define ALUa_RISING_EDGE 0x0100
#define ALUb_RISING_EDGE 0x0200

#define ALU_R(N) \
	append_mc1(ALUop(1, 1, N)); \
	append_mc1(datatoreg(1));
#define ALU_R_C(N) \
	append_mc1(pkptroutinc(0)); \
	append_mc2(ALUop(1, 0, N), datatoALUb); \
	append_mc1(datatoreg(1));
#define ALU_R_R(N)\
	append_mc1(ALUop(1, 2, N)); \
	append_mc1(datatoreg(1));
#define ALU_R_R_UNARY(N)\
	append_mc1(ALUop(2, 2, N)); \
	append_mc1(datatoreg(1));
#define ALU_R_R_R(N)\
	append_mc1(ALUop(1, 2, N)); \
	append_mc1(datatoreg(11));
#define ALU_R_R_C(N) \
	append_mc1(pkptroutinc(0)); \
	append_mc2(ALUop(1, 2, N), datatoALUb); \
	append_mc1(datatoreg(11));

#define COND_GOTO_R_C(jmp) \
	append_mc2(regtodata(1), incrementpk(0)); \
	append_mc2(tmpctosreg, jmp);
	
#define COND_GOTO_CMP_R_C_C(jmp) \
	append_mc1(pkptroutinc(1)); \
	append_mc2(datatoALUb, ALUop(1, 0, ALU_CMP) | ALUb_RISING_EDGE); \
	append_mc2(tmpctosreg, jmp);
	
#define COND_GOTO_CMP_UNSGN_R_C_C(jmp) \
	append_mc1(pkptroutinc(1)); \
	append_mc2(datatoALUb, ALUop(1, 0, ALU_CMP) | ALUb_RISING_EDGE); \
	append_mc2(tmpctosreg, jmp);
	
#define COND_GOTO_CMP_R_R_C(jmp) \
	append_mc2(ALUop(1, 2, ALU_CMP), incrementpk(0)); \
	append_mc2(tmpctosreg, jmp);
	
#define COND_GOTO_CMP_UNSGN_R_R_C(jmp) \
	append_mc2(ALUop(1, 2, ALU_UCMP), incrementpk(0)); \
	append_mc2(tmpctosreg, jmp);

#endif
