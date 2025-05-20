#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dunkprog.h"
#include "microops.h"
#include "opcodes.h"

uint16_t mr1_data[BINSIZE];
uint16_t mr2_data[BINSIZE];
uint16_t  dr_data[256];
	
int current_position = 0;

void append_mc1(uint16_t v1)
{
	mr1_data[current_position] = v1;
	mr2_data[current_position] = 0x0000;
	current_position += 1;
}

void append_mc2(uint16_t v1, uint16_t v2)
{
	mr1_data[current_position] = v1;
	mr2_data[current_position] = v2;
	current_position += 1;
}

void generate_roms() {
	// Initialise everything to 0 first, of course.
	for (int i = 0; i < NWORDS; i++) {
		mr1_data[i] = 0;
		mr2_data[i] = 0;
	}
	for (int i = 0; i < 256; i++) {
		dr_data[i] = 0;
	}
	
	// After an instruction completes, the micro-code address
	// register is reset to 0, and the CPU executes the following,
	// fetching and decoding the next instruction and jumping to
	// the appropriate point in the micro-code ROMs to execute it
	append_mc1(pkptroutinc);
	append_mc1(begini);
	
	code_sequence_for(CHILL);		// chill
	append_mc1(do_nothing);
	append_mc1(done);

	code_sequence_for(GOTO);		// goto
	append_mc1(pkptrout);
	append_mc1(datatopk);
	append_mc1(done);
	
	code_sequence_for(GOTO_IF_ZERO);		// goto_if_zero
	append_mc2(incrementpk, regtodata(1));
	append_mc2(tmpctosreg, tmptopkifz);
	append_mc1(done);
	
	code_sequence_for(GOTO_IF_NONZERO);		// goto_if_nonzero
	append_mc2(incrementpk, regtodata(1));
	append_mc2(tmpctosreg, tmptopkifnz);
	append_mc1(done);
	
	code_sequence_for(GOTO_IF_NEGATIVE);		// goto_if_negative
	append_mc2(incrementpk, regtodata(1));
	append_mc2(tmpatosreg, tmptopkifn);
	append_mc1(done);
	
	code_sequence_for(GOTO_IF_NONNEGATIVE);		// goto_if_nonnegative
	append_mc2(incrementpk, regtodata(1));
	append_mc2(tmpctosreg, tmptopkifnn);
	append_mc1(done);
	
	code_sequence_for(GOTO_IF_POSITIVE);		// goto_if_positive
	append_mc2(incrementpk, regtodata(1));
	append_mc2(tmpctosreg, tmptopkifp);
	append_mc1(done);
	
	code_sequence_for(GOTO_IF_NONPOSITIVE);		// goto_if_nonpositive
	append_mc2(incrementpk, regtodata(1));
	append_mc2(tmpctosreg, tmptopkifnp);
	append_mc1(done);

	code_sequence_for(GOTO_IF_EQUAL); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(GOTO_IF_UNEQUAL); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(GOTO_IF_LESS); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(GOTO_IF_GREATER); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(GOTO_IF_LEQ); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(GOTO_IF_GEQ); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(SWAP_R_R);
	append_mc1(regtodata(1));
	append_mc2(datatotmpa, regtodata(2));
	append_mc2(datatoreg(1), tmpatodata);
	append_mc1(datatoreg(2));
	append_mc1(done);

	/* SETS */
	code_sequence_for(SET_PC_C);		// Set *constant to constant
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, pkptroutinc);
	append_mc2(datatotmpb, tmpatoaddr);
	append_mc2(tmpbtodata, holdaddr);
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(SET_PC_PC);		// Set *constant to *constant
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, pkptroutinc);
	append_mc2(datatotmpb, holddataaddr);
	append_mc1(readRAM);
	append_mc1(done);

	code_sequence_for(SET_PC_R);		// Set *constant to rN
	append_mc1(pkptroutinc);
	append_mc2(holddataaddr, regtodata(1));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(SET_PC_PR);		// Set *constant to *rN
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, regptodata(1));
	append_mc1(datatotmpb);
	append_mc2(tmpatoaddr, tmpbtodata);
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(SET_PC_PRO);		// Set *constant to (*rN+offs)
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, pkptrout);
	append_mc1(datatooffs);
	append_mc1(regptodata_o(1));
	append_mc2(holddata, tmpatoaddr);
	append_mc1(writeRAM);
	append_mc1(done);

	code_sequence_for(SET_PC_SR);		// Set *constant to srN
	append_mc1(pkptroutinc);
	append_mc2(holddataaddr, sregtodata(1));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(SET_PC_PSR);		// Set *constant to *srN
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, sregptodata(1));
	append_mc1(datatotmpb);
	append_mc2(tmpatoaddr, tmpbtodata);
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(SET_PC_PSRO);		// Set *constant to *(srN+offs)
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(sregptodata_o(1));
	append_mc2(holddata, tmpatoaddr);
	append_mc1(writeRAM);
	append_mc1(done);

	code_sequence_for(SET_R_C);		// Set rN to constant
	append_mc1(pkptroutinc);
	append_mc1(datatoreg(1));
	append_mc1(done);
	
	code_sequence_for(SET_R_PC);		// Set rN to *constant
	append_mc1(pkptroutinc);
	append_mc2(holddataaddr, readRAM);
	append_mc1(datatoreg(1));
	append_mc1(done);

	code_sequence_for(SET_R_R);		// Set rN to rM
	append_mc1(regtodata(2));
	append_mc1(datatoreg(1));
	append_mc1(done);
	
	code_sequence_for(SET_R_PR);		// Set rN to *rM
	append_mc1(regptodata(2));
	append_mc1(datatoreg(1));
	append_mc1(done);
	
	code_sequence_for(SET_R_PRO);		// Set rN to *(rM+offs)
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(regptodata_o(2));
	append_mc1(datatoreg(1));
	append_mc1(done);

	code_sequence_for(SET_R_SR);		// Set rN to srM
	append_mc1(sregtodata(2));
	append_mc1(datatoreg(1));
	append_mc1(done);
	
	code_sequence_for(SET_R_PSR);		// Set rN to *srM
	append_mc1(sregptodata(2));
	append_mc1(datatoreg(1));
	append_mc1(done);
	
	code_sequence_for(SET_R_PSRO);		// Set rN to *(srM+offs)
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(sregptodata_o(2));
	append_mc1(datatoreg(1));
	append_mc1(done);

	code_sequence_for(SET_PR_C);		// Set *rN to constant
	append_mc1(pkptroutinc);
	append_mc2(holddata, regtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(SET_PR_PC);		// Set *rN to *constant
	append_mc1(pkptroutinc);
	append_mc2(holddataaddr, readRAM);
	append_mc2(holddata, regtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);

	code_sequence_for(SET_PR_R);		// Set *rN to rM
	append_mc2(regtoaddr(1), regtodata(2));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(SET_PR_PR);		// Set *rN to *rM
	append_mc1(regptodata(2));
	append_mc2(holddata, regtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(SET_PR_PRO);		// Set *rN to (*rM+offs)
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(regptodata_o(2));
	append_mc2(holddata, regtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);

	code_sequence_for(SET_PR_SR);		// Set *rN to srM
	append_mc2(regtoaddr(1), sregtodata(2));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(SET_PR_PSR);		// Set *rN to *srM
	append_mc1(sregptodata(2));
	append_mc2(holddata, regtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(SET_PR_PSRO);		// Set *rN to *(srM+offs)
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(sregptodata_o(2));
	append_mc2(holddata, regtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);

	code_sequence_for(SET_PRO_C);		// Set *(rN+offs) to constant
	append_mc1(pkptroutinc);
	append_mc2(datatooffs, pkptroutinc);
	append_mc1(datatotmpa);
	append_mc2(regtoaddr(1), holdaddr);
	append_mc2(tmpatodata, holdaddr);
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(SET_PRO_PC);		// Set *(rN+offs) to *constant
	append_mc1(pkptroutinc);
	append_mc2(datatooffs, pkptroutinc);
	append_mc2(holddataaddr, readRAM);
	append_mc2(holddata, regtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);

	code_sequence_for(SET_PRO_R);		// Set *(rN+offs) to rM
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc2(regtodata(2), regtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(SET_PRO_PR);		// Set *(rN+offs) to *rM
	append_mc1(pkptroutinc);
	append_mc2(datatooffs, regptodata(2));
	append_mc2(holddata, regtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(SET_PRO_PRO);		// Set *(rN+offs) to *(rM+offs)
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(regptodata_o(2));
	append_mc2(datatotmpa, pkptroutinc);
	append_mc1(datatooffs);
	append_mc2(tmpatodata, regtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(SET_PRO_SR);		// Set *(rN+offs) to srM
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc2(sregtodata(2), regtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(SET_PRO_PSR);		// Set *(rN+offs) to *srM
	append_mc1(pkptroutinc);
	append_mc2(datatooffs, sregptodata(2));
	append_mc2(holddata, regtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(SET_PRO_PSRO);		// Set *(rN+offs) to *(srM+offs)
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(sregptodata_o(2));
	append_mc2(datatotmpb, tmpatodata);
	append_mc1(datatooffs);
	append_mc2(tmpbtodata, regtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);

	code_sequence_for(SET_SR_C);		// Set srN to constant
	append_mc1(pkptroutinc);
	append_mc1(datatosreg(1));
	append_mc1(done);
	
	code_sequence_for(SET_SR_PC);		// Set srN to *constant
	append_mc1(pkptroutinc);
	append_mc2(holddataaddr, readRAM);
	append_mc1(datatosreg(1));
	append_mc1(done);

	code_sequence_for(SET_SR_R);		// Set srN to rM
	append_mc1(regtodata(2));
	append_mc1(datatosreg(1));
	append_mc1(done);
	
	code_sequence_for(SET_SR_PR);		// Set srN to *rM
	append_mc1(regtoaddr(2));
	append_mc1(readRAM);
	append_mc1(datatosreg(1));
	append_mc1(done);
	
	code_sequence_for(SET_SR_PRO);		// Set srN to (*rM+offs)
	append_mc1(pkptroutinc);
	append_mc2(datatooffs, regtoaddr(2));
	append_mc1(readRAM_o);
	append_mc1(datatosreg(1));
	append_mc1(done);
	
	code_sequence_for(SET_SR_SR);		// Set srN to srM
	append_mc1(sregtodata(2));
	append_mc1(datatosreg(1));
	append_mc1(done);
	
	code_sequence_for(SET_SR_PSR);		// Set srN to *srM
	append_mc1(sregtoaddr(2));
	append_mc1(readRAM);
	append_mc1(datatosreg(1));
	append_mc1(done);
	
	code_sequence_for(SET_SR_PSRO);		// Set srN to *(srM+offs)
	append_mc1(pkptroutinc);
	append_mc2(datatooffs, sregtoaddr(2));
	append_mc1(readRAM_o);
	append_mc1(datatosreg(1));
	append_mc1(done);

	code_sequence_for(SET_PSR_C);		// Set *srN to constant
	append_mc1(pkptroutinc);
	append_mc2(holddata, sregtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(SET_PSR_PC);		// Set *srN to *constant
	append_mc1(pkptroutinc);
	append_mc2(holddataaddr, readRAM);
	append_mc2(holddata, sregtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);

	code_sequence_for(SET_PSR_R);		// Set *srN to rM
	append_mc2(regtodata(2), sregtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(SET_PSR_PR);		// Set *srN to *rM
	append_mc1(regptodata(2));
	append_mc2(holddata, sregtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(SET_PSR_PRO);		// Set *srN to (*rM+offs)
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(regptodata_o(2));
	append_mc2(holddata, sregtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);

	code_sequence_for(SET_PSR_SR);		// Set *srN to srM
	append_mc2(sregtodata(2), sregtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(SET_PSR_PSR);		// Set *srN to *srM
	append_mc1(sregptodata(2));
	append_mc2(holddata, sregtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(SET_PSR_PSRO);		// Set *srN to *(srM+offs)
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(sregptodata_o(2));
	append_mc2(holddata, sregtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);

	code_sequence_for(SET_PSRO_C);		// Set *(srN+offs) to constant
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(pkptroutinc);
	append_mc2(holddata, sregtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(SET_PSRO_PC);		// Set *(srN+offs) to *constant
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(pkptroutinc);
	append_mc2(holddataaddr, readRAM);
	append_mc2(holddata, sregtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);

	code_sequence_for(SET_PSRO_R);		// Set *(srN+offs) to rM
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc2(regtodata(2), sregtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(SET_PSRO_PR);		// Set *(srN+offs) to *rM
	append_mc1(pkptroutinc);
	append_mc2(datatooffs, regptodata(2));
	append_mc2(holddata, sregtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(SET_PSRO_PRO);		// Set *(srN+offs) to *(rM+offs)
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, pkptroutinc);
	append_mc2(datatooffs, regptodata_o(2));
	append_mc2(datatotmpb, tmpatodata);
	append_mc1(datatooffs);
	append_mc2(tmpbtodata, sregtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(SET_PSRO_SR);		// Set *(srN+offs) to srM
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc2(sregtodata(2), sregtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(SET_PSRO_PSR);		// Set *(srN+offs) to *srM
	append_mc1(pkptroutinc);
	append_mc2(datatooffs, sregptodata(2));
	append_mc2(holddata, sregtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(SET_PSRO_PSRO);		// Set *(srN+offs) to *(srM+offs)
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(sregptodata_o(2));
	append_mc2(datatotmpb, tmpatodata);
	append_mc1(datatooffs);
	append_mc2(tmpbtodata, sregtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	

	code_sequence_for(ADD_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(SUB_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(MUL_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(UCMP_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(CMP_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(AND_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(OR_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(XOR_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(SHIFT_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(ADD_R_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(SUB_R_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(MUL_R_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(INC_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(DEC_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(NEG_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(UCMP_R_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(CMP_R_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(SGN_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(AND_R_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(OR_R_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(XOR_R_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(LSHIFT_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(SHIFT_R_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(RSHIFT_R_C); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(ADD_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(SUB_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(MUL_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(INC_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(DEC_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(NEG_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(UCMP_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(CMP_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(SGN_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(AND_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(OR_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(XOR_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(LSHIFT_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(SHIFT_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(RSHIFT_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(ADD_R_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(SUB_R_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(MUL_R_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(INC_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(DEC_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(NEG_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(UCMP_R_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(CMP_R_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(SGN_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(AND_R_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(OR_R_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(XOR_R_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(NOT_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(LSHIFT_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(SHIFT_R_R_R); // unimplemented
	// TODO
	append_mc1(done);

	code_sequence_for(RSHIFT_R_R); // unimplemented
	// TODO
	append_mc1(done);

	//**ALU**//

	
	//**stack stuff**//

	// pushes
	code_sequence_for(PUSH_C);		// push constant
	append_mc2(pkptroutinc, decrementsp);
	append_mc2(holddata, sptoaddr);
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(PUSH_PC);		// push *constant
	append_mc2(pkptroutinc, decrementsp);
	append_mc2(holddata, sptoaddr);
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(PUSH_R);		// push rN
	append_mc1(decrementsp);
	append_mc2(sptoaddr, regtodata(1));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(PUSH_PR);		// push *rN
	append_mc2(regptodata(1), decrementsp);
	append_mc2(holddata, sptoaddr);
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(PUSH_PRO);		// push *(rN+offs)
	append_mc2(pkptroutinc, decrementsp);
	append_mc2(datatooffs, regptodata(1));
	append_mc2(holddata, sptoaddr);
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(PUSH_SR);		// push srN
	append_mc1(decrementsp);
	append_mc2(sptoaddr, sregtodata(1));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(PUSH_PSR);		// push *srN
	append_mc2(sregptodata(1), decrementsp);
	append_mc2(holddata, sptoaddr);
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(PUSH_PSRO);		// push *(srN+offs)
	append_mc2(pkptroutinc, decrementsp);
	append_mc2(datatooffs, sregtodata(1));
	append_mc2(holddata, sptoaddr);
	append_mc1(writeRAM_o);
	append_mc1(done);

	// pops
	code_sequence_for(POP_PSR);		// pop
	append_mc1(incrementsp);
	append_mc1(done);
	
	code_sequence_for(POP_PC);		// pop *constant
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, spptodata);
	append_mc2(holddata, tmpatoaddr);
	append_mc2(writeRAM, incrementsp);
	append_mc1(done);
	
	code_sequence_for(POP_R);		// pop rN
	append_mc1(spptodata);
	append_mc2(datatoreg(1), incrementsp);
	append_mc1(done);
	
	code_sequence_for(POP_PR);		// pop *rN
	append_mc1(spptodata);
	append_mc2(holddata, regtoaddr(1));
	append_mc2(writeRAM, incrementsp);
	append_mc1(done);
	
	code_sequence_for(POP_PRO);		// pop *(rN+offs)
	append_mc1(pkptroutinc);
	append_mc2(datatooffs, spptodata);
	append_mc2(holddata, regtoaddr(1));
	append_mc2(writeRAM_o, incrementsp);
	append_mc1(done);
	
	code_sequence_for(POP_SR);		// pop srN
	append_mc1(spptodata);
	append_mc2(datatosreg(1), incrementsp);
	append_mc1(done);
	
	code_sequence_for(POP_PSRO);		// pop *(srN+offs)
	append_mc1(pkptroutinc);
	append_mc2(datatooffs, spptodata);
	append_mc2(holddata, sregtoaddr(1));
	append_mc2(writeRAM_o, incrementsp);
	append_mc1(done);

	// function stuff
	code_sequence_for(CALL);		 // call
	append_mc2(pkptroutinc, decrementsp);
	append_mc2(datatotmpa, pktodata);
	append_mc2(holddata, sptoaddr);
	append_mc2(writeRAM, tmpatopk);
	append_mc1(stacking);
	append_mc1(done);
	
	code_sequence_for(RETURN);		// return
	append_mc2(spptodata, incrementsp);
	append_mc1(unstacking);
	append_mc2(spptodata, incrementsp);
	append_mc1(datatopk);
	append_mc1(done);
	
	// I/O pins
	code_sequence_for(PINMODE_INPUT_C);		// pinmode_input constant
	append_mc1(pinmodein(1));
	append_mc1(done);
	
	code_sequence_for(PINMODE_OUTPUT_C);		// pinmode_output constant
	append_mc1(pinmodeout(1));
	append_mc1(done);
	
	
	code_sequence_for(PINMODE_INPUT_R);		// pinmode_input rN
	append_mc1(regtodata(1));
	append_mc1(datatotmpc);
	append_mc1(pinmodein(1));
	append_mc1(done);
	
	code_sequence_for(PINMODE_OUTPUT_R);		// pinmode_output rN
	append_mc1(regtodata(1));
	append_mc1(datatotmpc);
	append_mc1(pinmodeout(1));
	append_mc1(done);
	

	code_sequence_for(SET_PIN_LOW_C);		// set_pin_low constant
	append_mc1(setpinlow(1));
	append_mc1(done);
	
	code_sequence_for(SET_PIN_HIGH_C);		// set_pin_high constant
	append_mc1(setpinhigh(1));
	append_mc1(done);

	code_sequence_for(SET_PIN_LOW_R);		// set_pin_low rN
	append_mc1(setpinlow(1));
	append_mc1(done);
	
	code_sequence_for(SET_PIN_HIGH_R);		// set_pin_high rN
	append_mc1(regtodata(1));
	append_mc1(datatotmpc);
	append_mc1(setpinhigh(11));
	append_mc1(done);

	code_sequence_for(SET_PIN_LOW_R);		// set_pin_low rN
	append_mc1(regtodata(1));
	append_mc1(datatotmpc);
	append_mc1(setpinlow(11));
	append_mc1(done);
	
	code_sequence_for(SET_PIN_HIGH_R);		// set_pin_high rN
	append_mc1(setpinhigh(1));
	append_mc1(done);
	
	code_sequence_for(READ_PIN_C_R);		// read_pin, rN constant
	append_mc1(pintodata(1));
	append_mc1(datatoreg(2));
	append_mc1(done);
	
	code_sequence_for(WRITE_PIN_C_R);		// write_pin constant, rN
	append_mc1(regtodata(2));
	append_mc1(datatopin(1));
	append_mc1(done);
	
	code_sequence_for(READ_PIN_R_R);		// read_p, rN rM
	append_mc1(regtodata(1));
	append_mc1(datatotmpc);
	append_mc1(pintodata(11));
	append_mc1(datatoreg(2));
	append_mc1(done);
	
	code_sequence_for(WRITE_PIN_R_R);		// write_pin rN, rM
	append_mc1(regtodata(1));
	append_mc1(datatotmpc);
	append_mc1(datatopin(11));
	append_mc1(done);
}

int main(int argc, char* argv[])
{
	FILE* mainROM1 = fopen("roms/main_rom_1", "wb");
	if (!mainROM1) {
		perror("Error opening main_rom_1 file");
		exit(EXIT_FAILURE);
	}
	
	FILE* mainROM2 = fopen("roms/main_rom_2", "wb");
	if (!mainROM2) {
		perror("Error opening main_rom_2 file");
		exit(EXIT_FAILURE);
	}

	FILE* decoderROM = fopen("roms/decoder_rom", "wb");
	if (!decoderROM) {
		perror("Error opening decoder_rom file");
		exit(EXIT_FAILURE);
	}

	generate_roms();

	fwrite(mr1_data, 2, current_position, mainROM1);
	fclose(mainROM1);
	fwrite(mr2_data, 2, current_position, mainROM2);
	fclose(mainROM2);
	fwrite(dr_data, 2, 256, decoderROM);
	fclose(decoderROM);

	return 0;
}
