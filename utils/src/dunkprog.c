#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_REGISTERS 16
#define N_SPECIAL_REGS 5

#define N_PINS 16

#define N_ALU_OPS 8

#define MIX_IN				0b0000000010000000

#define do_nothing 			0x0000
#define begini 				0x0001

#define sregtodata(X) 	 (0x0002 + 0x1000 * X) | MIX_IN
#define sregtoaddr(X) 	 (0x0003 + 0x1000 * X) | MIX_IN
#define sregptodata(X) 	 (0x0004 + 0x1000 * X) | MIX_IN
#define sregptodata_o(X) (0x0005 + 0x1000 * X) | MIX_IN
#define datatosreg(X) 	 (0x0006 + 0x1000 * X) | MIX_IN

#define pktodata 		0x0002
#define pktoaddr 		0x0003
#define pkptrout 		0x0004
#define pkptrout_o		0x0005
#define datatopk 		0x0006

#define sptodata 		0x1002
#define sptoaddr 		0x1003
#define spptodata 		0x1004
#define spptodata_o		0x1005
#define datatosp 		0x1006

#define tmpatodata 		0x2002
#define tmpatoaddr 		0x2003
#define tmpaptodata 	0x2004
#define tmpaptodata_o 	0x2005
#define datatotmpa 		0x2006

#define tmpbtodata 		0x3002
#define tmpbtoaddr 		0x3003
#define tmpbptodata 	0x3004
#define tmpbptodata_o 	0x3005
#define datatotmpb 		0x3006

#define offstodata 		0x4002
#define offstoaddr 		0x4003
#define offsptodata 	0x4004
#define offsptodata_o 	0x4005
#define datatooffs 		0x4006

#define incrementpk 	0x0010

#define pkptroutinc		0x0011

#define tmpatopk 		0x0012
#define tmpbtopk 		0x0013

#define tmpatopkifz 	0x0014
#define tmpatopkifnz 	0x0015

#define tmpatopkifn 	0x0016
#define tmpatopkifnn 	0x0017

#define tmpatopkifp 	0x0018
#define tmpatopkifnp 	0x0019


#define incrementsp 	0x1010
#define decrementsp 	0x1011


#define regtodata(X) 	(0x0020 + 0x1000 * X) | MIX_IN
#define regtoaddr(X) 	(0x0021 + 0x0100 * X) | MIX_IN
#define regptodata(X) 	(0x0022 + 0x0100 * X) | MIX_IN
#define regptodata_o(X) (0x0023 + 0x0100 * X) | MIX_IN
#define datatoreg(X) 	(0x0024 + 0x1000 * X) | MIX_IN

#define writeRAM 		0x0025
#define writeRAM_o 		0x0026

#define readRAM 		0x0027
#define readRAM_o 		0x0028

#define pinmodein(N)  	(0x0030 + 0x1000 * N) | MIX_IN
#define pinmodeout(N) 	(0x0031 + 0x1000 * N) | MIX_IN
#define setpinlow(N)  	(0x0032 + 0x1000 * N) | MIX_IN
#define setpinhigh(N) 	(0x0033 + 0x1000 * N) | MIX_IN
#define pintodata(N) 	(0x0034 + 0x1000 * N) | MIX_IN
#define datatopin(N)  	(0x0035 + 0x1000 * N) | MIX_IN

#define ALUop(X, Y, o) 	(0x0040 + 0x1000 * X + 0x0100 * Y + 0x0001 * o) | MIX_IN

#define holddata 		0x0050
#define holdaddr	 	0x0051

#define holdaddrdata 	0x0052
#define holddataaddr 	0x0053

#define done  0xfffe
#define reset 0xffff

#define BINSIZE 2 * 64 * 1024
#define NWORDS 64 * 1024

