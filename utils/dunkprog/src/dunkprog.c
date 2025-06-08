#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dunkprog.h"
#include "microops.h"
#include "opcodes.h"

int mr1_data[BINSIZE];
int mr2_data[BINSIZE];
int dr_data[256];
int		 pr_data[8192];
	
int current_position = 0;

void append_mc1(int v1)
{
	mr1_data[current_position] = v1;
	mr2_data[current_position] = 0x0000;
	current_position += 1;
}

void append_mc2(int v1, int v2)
{
	mr1_data[current_position] = v1;
	mr2_data[current_position] = v2;
	current_position += 1;
}

#define PTR_PRINTER		0b001

#define IMM_PRINTER		0b000
#define REG_PRINTER  	0b010
#define SREG_PRINTER	0b110

#define NO_ARG_PRINTER		0b100

#define FN_PRINTER 			0b000000
#define SN_PRINTER 			0b001000
#define F1_PRINTER			0b010000
#define F2_PRINTER			0b011000
#define FN_OFFS1_PRINTER	0b100000
#define FN_OFFS2_PRINTER	0b101000
#define SN_OFFS1_PRINTER	0b101000
#define SN_OFFS2_PRINTER	0b111000

int struct_flags_to_printer_code(int type, int position)
{
	int arg_code = (((type & ~POINTER) == REGISTER != 0) ? REG_PRINTER : 0) + (((type & ~POINTER) == S_REGISTER) ? SREG_PRINTER : 0);
	
	arg_code += ((type & POINTER) != 0) ? PTR_PRINTER : 0;
	
	switch (position)
	{
		case FIRST_NIBBLE:
			arg_code += FN_PRINTER;
			break;
		
		case SECOND_NIBBLE:
			arg_code += SN_PRINTER;
			break;
		
		case FIRST_NIBBLE | SECOND_NIBBLE:
			arg_code += FN_PRINTER;
			break;
		
		case FOLLOWING(1):
			arg_code += F1_PRINTER;
			break;
		
		case FOLLOWING(2):
			arg_code += F2_PRINTER;
			break;
		
		case FIRST_NIBBLE | OFFSET_FOLLOWING(1):
			arg_code += FN_OFFS1_PRINTER;
			break;
		
		case FIRST_NIBBLE | OFFSET_FOLLOWING(2):
			arg_code += FN_OFFS2_PRINTER;
			break;
		
		case SECOND_NIBBLE | OFFSET_FOLLOWING(1):
			arg_code += SN_OFFS1_PRINTER;
			break;
		
		case SECOND_NIBBLE | OFFSET_FOLLOWING(2):
			arg_code += SN_OFFS2_PRINTER;
			break;
	}
	
	return arg_code;
}

void gen_printer_code(int opcode)
{
	int len = strlen(instrs[opcode].name);
	int start_position = opcode << 5;
	int position = start_position;
	
	for (int i = 0; i < len; i++)
		pr_data[position++] = (int)instrs[opcode].name[i];
	
	if (instrs[opcode].n_parameters == 0)
	{
		while (position < start_position + 32)
			pr_data[position++] = 0x00;
		
		return;
	}
	pr_data[position++] = 0xa0;
	
	int arg_code = 0;
	
	for (int i = 2; i > -1; i--)
	{
		arg_code = arg_code << 6;
		
		if (instrs[opcode].n_parameters <= i)
		{
			arg_code += NO_ARG_PRINTER;
			continue;
		}
		
		arg_code += struct_flags_to_printer_code(instrs[opcode].parameter_types[i], instrs[opcode].parameter_positions[i]);
	}
	
	while (position < start_position + 32)
		pr_data[position++] = arg_code;
}

void writeout_v3_hex_addressed(int *buf, int n, FILE *outfile)
{
	if (!outfile || !buf)
	{
		fprintf(stderr, "Error: NULL pointer passed to function \"writeout_v3_hex_addressed\"\n");
		exit(EXIT_FAILURE);
	}
	
	fputs("v3.0 hex words addressed", outfile);
	
	for (int i = 0; i < n; i++)
	{
		if (i % 8 == 0)
			fprintf(outfile, "\n%04x:", i);
		
		fprintf(outfile, " %05x", buf[i]);
	}
}

void begin_instruction(int opcode)
{
	code_sequence_for(opcode);
	gen_printer_code(opcode);
}

void generate_roms()
{
	// Initialise everything to 0 first, of course.
	for (int i = 0; i < NWORDS; i++)
	{
		mr1_data[i] = 0;
		mr2_data[i] = 0;
	}
	for (int i = 0; i < 256; i++)
	{
		dr_data[i] = 0;
	}
	

	// After an instruction completes, the micro-code address
	// register is reset to 0, and the CPU executes the following,
	// fetching and decoding the next instruction and jumping to
	// the appropriate point in the micro-code ROMs to execute it
	append_mc1(pkptroutinc(0));
	append_mc2(begini, pkptrout);
	
	begin_instruction(CHILL);		// chill
	append_mc1(do_nothing);
	append_mc1(done);

	begin_instruction(GOTO_C);		// goto
	append_mc2(tmpctosreg, tmptopk);
	append_mc1(done);

	begin_instruction(GOTO_IF_ZERO_R_C);
	COND_GOTO_R_C(tmptopkifz);
	append_mc1(done);

	begin_instruction(GOTO_IF_NONZERO_R_C);
	COND_GOTO_R_C(tmptopkifnz);
	append_mc1(done);

	begin_instruction(GOTO_IF_NEGATIVE_R_C);
	COND_GOTO_R_C(tmptopkifn);
	append_mc1(done);

	begin_instruction(GOTO_IF_NONNEGATIVE_R_C);
	COND_GOTO_R_C(tmptopkifnn);
	append_mc1(done);

	begin_instruction(GOTO_IF_POSITIVE_R_C);
	COND_GOTO_R_C(tmptopkifp);
	append_mc1(done);

	begin_instruction(GOTO_IF_NONPOSITIVE_R_C);
	COND_GOTO_R_C(tmptopkifnp);
	append_mc1(done);

	begin_instruction(GOTO_IF_EQUAL_R_R_C);
	COND_GOTO_CMP_R_R_C(tmptopkifz);
	append_mc1(done);

	begin_instruction(GOTO_IF_UNEQUAL_R_R_C);
	COND_GOTO_CMP_R_R_C(tmptopkifnz);
	append_mc1(done);

	begin_instruction(GOTO_IF_LESS_R_R_C);
	COND_GOTO_CMP_R_R_C(tmptopkifn);
	append_mc1(done);

	begin_instruction(GOTO_IF_GREATER_R_R_C);
	COND_GOTO_CMP_R_R_C(tmptopkifp);
	append_mc1(done);

	begin_instruction(GOTO_IF_LEQ_R_R_C);
	COND_GOTO_CMP_R_R_C(tmptopkifnp);
	append_mc1(done);

	begin_instruction(GOTO_IF_GEQ_R_R_C);
	COND_GOTO_CMP_R_R_C(tmptopkifnn);
	append_mc1(done);

	begin_instruction(GOTO_IF_EQUAL_R_C_C);
	COND_GOTO_CMP_R_C_C(tmptopkifz);
	append_mc1(done);

	begin_instruction(GOTO_IF_UNEQUAL_R_C_C);
	COND_GOTO_CMP_R_C_C(tmptopkifnz);
	append_mc1(done);

	begin_instruction(GOTO_IF_LESS_R_C_C);
	COND_GOTO_CMP_R_C_C(tmptopkifn);
	append_mc1(done);

	begin_instruction(GOTO_IF_GREATER_R_C_C);
	COND_GOTO_CMP_R_C_C(tmptopkifp);
	append_mc1(done);

	begin_instruction(GOTO_IF_LEQ_R_C_C);
	COND_GOTO_CMP_R_C_C(tmptopkifnp);
	append_mc1(done);

	begin_instruction(GOTO_IF_GEQ_R_C_C);
	COND_GOTO_CMP_R_C_C(tmptopkifnn);
	append_mc1(done);

	begin_instruction(GOTO_IF_LESS_UNSGN_R_R_C);
	COND_GOTO_CMP_UNSGN_R_R_C(tmptopkifn);
	append_mc1(done);

	begin_instruction(GOTO_IF_GREATER_UNSGN_R_R_C);
	COND_GOTO_CMP_UNSGN_R_R_C(tmptopkifp);
	append_mc1(done);

	begin_instruction(GOTO_IF_LEQ_UNSGN_R_R_C);
	COND_GOTO_CMP_UNSGN_R_R_C(tmptopkifnp);
	append_mc1(done);

	begin_instruction(GOTO_IF_GEQ_UNSGN_R_R_C);
	COND_GOTO_CMP_UNSGN_R_R_C(tmptopkifnn);
	append_mc1(done);

	begin_instruction(GOTO_IF_EQUAL_UNSGN_R_C_C);
	COND_GOTO_CMP_UNSGN_R_C_C(tmptopkifz);
	append_mc1(done);

	begin_instruction(GOTO_IF_UNEQUAL_UNSGN_R_C_C);
	COND_GOTO_CMP_UNSGN_R_C_C(tmptopkifnz);
	append_mc1(done);

	begin_instruction(GOTO_IF_LESS_UNSGN_R_C_C);
	COND_GOTO_CMP_UNSGN_R_C_C(tmptopkifn);
	append_mc1(done);

	begin_instruction(GOTO_IF_GREATER_UNSGN_R_C_C);
	COND_GOTO_CMP_UNSGN_R_C_C(tmptopkifp);
	append_mc1(done);

	begin_instruction(GOTO_IF_LEQ_UNSGN_R_C_C);
	COND_GOTO_CMP_UNSGN_R_C_C(tmptopkifnp);
	append_mc1(done);

	begin_instruction(GOTO_IF_GEQ_UNSGN_R_C_C);
	COND_GOTO_CMP_UNSGN_R_C_C(tmptopkifnn);
	append_mc1(done);

	begin_instruction(SWAP_R_R);
	append_mc1(regtodata(1));
	append_mc2(datatotmpa, regtodata(2));
	append_mc2(datatoreg(1), tmpatodata);
	append_mc1(datatoreg(2));
	append_mc1(done);

	/* SETS */
	begin_instruction(SET_PC_C);		// Set *constant to constant
	append_mc1(pkptroutinc(1));
	append_mc2(holddata, tmpctoaddr);
	append_mc1(writeRAM);
	append_mc1(done);
	
	begin_instruction(SET_PC_PC);		// Set *constant to *constant
	append_mc1(pkptroutinc(1));
	append_mc2(dataasaddr, readRAM);
	append_mc2(holddata, tmpctoaddr);
	append_mc1(writeRAM);
	append_mc1(done);
	
	begin_instruction(SET_PC_R);		// Set *constant to rN
	append_mc2(tmpctoaddr, regtodata(2));
	append_mc2(writeRAM, incrementpk(0));
	append_mc1(done);
	
	begin_instruction(SET_PC_PR);		// Set *constant to (*rN+offs)
	append_mc1(pkptroutinc(1));
	append_mc2(dataasoffs, regptodata_o(2));
	append_mc2(holddata, tmpctoaddr);
	append_mc1(writeRAM);
	append_mc1(done);

	begin_instruction(SET_PC_SR);		// Set *constant to srN
	append_mc2(tmpctoaddr, sregtodata(2));
	append_mc1(writeRAM);
	append_mc1(done);
	
	begin_instruction(SET_PC_PSR);		// Set *constant to *(srN+offs)
	append_mc1(pkptroutinc(1));
	append_mc2(dataasoffs, sregptodata_o(2));
	append_mc2(holddata, tmpctoaddr);
	append_mc1(writeRAM);
	append_mc1(done);

	begin_instruction(SET_R_C);		// Set rN to constant
	append_mc1(tmpctodata);
	append_mc2(datatoreg(1), incrementpk(0));
	append_mc1(done);
	
	begin_instruction(SET_R_PC);		// Set rN to *constant
	append_mc1(tmpctodata);
	append_mc2(holddataaddr, readRAM);
	append_mc2(datatoreg(1), incrementpk(0));
	append_mc1(done);

	begin_instruction(SET_R_R);		// Set rN to rM
	append_mc1(regtodata(2));
	append_mc1(datatoreg(1));
	append_mc1(done);
	
	begin_instruction(SET_R_PR);		// Set rN to *(rM+offs)
	append_mc2(tmpcasoffs, regptodata_o(2));
	append_mc2(datatoreg(1), incrementpk(0));
	append_mc1(done);

	begin_instruction(SET_R_SR);		// Set rN to srM
	append_mc1(sregtodata(2));
	append_mc1(datatoreg(1));
	append_mc1(done);
	
	begin_instruction(SET_R_PSR);		// Set rN to *(srM+offs)
	append_mc1(pkptroutinc(0));
	append_mc2(dataasoffs, sregptodata_o(2));
	append_mc1(datatoreg(1));
	append_mc1(done);

	begin_instruction(SET_PR_C);		// Set *(rN+offs) to constant
	append_mc2(tmpctooffs, pkptroutinc(1));
	append_mc2(holddata, regtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	begin_instruction(SET_PR_PC);		// Set *(rN+offs) to *constant
	append_mc1(pkptroutinc(1));
	append_mc2(dataasaddr, readRAM);
	append_mc2(holddata, regtoaddr(1));
	append_mc2(tmpcasoffs, writeRAM_o);
	append_mc1(done);

	begin_instruction(SET_PR_R);		// Set *(rN+offs) to rM
	append_mc2(tmpctooffs, incrementpk(0));
	append_mc2(regtodata(2), regtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	begin_instruction(SET_PR_PR);		// Set *(rN+offs) to *(rM+offs)
	append_mc1(pkptroutinc(1));
	append_mc2(dataasoffs, regptodata_o(2));
	append_mc2(holddata, regtoaddr(1));
	append_mc2(tmpcasoffs, writeRAM_o);
	append_mc1(done);
	
	begin_instruction(SET_PR_SR);		// Set *(rN+offs) to srM
	append_mc2(tmpctooffs, incrementpk(0));
	append_mc2(sregtodata(2), regtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	begin_instruction(SET_PR_PSR);		// Set *(rN+offs) to *(srM+offs)
	append_mc1(pkptroutinc(1));
	append_mc2(dataasoffs, sregptodata_o(2));
	append_mc2(holddata, regtoaddr(1));
	append_mc2(tmpcasoffs, writeRAM_o);
	append_mc1(done);

	begin_instruction(SET_SR_C);		// Set srN to constant
	append_mc1(tmpctodata);
	append_mc2(datatosreg(1), incrementpk(0));
	append_mc1(done);
	
	begin_instruction(SET_SR_PC);		// Set srN to *constant
	append_mc1(tmpctodata);
	append_mc2(holddataaddr, readRAM);
	append_mc2(datatosreg(1), incrementpk(0));
	append_mc1(done);

	begin_instruction(SET_SR_R);		// Set srN to rM
	append_mc1(regtodata(2));
	append_mc1(datatosreg(1));
	append_mc1(done);
	
	begin_instruction(SET_SR_PR);		// Set srN to *(rM+offs)
	append_mc1(pkptroutinc(0));
	append_mc2(dataasoffs, regptodata_o(2));
	append_mc1(datatosreg(1));
	append_mc1(done);

	begin_instruction(SET_SR_SR);		// Set srN to srM
	append_mc1(sregtodata(2));
	append_mc1(datatosreg(1));
	append_mc1(done);
	
	begin_instruction(SET_SR_PSR);		// Set srN to *(srM+offs)
	append_mc1(pkptroutinc(0));
	append_mc2(dataasoffs, sregptodata_o(2));
	append_mc1(datatosreg(1));
	append_mc1(done);

	begin_instruction(SET_PSR_C);		// Set *(srN+offs) to constant
	append_mc2(tmpctooffs, pkptroutinc(1));
	append_mc2(holddata, sregtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	begin_instruction(SET_PSR_PC);		// Set *(srN+offs) to *constant
	append_mc1(pkptroutinc(1));
	append_mc2(dataasaddr, readRAM);
	append_mc2(holddata, sregtoaddr(1));
	append_mc2(tmpcasoffs, writeRAM_o);
	append_mc1(done);

	begin_instruction(SET_PSR_R);		// Set *(srN+offs) to rM
	append_mc2(tmpctooffs, incrementpk(0));
	append_mc2(regtodata(2), sregtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	begin_instruction(SET_PSR_PR);		// Set *(srN+offs) to *(rM+offs)
	append_mc1(pkptroutinc(1));
	append_mc2(dataasoffs, regptodata_o(2));
	append_mc2(holddata, sregtoaddr(1));
	append_mc2(tmpcasoffs, writeRAM_o);
	append_mc1(done);
	
	begin_instruction(SET_PSR_SR);		// Set *(srN+offs) to srM
	append_mc2(tmpctooffs, incrementpk(0));
	append_mc2(sregtodata(2), sregtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	begin_instruction(SET_PSR_PSR);		// Set *(srN+offs) to *(srM+offs)
	append_mc1(pkptroutinc(1));
	append_mc2(dataasoffs, sregptodata_o(2));
	append_mc2(holddata, sregtoaddr(1));
	append_mc2(tmpcasoffs, writeRAM_o);
	append_mc1(done);
	
	// regex: code_sequence_for\((ADD|SUB|MUL|INC|DEC|NEG|CMP|UCMP|SGN|AND|OR|XOR|NOT|LSHIFT|RSHIFT|SHIFT)((_[RC])+)\);[^\n]*\n[^\n]*\n
	// replace with: begin_instruction(\1\2);\n\tALU\2(ALU_\1);\n
	
	begin_instruction(ADD_R_C);
	ALU_R_C(ALU_ADD);
	append_mc1(done);

	begin_instruction(SUB_R_C);
	ALU_R_C(ALU_SUB);
	append_mc1(done);

	begin_instruction(MUL_R_C);
	ALU_R_C(ALU_MUL);
	append_mc1(done);

	begin_instruction(DIV_R_C);
	ALU_R_C(ALU_DIV);
	append_mc1(done);

	begin_instruction(REM_R_C);
	ALU_R_C(ALU_REM);
	append_mc1(done);

	begin_instruction(UCMP_R_C);
	ALU_R_C(ALU_UCMP);
	append_mc1(done);

	begin_instruction(CMP_R_C);
	ALU_R_C(ALU_CMP);
	append_mc1(done);

	begin_instruction(AND_R_C);
	ALU_R_C(ALU_AND);
	append_mc1(done);

	begin_instruction(OR_R_C);
	ALU_R_C(ALU_OR);
	append_mc1(done);

	begin_instruction(XOR_R_C);
	ALU_R_C(ALU_XOR);
	append_mc1(done);

	begin_instruction(SHIFT_R_C);
	ALU_R_C(ALU_SHIFT);
	append_mc1(done);

	begin_instruction(ADD_R_R_C);
	ALU_R_R_C(ALU_ADD);
	append_mc1(done);

	begin_instruction(SUB_R_R_C);
	ALU_R_R_C(ALU_SUB);
	append_mc1(done);

	begin_instruction(MUL_R_R_C);
	ALU_R_R_C(ALU_MUL);
	append_mc1(done);

	begin_instruction(DIV_R_R_C);
	ALU_R_R_C(ALU_DIV);
	append_mc1(done);

	begin_instruction(REM_R_R_C);
	ALU_R_R_C(ALU_REM);
	append_mc1(done);

	begin_instruction(UCMP_R_R_C);
	ALU_R_R_C(ALU_UCMP);
	append_mc1(done);

	begin_instruction(CMP_R_R_C);
	ALU_R_R_C(ALU_CMP);
	append_mc1(done);

	begin_instruction(AND_R_R_C);
	ALU_R_R_C(ALU_AND);
	append_mc1(done);

	begin_instruction(OR_R_R_C);
	ALU_R_R_C(ALU_OR);
	append_mc1(done);

	begin_instruction(XOR_R_R_C);
	ALU_R_R_C(ALU_XOR);
	append_mc1(done);
	
	begin_instruction(SHIFT_R_R_C);
	ALU_R_R_C(ALU_SHIFT);
	append_mc1(done);

	begin_instruction(ADD_R_R);
	ALU_R_R(ALU_ADD);
	append_mc1(done);

	begin_instruction(SUB_R_R);
	ALU_R_R(ALU_SUB);
	append_mc1(done);

	begin_instruction(MUL_R_R);
	ALU_R_R(ALU_MUL);
	append_mc1(done);

	begin_instruction(DIV_R_R);
	ALU_R_R(ALU_DIV);
	append_mc1(done);

	begin_instruction(REM_R_R);
	ALU_R_R(ALU_REM);
	append_mc1(done);

	begin_instruction(INC_R);
	ALU_R(ALU_INC);
	append_mc1(done);

	begin_instruction(DEC_R);
	ALU_R(ALU_DEC);
	append_mc1(done);

	begin_instruction(UCMP_R_R);
	ALU_R_R(ALU_UCMP);
	append_mc1(done);

	begin_instruction(CMP_R_R);
	ALU_R_R(ALU_CMP);
	append_mc1(done);

	begin_instruction(NOT_R);
	ALU_R(ALU_NOT);
	append_mc1(done);

	begin_instruction(AND_R_R);
	ALU_R_R(ALU_AND);
	append_mc1(done);

	begin_instruction(OR_R_R);
	ALU_R_R(ALU_OR);
	append_mc1(done);

	begin_instruction(XOR_R_R);
	ALU_R_R(ALU_XOR);
	append_mc1(done);

	begin_instruction(LSHIFT_R);
	ALU_R(ALU_LSHIFT);
	append_mc1(done);

	begin_instruction(SHIFT_R_R);
	ALU_R_R(ALU_SHIFT);
	append_mc1(done);

	begin_instruction(RSHIFT_R);
	ALU_R(ALU_RSHIFT);
	append_mc1(done);

	begin_instruction(ADD_R_R_R);
	ALU_R_R_R(ALU_ADD);
	append_mc1(done);

	begin_instruction(SUB_R_R_R);
	ALU_R_R_R(ALU_SUB);
	append_mc1(done);

	begin_instruction(MUL_R_R_R);
	ALU_R_R_R(ALU_MUL);
	append_mc1(done);

	begin_instruction(DIV_R_R_R);
	ALU_R_R_R(ALU_DIV);
	append_mc1(done);

	begin_instruction(REM_R_R_R);
	ALU_R_R_R(ALU_REM);
	append_mc1(done);

	begin_instruction(INC_R_R);
	ALU_R_R_C(ALU_INC);
	append_mc1(done);

	begin_instruction(DEC_R_R);
	ALU_R_R_C(ALU_DEC);
	append_mc1(done);

	begin_instruction(UCMP_R_R_R);
	ALU_R_R_R(ALU_UCMP);
	append_mc1(done);

	begin_instruction(CMP_R_R_R);
	ALU_R_R_R(ALU_CMP);
	append_mc1(done);

	begin_instruction(AND_R_R_R);
	ALU_R_R_R(ALU_AND);
	append_mc1(done);

	begin_instruction(OR_R_R_R);
	ALU_R_R_R(ALU_OR);
	append_mc1(done);

	begin_instruction(XOR_R_R_R);
	ALU_R_R_R(ALU_XOR);
	append_mc1(done);

	begin_instruction(NOT_R_R);
	ALU_R_R_C(ALU_NOT);
	append_mc1(done);

	begin_instruction(LSHIFT_R_R);
	ALU_R_R_C(ALU_LSHIFT);
	append_mc1(done);

	begin_instruction(SHIFT_R_R_R);
	ALU_R_R_R(ALU_SHIFT);
	append_mc1(done);

	begin_instruction(RSHIFT_R_R);
	ALU_R_R_C(ALU_RSHIFT);
	append_mc1(done);

	//**stack stuff**//

	// pushes
	begin_instruction(PUSH);
	append_mc1(decrementsp(0));
	append_mc1(done);
	
	begin_instruction(PUSH_C);		// push constant
	append_mc2(tmpctodata, predecsptoaddr);
	append_mc2(writeRAM, incrementpk(0));
	append_mc1(done);
	
	begin_instruction(PUSH_PC);		// push *constant
	append_mc2(tmpcptodata, predecsptoaddr);
	append_mc2(writeRAM, incrementpk(0));
	append_mc1(done);
	
	begin_instruction(PUSH_R);		// push rN
	append_mc2(regtodata(1), predecsptoaddr);
	append_mc1(writeRAM);
	append_mc1(done);
	
	begin_instruction(PUSH_PR);		// push *(rN+offs)
	append_mc2(incrementpk(0), regptodata(1));
	append_mc2(holddata, predecsptoaddr);
	append_mc2(tmpcasoffs, writeRAM_o);
	append_mc1(done);
	
	begin_instruction(PUSH_SR);		// push srN
	append_mc2(predecsptoaddr, sregtodata(1));
	append_mc1(writeRAM);
	append_mc1(done);
	
	begin_instruction(PUSH_PSR);		// push *(srN+offs)
	append_mc2(incrementpk(0), sregptodata(1));
	append_mc2(holddata, predecsptoaddr);
	append_mc2(tmpcasoffs, writeRAM_o);
	append_mc1(done);

	// pops
	begin_instruction(POP);		// pop
	append_mc1(incrementsp(0));
	append_mc1(done);
	
	begin_instruction(POP_PC);		// pop *constant
	append_mc2(incrementpk(0), postincspptrout);
	append_mc2(holddata, tmpctoaddr);
	append_mc1(writeRAM);
	append_mc1(done);
	
	begin_instruction(POP_R);		// pop rN
	append_mc1(postincspptrout);
	append_mc1(datatoreg(1));
	append_mc1(done);
	
	begin_instruction(POP_PR);		// pop *(rN+offs)
	append_mc2(incrementpk(0), postincspptrout);
	append_mc2(holddata, regtoaddr(1));
	append_mc2(tmpcasoffs, writeRAM_o);
	append_mc1(done);
	
	begin_instruction(POP_SR);		// pop srN
	append_mc1(postincspptrout);
	append_mc1(datatosreg(1));
	append_mc1(done);
	
	begin_instruction(POP_PSR);		// pop *(srN+offs)
	append_mc2(incrementpk(0), postincspptrout);
	append_mc2(holddata, sregtoaddr(1));
	append_mc2(tmpcasoffs, writeRAM_o);
	append_mc1(done);

	/* FUNCTION STUFF */
	
	begin_instruction(CALL_C);
	append_mc1(incrementpk(0));
	append_mc2(pktodata, predecsptoaddr);
	append_mc1(writeRAM);
	append_mc2(tmpctosreg, tmptopk);
	append_mc1(stacking);
	append_mc1(done);
	
	begin_instruction(FASTCALL_C);
	append_mc1(incrementpk(0));
	append_mc2(pktodata, predecsptoaddr);
	append_mc1(writeRAM);
	append_mc2(tmpctosreg, tmptopk);
	append_mc1(predecsptoaddr);
	append_mc1(writeRAM);
	append_mc1(done);
	
	begin_instruction(RETURN);		// return
	append_mc2(preunstacking, postincspptrout); // put the stacking mask on the data bus for the unstacker
	append_mc1(unstacking);
	append_mc2(spptodata, incrementsp(0));
	append_mc1(datatopk);
	append_mc1(done);
	
	// I/O pins
	begin_instruction(PINMODE_INPUT_C);		// pinmode_input constant
	append_mc1(pinmodein(1));
	append_mc1(done);
	
	begin_instruction(PINMODE_OUTPUT_C);		// pinmode_output constant
	append_mc1(pinmodeout(1));
	append_mc1(done);
	
	
	begin_instruction(PINMODE_INPUT_R);		// pinmode_input rN
	append_mc1(regtodata(1));
	append_mc1(datatotmpc);
	append_mc1(pinmodein(1));
	append_mc1(done);
	
	begin_instruction(PINMODE_OUTPUT_R);		// pinmode_output rN
	append_mc1(regtodata(1));
	append_mc1(datatotmpc);
	append_mc1(pinmodeout(1));
	append_mc1(done);
	

	begin_instruction(SET_PIN_LOW_C);		// set_pin_low constant
	append_mc1(setpinlow(1));
	append_mc1(done);
	
	begin_instruction(SET_PIN_HIGH_C);		// set_pin_high constant
	append_mc1(setpinhigh(1));
	append_mc1(done);

	begin_instruction(SET_PIN_LOW_R);		// set_pin_low rN
	append_mc1(setpinlow(1));
	append_mc1(done);
	
	begin_instruction(SET_PIN_HIGH_R);		// set_pin_high rN
	append_mc1(regtodata(1));
	append_mc1(datatotmpc);
	append_mc1(setpinhigh(11));
	append_mc1(done);

	begin_instruction(SET_PIN_LOW_R);		// set_pin_low rN
	append_mc1(regtodata(1));
	append_mc1(datatotmpc);
	append_mc1(setpinlow(11));
	append_mc1(done);
	
	begin_instruction(SET_PIN_HIGH_R);		// set_pin_high rN
	append_mc1(setpinhigh(1));
	append_mc1(done);
	
	begin_instruction(READ_PIN_C_R);		// read_pin, rN constant
	append_mc1(pintodata(1));
	append_mc1(datatoreg(2));
	append_mc1(done);
	
	begin_instruction(WRITE_PIN_C_R);		// write_pin constant, rN
	append_mc1(regtodata(2));
	append_mc1(datatopin(1));
	append_mc1(done);
	
	begin_instruction(READ_PIN_R_R);		// read_p, rN rM
	append_mc1(regtodata(1));
	append_mc1(datatotmpc);
	append_mc1(pintodata(11));
	append_mc1(datatoreg(2));
	append_mc1(done);
	
	begin_instruction(WRITE_PIN_R_R);		// write_pin rN, rM
	append_mc1(regtodata(1));
	append_mc1(datatotmpc);
	append_mc1(datatopin(11));
	append_mc1(done);
	
	begin_instruction(PRINTS_C_C);
	append_mc1(pkptroutinc(1));
	append_mc2(tmpctodata, holddataaddr);
	append_mc1(writeRAM);
	append_mc1(pinmodeout(3) & ~MIX_IN);
	append_mc1(setpinhigh(3) & ~MIX_IN);
	append_mc1(setpinlow(3) & ~MIX_IN);
	append_mc1(done);
	
	begin_instruction(PRINTS_PR_C);
	append_mc1(pkptroutinc(1));
	append_mc1(datatooffs);
	append_mc2(regtodata(1), tmpctoaddr);
	append_mc1(writeRAM_o);
	append_mc1(pinmodeout(3) & ~MIX_IN);
	append_mc1(setpinhigh(3) & ~MIX_IN);
	append_mc1(setpinlow(3) & ~MIX_IN);
	append_mc1(done);
	
	
	begin_instruction(HANDLE_INTERRUPT);
	append_mc2(decrementsp(0), decrementpk(0));
	append_mc2(pktodata, sptoaddr);
	append_mc2(writeRAM, it_to_pk);
	append_mc1(stacking);
	append_mc1(done);
	
	begin_instruction(SYSCALL_C);
	append_mc1(incrementpk(0));
	append_mc2(pktodata, predecsptoaddr);
	append_mc2(writeRAM, sctaddrout);
	append_mc1(datatooffs);
	append_mc2(tmpctoaddr, readRAM_o);
	append_mc1(datatopk);
	append_mc1(stacking);
	append_mc1(done);
}

int main(int argc, char* argv[])
{
	FILE* mainROM1 = fopen("roms/main_rom_1", "wb");
	if (!mainROM1)
	{
		perror("Error opening main_rom_1 file");
		exit(EXIT_FAILURE);
	}
	
	FILE* mainROM2 = fopen("roms/main_rom_2", "wb");
	if (!mainROM2)
	{
		perror("Error opening main_rom_2 file");
		exit(EXIT_FAILURE);
	}

	FILE* decoderROM = fopen("roms/decoder_rom", "wb");
	if (!decoderROM)
	{
		perror("Error opening decoder_rom file");
		exit(EXIT_FAILURE);
	}
	
	FILE* printerROM = fopen("roms/printer_rom", "w");
	if (!printerROM)
	{
		perror("Error opening printer_rom file");
		exit(EXIT_FAILURE);
	}

	generate_roms();

	writeout_v3_hex_addressed(mr1_data, current_position, mainROM1);
	writeout_v3_hex_addressed(mr2_data, current_position, mainROM2);
	writeout_v3_hex_addressed(dr_data, 256, decoderROM);
	writeout_v3_hex_addressed(pr_data, 8192, printerROM);
	
	fclose(mainROM1);
	fclose(mainROM2);
	fclose(decoderROM);
	fclose(printerROM);

	return 0;
}