#define code_sequence_for(X) dr_data[X] = (uint16_t)current_position;

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
	
	code_sequence_for(0x00);		// chill
	append_mc1(do_nothing);
	append_mc1(done);

	code_sequence_for(0x01);		// goto
	append_mc1(pkptrout);
	append_mc1(datatopk);
	append_mc1(done);
	
	code_sequence_for(0x02);		// goto_if_zero
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, regtodata(1));
	append_mc1(tmpatopkifz);
	append_mc1(done);
	
	code_sequence_for(0x03);		// goto_if_nonzero
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, regtodata(1));
	append_mc1(tmpatopkifnz);
	append_mc1(done);
	
	code_sequence_for(0x04);		// goto_if_negative
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, regtodata(1));
	append_mc1(tmpatopkifn);
	append_mc1(done);
	
	code_sequence_for(0x05);		// goto_if_nonnegative
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, regtodata(1));
	append_mc1(tmpatopkifnn);
	append_mc1(done);
	
	code_sequence_for(0x06);		// goto_if_positive
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, regtodata(1));
	append_mc1(tmpatopkifp);
	append_mc1(done);
	
	code_sequence_for(0x07);		// goto_if_nonpositive
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, regtodata(1));
	append_mc1(tmpatopkifnp);
	append_mc1(done);

	code_sequence_for(0x10);		// Set *constant to constant
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, pkptroutinc);
	append_mc2(datatotmpb, tmpatoaddr);
	append_mc2(tmpbtodata, holdaddr);
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(0x11);		// Set *constant to *constant
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, pkptroutinc);
	append_mc2(datatotmpb, holddataaddr);
	append_mc1(readRAM);
	append_mc1(done);

	code_sequence_for(0x12);		// Set *constant to rN
	append_mc1(pkptroutinc);
	append_mc2(holddataaddr, regtodata(1));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(0x13);		// Set *constant to *rN
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, regptodata(1));
	append_mc1(datatotmpb);
	append_mc2(tmpatoaddr, tmpbtodata);
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(0x14);		// Set *constant to (*rN+offs)
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, pkptrout);
	append_mc1(datatooffs);
	append_mc1(regptodata_o(1));
	append_mc2(holddata, tmpatoaddr);
	append_mc1(writeRAM);
	append_mc1(done);

	code_sequence_for(0x15);		// Set *constant to srN
	append_mc1(pkptroutinc);
	append_mc2(holddataaddr, sregtodata(1));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(0x16);		// Set *constant to *srN
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, sregptodata(1));
	append_mc1(datatotmpb);
	append_mc2(tmpatoaddr, tmpbtodata);
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(0x17);		// Set *constant to *(srN+offs)
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(sregptodata_o(1));
	append_mc2(holddata, tmpatoaddr);
	append_mc1(writeRAM);
	append_mc1(done);

	code_sequence_for(0x18);		// Set rN to constant
	append_mc1(pkptroutinc);
	append_mc1(datatoreg(1));
	append_mc1(done);
	
	code_sequence_for(0x19);		// Set rN to *constant
	append_mc1(pkptroutinc);
	append_mc2(holddataaddr, readRAM);
	append_mc1(datatoreg(1));
	append_mc1(done);

	code_sequence_for(0x1a);		// Set rN to rM
	append_mc1(regtodata(2));
	append_mc1(datatoreg(1));
	append_mc1(done);
	
	code_sequence_for(0x1b);		// Set rN to *rM
	append_mc1(regptodata(2));
	append_mc1(datatoreg(1));
	append_mc1(done);
	
	code_sequence_for(0x1c);		// Set rN to *(rM+offs)
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(regptodata_o(2));
	append_mc1(datatoreg(1));
	append_mc1(done);

	code_sequence_for(0x1e);		// Set rN to srM
	append_mc1(sregtodata(2));
	append_mc1(datatoreg(1));
	append_mc1(done);
	
	code_sequence_for(0x1f);		// Set rN to *srM
	append_mc1(sregptodata(2));
	append_mc1(datatoreg(1));
	append_mc1(done);
	
	code_sequence_for(0x20);		// Set rN to *(srM+offs)
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(sregptodata_o(2));
	append_mc1(datatoreg(1));
	append_mc1(done);

	code_sequence_for(0x21);		// Set *rN to constant
	append_mc1(pkptroutinc);
	append_mc2(holddata, regtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(0x22);		// Set *rN to *constant
	append_mc1(pkptroutinc);
	append_mc2(holddataaddr, readRAM);
	append_mc2(holddata, regtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);

	code_sequence_for(0x23);		// Set *rN to rM
	append_mc2(regtoaddr(1), regtodata(2));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(0x24);		// Set *rN to *rM
	append_mc1(regptodata(2));
	append_mc2(holddata, regtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(0x25);		// Set *rN to (*rM+offs)
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(regptodata_o(2));
	append_mc2(holddata, regtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);

	code_sequence_for(0x26);		// Set *rN to srM
	append_mc2(regtoaddr(1), sregtodata(2));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(0x27);		// Set *rN to *srM
	append_mc1(sregptodata(2));
	append_mc2(holddata, regtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(0x28);		// Set *rN to *(srM+offs)
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(sregptodata_o(2));
	append_mc2(holddata, regtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);

	code_sequence_for(0x29);		// Set *(rN+offs) to constant
	append_mc1(pkptroutinc);
	append_mc2(datatooffs, pkptroutinc);
	append_mc1(datatotmpa);
	append_mc2(regtoaddr(1), holdaddr);
	append_mc2(tmpatodata, holdaddr);
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(0x2a);		// Set *(rN+offs) to *constant
	append_mc1(pkptroutinc);
	append_mc2(datatooffs, pkptroutinc);
	append_mc2(holddataaddr, readRAM);
	append_mc2(holddata, regtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);

	code_sequence_for(0x2b);		// Set *(rN+offs) to rM
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc2(regtodata(2), regtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(0x2c);		// Set *(rN+offs) to *rM
	append_mc1(pkptroutinc);
	append_mc2(datatooffs, regptodata(2));
	append_mc2(holddata, regtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(0x2d);		// Set *(rN+offs) to *(rM+offs)
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(regptodata_o(2));
	append_mc2(datatotmpa, pkptroutinc);
	append_mc1(datatooffs);
	append_mc2(tmpatodata, regtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(0x2e);		// Set *(rN+offs) to srM
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc2(sregtodata(2), regtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(0x2f);		// Set *(rN+offs) to *srM
	append_mc1(pkptroutinc);
	append_mc2(datatooffs, sregptodata(2));
	append_mc2(holddata, regtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(0x30);		// Set *(rN+offs) to *(srM+offs)
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(sregptodata_o(2));
	append_mc2(datatotmpb, tmpatodata);
	append_mc1(datatooffs);
	append_mc2(tmpbtodata, regtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);

	code_sequence_for(0x31);		// Set srN to constant
	append_mc1(pkptroutinc);
	append_mc1(datatosreg(1));
	append_mc1(done);
	
	code_sequence_for(0x32);		// Set srN to *constant
	append_mc1(pkptroutinc);
	append_mc2(holddataaddr, readRAM);
	append_mc1(datatosreg(1));
	append_mc1(done);

	code_sequence_for(0x33);		// Set srN to rM
	append_mc1(regtodata(2));
	append_mc1(datatosreg(1));
	append_mc1(done);
	
	code_sequence_for(0x34);		// Set srN to *rM
	append_mc1(regtoaddr(2));
	append_mc1(readRAM);
	append_mc1(datatosreg(1));
	append_mc1(done);
	
	code_sequence_for(0x35);		// Set srN to (*rM+offs)
	append_mc1(pkptroutinc);
	append_mc2(datatooffs, regtoaddr(2));
	append_mc1(readRAM_o);
	append_mc1(datatosreg(1));
	append_mc1(done);
	
	code_sequence_for(0x36);		// Set srN to srM
	append_mc1(sregtodata(2));
	append_mc1(datatosreg(1));
	append_mc1(done);
	
	code_sequence_for(0x37);		// Set srN to *srM
	append_mc1(sregtoaddr(2));
	append_mc1(readRAM);
	append_mc1(datatosreg(1));
	append_mc1(done);
	
	code_sequence_for(0x38);		// Set srN to *(srM+offs)
	append_mc1(pkptroutinc);
	append_mc2(datatooffs, sregtoaddr(2));
	append_mc1(readRAM_o);
	append_mc1(datatosreg(1));
	append_mc1(done);

	code_sequence_for(0x39);		// Set *srN to constant
	append_mc1(pkptroutinc);
	append_mc2(holddata, sregtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(0x3a);		// Set *srN to *constant
	append_mc1(pkptroutinc);
	append_mc2(holddataaddr, readRAM);
	append_mc2(holddata, sregtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);

	code_sequence_for(0x3b);		// Set *srN to rM
	append_mc2(regtodata(2), sregtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(0x3c);		// Set *srN to *rM
	append_mc1(regptodata(2));
	append_mc2(holddata, sregtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(0x3d);		// Set *srN to (*rM+offs)
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(regptodata_o(2));
	append_mc2(holddata, sregtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);

	code_sequence_for(0x3e);		// Set *srN to srM
	append_mc2(sregtodata(2), sregtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(0x3f);		// Set *srN to *srM
	append_mc1(sregptodata(2));
	append_mc2(holddata, sregtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(0x40);		// Set *srN to *(srM+offs)
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(sregptodata_o(2));
	append_mc2(holddata, sregtoaddr(1));
	append_mc1(writeRAM);
	append_mc1(done);

	code_sequence_for(0x41);		// Set *(srN+offs) to constant
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(pkptroutinc);
	append_mc2(holddata, sregtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(0x42);		// Set *(srN+offs) to *constant
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(pkptroutinc);
	append_mc2(holddataaddr, readRAM);
	append_mc2(holddata, sregtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);

	code_sequence_for(0x43);		// Set *(srN+offs) to rM
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc2(regtodata(2), sregtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(0x44);		// Set *(srN+offs) to *rM
	append_mc1(pkptroutinc);
	append_mc2(datatooffs, regptodata(2));
	append_mc2(holddata, sregtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(0x45);		// Set *(srN+offs) to *(rM+offs)
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, pkptroutinc);
	append_mc2(datatooffs, regptodata_o(2));
	append_mc2(datatotmpb, tmpatodata);
	append_mc1(datatooffs);
	append_mc2(tmpbtodata, sregtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(0x46);		// Set *(srN+offs) to srM
	append_mc1(pkptroutinc);
	append_mc1(datatooffs);
	append_mc2(sregtodata(2), sregtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(0x47);		// Set *(srN+offs) to *srM
	append_mc1(pkptroutinc);
	append_mc2(datatooffs, sregptodata(2));
	append_mc2(holddata, sregtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	
	code_sequence_for(0x48);		// Set *(srN+offs) to *(srM+offs)
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, pkptroutinc);
	append_mc1(datatooffs);
	append_mc1(sregptodata_o(2));
	append_mc2(datatotmpb, tmpatodata);
	append_mc1(datatooffs);
	append_mc2(tmpbtodata, sregtoaddr(1));
	append_mc1(writeRAM_o);
	append_mc1(done);
	

	//**ALU**//
	for (int n = 0; n < 16; n++) {
		code_sequence_for(0x50+n);
		append_mc1(ALUop(1, 2, n));
		append_mc1(datatoreg(1));
		append_mc1(done);
	}
	
	//**stack stuff**//

	// pushes
	code_sequence_for(0x60);		// push
	append_mc1(decrementsp);
	append_mc1(done);
	
	code_sequence_for(0x61);		// push constant
	append_mc2(pkptroutinc, decrementsp);
	append_mc2(holddata, sptoaddr);
	append_mc1(writeRAM);
	append_mc1(done);
	
	code_sequence_for(0x62);		// push *constant
	append_mc1(pkptroutinc);
	append_mc2(holddata, sptoaddr);
	append_mc2(writeRAM, decrementsp);
	append_mc1(done);
	
	code_sequence_for(0x63);		// push rN
	append_mc2(sptoaddr, regtodata(1));
	append_mc2(writeRAM, decrementsp);
	append_mc1(done);
	
	code_sequence_for(0x64);		// push *rN
	append_mc1(regptodata(1));
	append_mc2(holddata, sptoaddr);
	append_mc2(writeRAM, decrementsp);
	append_mc1(done);
	
	code_sequence_for(0x65);		// push *(rN+offs)
	append_mc1(pkptroutinc);
	append_mc2(datatooffs, regptodata(1));
	append_mc2(holddata, sptoaddr);
	append_mc2(writeRAM_o, decrementsp);
	append_mc1(done);
	
	code_sequence_for(0x66);		// push srN
	append_mc2(sptoaddr, sregtodata(1));
	append_mc2(writeRAM, decrementsp);
	append_mc1(done);
	
	code_sequence_for(0x67);		// push *srN
	append_mc1(sregptodata(1));
	append_mc2(holddata, sptoaddr);
	append_mc2(writeRAM, decrementsp);
	append_mc1(done);
	
	code_sequence_for(0x68);		// push *(srN+offs)
	append_mc1(pkptroutinc);
	append_mc2(datatooffs, sregtodata(1));
	append_mc2(holddata, sptoaddr);
	append_mc2(writeRAM_o, decrementsp);
	append_mc1(done);

	// pops
	code_sequence_for(0x69);		// pop
	append_mc1(incrementsp);
	append_mc1(done);
	
	code_sequence_for(0x6a);		// pop *constant
	append_mc1(pkptroutinc);
	append_mc2(datatotmpa, spptodata);
	append_mc2(holddata, tmpatoaddr);
	append_mc2(writeRAM, incrementsp);
	append_mc1(done);
	
	code_sequence_for(0x6b);		// pop rN
	append_mc1(spptodata);
	append_mc2(datatoreg(1), incrementsp);
	append_mc1(done);
	
	code_sequence_for(0x6c);		// pop *rN
	append_mc1(spptodata);
	append_mc2(holddata, regtoaddr(1));
	append_mc2(writeRAM, incrementsp);
	append_mc1(done);
	
	code_sequence_for(0x6d);		// pop *(rN+offs)
	append_mc1(pkptroutinc);
	append_mc2(datatooffs, spptodata);
	append_mc2(holddata, regtoaddr(1));
	append_mc2(writeRAM_o, incrementsp);
	append_mc1(done);
	
	code_sequence_for(0x6e);		// pop srN
	append_mc1(spptodata);
	append_mc2(datatosreg(1), incrementsp);
	append_mc1(done);
	
	code_sequence_for(0x6f);		// pop *srN
	append_mc1(spptodata);
	append_mc2(holddata, sregtoaddr(1));
	append_mc2(writeRAM, incrementsp);
	append_mc1(done);
	
	code_sequence_for(0x70);		// pop *(srN+offs)
	append_mc1(pkptroutinc);
	append_mc2(datatooffs, spptodata);
	append_mc2(holddata, sregtoaddr(1));
	append_mc2(writeRAM_o, incrementsp);
	append_mc1(done);

	// function stuff
	code_sequence_for(0x80);		// call
	append_mc2(pkptroutinc, decrementsp);
	append_mc2(datatotmpa, pktodata);
	append_mc2(holddata, sptoaddr);
	append_mc2(writeRAM, tmpatopk);
	append_mc1(done);
	
	code_sequence_for(0x81);		// return
	append_mc1(spptodata);
	append_mc2(datatopk, incrementsp);
	append_mc1(done);

	// I/O pins
	code_sequence_for(0xa0);		// pinmode_input constant
	append_mc1(pinmodein(1));
	append_mc1(done);
	
	code_sequence_for(0xa1);		// pinmode_output constant
	append_mc1(pinmodeout(1));
	append_mc1(done);

	code_sequence_for(0xa2);		// set_pin_low constant
	append_mc1(setpinlow(1));
	append_mc1(done);
	
	code_sequence_for(0xa3);		// set_pin_high constant
	append_mc1(setpinhigh(1));
	append_mc1(done);

	code_sequence_for(0xa4);		// read_pin constant, rN
	append_mc1(pintodata(1));
	append_mc1(datatoreg(2));
	append_mc1(done);
	
	code_sequence_for(0xa5);		// write_pin constant, rN
	append_mc1(regtodata(2));
	append_mc1(datatopin(1));
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
