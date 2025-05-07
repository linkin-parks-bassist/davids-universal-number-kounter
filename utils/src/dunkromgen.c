#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_REGISTERS 16
#define N_SPECIAL_REGS 5

#define N_PINS 16

#define N_ALU_OPS 8

#define begini 0x0001

#define specialtodata(X) 	0x0002 + 0x1000 * X
#define specialtoaddr(X) 	0x0003 + 0x1000 * X
#define specialptodata(X) 	0x0004 + 0x1000 * X
#define specialptodatao(X) 	0x0005 + 0x1000 * X
#define datatospecial(X) 	0x0006 + 0x1000 * X

#define pktodata 		0x0002
#define pktoaddr 		0x0003
#define pkptrout 		0x0004
#define pkoptrout 		0x0005
#define datatopk 		0x0006

#define sptodata 		0x1002
#define sptoaddr 		0x1003
#define spptodata 		0x1004
#define spptodatao 		0x1005
#define datatosp 		0x1006

#define tmpatodata 		0x2002
#define tmpatoaddr 		0x2003
#define tmpaptodata 	0x2004
#define tmpaptodatao 	0x2005
#define datatotmpa 		0x2006

#define tmpbtodata 		0x3002
#define tmpbtoaddr 		0x3003
#define tmpbptodata 	0x3004
#define tmpbptodatao 	0x3005
#define datatotmpb 		0x3006

#define offstodata 		0x4002
#define offstoaddr 		0x4003
#define offsptodata 	0x4004
#define offsptodatao 	0x4005
#define datatooffs 		0x4006

#define incrementpk 	0x0010

#define pkptroutinc		0x0011

#define writetmpatopk 	0x0012
#define writetmpbtopk 	0x0013

#define tmpatopkifz 	0x0014
#define tmpatopkifnz 	0x0015

#define tmpatopkifn 	0x0016
#define tmpatopkifnn 	0x0017

#define tmpatopkifp 	0x0018
#define tmpatopkifnp 	0x0019


#define incrementsp 	0x1010
#define decrementsp 	0x1011


#define regtodata(X) 	0x0020 + 0x1000 * X
#define regtoaddr(X) 	0x0021 + 0x1000 * X
#define regptodata(X) 	0x0022 + 0x1000 * X
#define regptodatao(X) 	0x0023 + 0x1000 * X
#define datatoreg(X) 	0x0024 + 0x1000 * X

#define writeRAM 		0x0025
#define writeRAMo 		0x0026

#define readRAM 		0x0027
#define readRAMo 		0x0028

#define pinmodein(N)  	0x0030 + 0x1000 * N
#define pinmodeout(N) 	0x0031 + 0x1000 * N
#define setpinlow(N)  	0x0032 + 0x1000 * N
#define setpinhigh(N) 	0x0033 + 0x1000 * N
#define pintodata(N) 	0x0034 + 0x1000 * N
#define datatopin(N)  	0x0035 + 0x1000 * N

#define ALUop(X, Y, Z) 	0x0040 + 0x1000 * X + 0x0100 * Y + 0x0001 * Z

#define holddata 		0x0050
#define holdaddr	 	0x0051

#define holddatalong 	0x0250
#define holdaddrlong 	0x0251

#define holdaddrdata 	0x0052
#define holddataaddr 	0x0053

#define done 0xfffe
#define reset 0xffff

#define BINSIZE 2 * 64 * 1024
#define NWORDS 64 * 1024

#define CODE_SEQUENCE_FOR(X) dr_data[X] = (uint16_t)current_position;

uint16_t mr1_data[BINSIZE];
uint16_t mr2_data[BINSIZE];
uint16_t  dr_data[BINSIZE];
	
int current_position = 0;

void WRITEOUT1(uint16_t v1)
{
	mr1_data[current_position] = v1;
	mr2_data[current_position] = 0x0000;
	current_position += 1;
}

void WRITEOUT2(uint16_t v1, uint16_t v2)
{
	mr1_data[current_position] = v1;
	mr2_data[current_position] = v2;
	current_position += 1;
}

int main(int argc, char* argv[])
{
	// Write processed data to output file
	FILE* mainROM1 = fopen("roms/main_rom_1", "wb");
	if (!mainROM1) {
		perror("Error opening output file");
		exit(EXIT_FAILURE);
	}
	
	FILE* mainROM2 = fopen("roms/main_rom_2", "wb");
	if (!mainROM2) {
		perror("Error opening output file");
		exit(EXIT_FAILURE);
	}

	FILE* decoderROM = fopen("roms/decoder_rom", "wb");
	if (!decoderROM) {
		perror("Error opening output file");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < NWORDS; i++) {
		mr1_data[i] = 0;
	}
	for (int i = 0; i < NWORDS; i++) {
		mr2_data[i] = 0;
	}
	for (int i = 0; i < NWORDS; i++) {
		dr_data[i] = 0;
	}
	

	int N;
	int M;
	int k;

	WRITEOUT1(pkptroutinc);
	WRITEOUT1(begini);

	CODE_SEQUENCE_FOR(0x0000); // chill

	WRITEOUT1(0x0000);
	WRITEOUT1(done);

	CODE_SEQUENCE_FOR(0x0001); // goto

	WRITEOUT1(pkptrout);
	WRITEOUT1(datatopk);
	WRITEOUT1(done);

	// the various conditional gotos
	for (N = 0; N < N_REGISTERS; N++) {
		for (int j = 0; j < 3; j++) {
			for (int i = 0; i < 2; i++) {
				CODE_SEQUENCE_FOR(0x1000 * N + 0x0100 * j + 0x0002 + 0x0001 * i);

				WRITEOUT1(pkptrout);
				WRITEOUT1(datatotmpa);
				WRITEOUT1(regtodata(N));
				WRITEOUT1(0x0014 + (i + 2 * j));
				WRITEOUT1(done);
			}
		}
	}

	// ALU stuff
	for (N = 0; N < N_REGISTERS; N++) {
		for (M = 0; M < N_REGISTERS; M++) {
			for (k = 0; k < N_ALU_OPS; k++) {
				CODE_SEQUENCE_FOR(0x0010 + 0x1000 * N + 0x0100 * M + 0x0001 * k);

				WRITEOUT1(ALUop(N, M, k));
				WRITEOUT1(datatoreg(N));
				WRITEOUT1(done);
			}
		}
	}

	//** FUNCTION STUFF **//

	// call
	CODE_SEQUENCE_FOR(0x0004);

	WRITEOUT1(pkptroutinc);
	WRITEOUT2(datatotmpa, decrementsp);
	WRITEOUT2(pktodata, sptoaddr);
	WRITEOUT2(writeRAM, tmpatodata);
	WRITEOUT1(datatopk);
	WRITEOUT1(done);

	// return
	CODE_SEQUENCE_FOR(0x0005);

	WRITEOUT1(spptodata);
	WRITEOUT2(datatopk, incrementsp);
	WRITEOUT1(done);

	////////////////
	//************//
	//***PUSHES***//
	//************//
	////////////////

	// blank push
	CODE_SEQUENCE_FOR(0x0006);

	WRITEOUT1(decrementsp);
	WRITEOUT1(done);

	// push constant onto the stack
	CODE_SEQUENCE_FOR(0x0106);

	WRITEOUT1(decrementsp);
	WRITEOUT1(pkptrout);
	WRITEOUT2(holddatalong, sptoaddr);
	WRITEOUT1(writeRAM);
	WRITEOUT1(incrementpk);
	WRITEOUT1(done);

	// push *constant onto the stack
	CODE_SEQUENCE_FOR(0x0206);

	WRITEOUT1(decrementsp);
	WRITEOUT1(pkptrout);
	WRITEOUT1(holddataaddr);
	WRITEOUT1(readRAM);
	WRITEOUT1(holddatalong);
	WRITEOUT1(sptoaddr);
	WRITEOUT1(writeRAM);
	WRITEOUT1(incrementpk);
	WRITEOUT1(done);

	for (N = 0; N < N_REGISTERS; N++) {
		// push register onto the stack
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0306);

		WRITEOUT1(decrementsp);
		WRITEOUT1(sptoaddr);
		WRITEOUT1(regtodata(N));
		WRITEOUT1(writeRAM);
		WRITEOUT1(done);

		// push *register onto the stack
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0406);

		WRITEOUT1(decrementsp);
		WRITEOUT1(regptodata(N));
		WRITEOUT1(holddatalong);
		WRITEOUT1(sptoaddr);
		WRITEOUT1(writeRAM);
		WRITEOUT1(done);

		// push *(rN+offs)
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0506);

		WRITEOUT1(pkptrout);
		WRITEOUT1(datatooffs);
		WRITEOUT1(incrementpk);
		WRITEOUT1(decrementsp);
		WRITEOUT1(regptodatao(N));
		WRITEOUT1(holddatalong);
		WRITEOUT1(sptoaddr);
		WRITEOUT1(writeRAM);
		WRITEOUT1(done);
	}

	for (N = 0; N < N_SPECIAL_REGS; N++) {
		// push srN onto the stack
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0306);

		WRITEOUT1(decrementsp);
		WRITEOUT1(sptoaddr);
		WRITEOUT1(regtodata(N));
		WRITEOUT1(writeRAM);
		WRITEOUT1(done);

		// push *srN onto the stack
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0406);

		WRITEOUT1(decrementsp);
		WRITEOUT1(specialptodata(N));
		WRITEOUT1(holddatalong);
		WRITEOUT1(sptoaddr);
		WRITEOUT1(writeRAM);
		WRITEOUT1(done);

		// push *(srN+offs)
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0506);

		WRITEOUT1(pkptrout);
		WRITEOUT1(datatooffs);
		WRITEOUT1(incrementpk);
		WRITEOUT1(decrementsp);
		WRITEOUT1(specialptodatao(N));
		WRITEOUT1(holddatalong);
		WRITEOUT1(sptoaddr);
		WRITEOUT1(writeRAM);
		WRITEOUT1(done);
	}

	//////////////
	//**********//
	//***POPS***//
	//**********//
	//////////////

	// pop to nowhere
	CODE_SEQUENCE_FOR(0x0007);

	WRITEOUT1(incrementsp);
	WRITEOUT1(done);

	// pop to *constant
	CODE_SEQUENCE_FOR(0x0207);

	WRITEOUT1(pkptrout);
	WRITEOUT1(datatotmpa);
	WRITEOUT1(spptodata);
	WRITEOUT1(holddatalong);
	WRITEOUT1(tmpatoaddr);
	WRITEOUT1(writeRAM);
	WRITEOUT1(incrementpk);
	WRITEOUT1(incrementsp);
	WRITEOUT1(done);

	for (N = 0; N < N_REGISTERS; N++) {
		// pop top of stack into rN
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0307);

		WRITEOUT1(spptodata);
		WRITEOUT1(datatoreg(N));
		WRITEOUT1(incrementsp);
		WRITEOUT1(done);

		// pop top of stack into *rN
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0407);

		WRITEOUT1(spptodata);
		WRITEOUT1(holddatalong);
		WRITEOUT1(regtoaddr(N));
		WRITEOUT1(writeRAM);
		WRITEOUT1(incrementsp);
		WRITEOUT1(done);

		// pop top of stack into *(rN+offs)
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0507);

		WRITEOUT1(pkptrout);
		WRITEOUT1(datatooffs);
		WRITEOUT1(incrementpk);
		WRITEOUT1(spptodata);
		WRITEOUT1(holddatalong);
		WRITEOUT1(regtoaddr(N));
		WRITEOUT1(writeRAMo);
		WRITEOUT1(incrementsp);
		WRITEOUT1(done);
	}

	for (N = 0; N < N_REGISTERS; N++) {
		// pop top of stack into srN
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0607);

		WRITEOUT1(spptodata);
		WRITEOUT1(datatospecial(N));
		WRITEOUT1(incrementsp);
		WRITEOUT1(done);

		// pop top of stack into *srN
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0707);

		WRITEOUT1(spptodata);
		WRITEOUT1(holddatalong);
		WRITEOUT1(specialtoaddr(N));
		WRITEOUT1(writeRAM);
		WRITEOUT1(incrementsp);
		WRITEOUT1(done);

		// pop top of stack into *(srN+offs)
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0807);

		WRITEOUT1(pkptrout);
		WRITEOUT1(datatooffs);
		WRITEOUT1(incrementpk);
		WRITEOUT1(spptodata);
		WRITEOUT1(holddatalong);
		WRITEOUT1(specialtoaddr(N));
		WRITEOUT1(writeRAMo);
		WRITEOUT1(incrementsp);
		WRITEOUT1(done);
	}

	/////////////////
	//*************//
	//***SETTING***//
	//*************//
	/////////////////

	//~ 0x0030, CONSTANT | POINTER, FOLLOWING(1),   CONSTANT, FOLLOWING(2)), set
	// *constant to constant
	CODE_SEQUENCE_FOR(0x0030);

	WRITEOUT1(pkptrout);
	WRITEOUT1(datatotmpa);
	WRITEOUT1(incrementpk);
	WRITEOUT1(pkptrout);
	WRITEOUT1(holddatalong);
	WRITEOUT1(tmpatoaddr);
	WRITEOUT1(writeRAM);
	WRITEOUT1(incrementpk);
	WRITEOUT1(done);

	//~ 0x0130, CONSTANT | POINTER, FOLLOWING(1),   CONSTANT | POINTER,
	//FOLLOWING(2)), set *constant to *constant
	CODE_SEQUENCE_FOR(0x0130);

	WRITEOUT1(pkptrout);
	WRITEOUT1(datatotmpa);
	WRITEOUT1(incrementpk);
	WRITEOUT1(pkptrout);
	WRITEOUT1(holddataaddr);
	WRITEOUT1(readRAM);
	WRITEOUT1(holddatalong);
	WRITEOUT1(tmpatoaddr);
	WRITEOUT1(writeRAM);
	WRITEOUT1(done);

	for (N = 0; N < N_REGISTERS; N++) {
		// set *constant to rN
		//~ 0x0031, CONSTANT | POINTER, FOLLOWING(1),   REGISTER, FIRST_NIBBLE)
		CODE_SEQUENCE_FOR(0x0031 + 0x1000 * N);

		WRITEOUT1(pkptrout);
		WRITEOUT1(holddataaddr);
		WRITEOUT1(regtodata(N));
		WRITEOUT1(writeRAM);
		WRITEOUT1(incrementpk);
		WRITEOUT1(done);

		// set *constant to *rN
		//~ 0x0131, CONSTANT | POINTER, FOLLOWING(1),   REGISTER | POINTER,
		//FIRST_NIBBLE)
		CODE_SEQUENCE_FOR(0x0131 + 0x1000 * N);

		WRITEOUT1(pkptrout);
		WRITEOUT1(datatotmpa);
		WRITEOUT1(regptodata(N));
		WRITEOUT1(holddatalong);
		WRITEOUT1(tmpatoaddr);
		WRITEOUT1(writeRAM);
		WRITEOUT1(done);

		// set *constant to *(rN+offs)
		//~ 0x0231, CONSTANT | POINTER, FOLLOWING(1),   REGISTER | POINTER |
		//W_OFFSET, FIRST_NIBBLE | FOLLOWING(2))
		CODE_SEQUENCE_FOR(0x0231 + 0x1000 * N);
		// implement
	}

	for (N = 0; N < N_SPECIAL_REGS; N++) {
		// set *constant to srN
		//~ 0x0331, CONSTANT | POINTER, FOLLOWING(1), S_REGISTER, FIRST_NIBBLE)
		CODE_SEQUENCE_FOR(0x0331 + 0x1000 * N);

		WRITEOUT1(pkptrout);
		WRITEOUT1(holddataaddr);
		WRITEOUT1(specialtodata(N));
		WRITEOUT1(writeRAM);
		WRITEOUT1(incrementpk);
		WRITEOUT1(done);

		// set *constant to srN
		//~ 0x0431, CONSTANT | POINTER, FOLLOWING(1), S_REGISTER | POINTER,
		//FIRST_NIBBLE)
		CODE_SEQUENCE_FOR(0x0431 + 0x1000 * N);

		WRITEOUT1(pkptrout);
		WRITEOUT1(datatotmpa);
		WRITEOUT1(specialptodata(N));
		WRITEOUT1(holddatalong);
		WRITEOUT1(tmpatoaddr);
		WRITEOUT1(writeRAM);
		WRITEOUT1(done);

		// set *constant to *(srN+offs)
		//~ 0x0531, CONSTANT | POINTER, FOLLOWING(1), S_REGISTER | POINTER |
		//W_OFFSET, FIRST_NIBBLE | FOLLOWING(2))
		CODE_SEQUENCE_FOR(0x0531 + 0x1000 * N);
		WRITEOUT1(pkptroutinc);
		WRITEOUT2(datatotmpa, pkptroutinc);
		WRITEOUT1(datatooffs);
		WRITEOUT1(specialptodatao(N));
		WRITEOUT1(datatotmpb);
		WRITEOUT2(tmpbtodata, tmpatoaddr);
		WRITEOUT1(writeRAM);
		WRITEOUT1(done);

	}

	for (N = 0; N < N_REGISTERS; N++) {
		// set rN to constant
		//~ 0x0033, REGISTER, FIRST_NIBBLE, CONSTANT,           FOLLOWING(1)),
		CODE_SEQUENCE_FOR(0x0033 + 0x1000 * N);

		WRITEOUT1(pkptrout);
		WRITEOUT1(datatoreg(N));
		WRITEOUT1(incrementpk);
		WRITEOUT1(done);

		// set rN to *constant
		//~ 0x0034, REGISTER, FIRST_NIBBLE, CONSTANT | POINTER, FOLLOWING(1)),
		CODE_SEQUENCE_FOR(0x0034 + 0x1000 * N);

		WRITEOUT1(pkptrout);
		WRITEOUT1(holddataaddr);
		WRITEOUT1(readRAM);
		WRITEOUT1(datatoreg(N));
		WRITEOUT1(incrementpk);
		WRITEOUT1(done);

		// set *rN to constant
		//~ 0x003b, REGISTER | POINTER, FIRST_NIBBLE, CONSTANT, FOLLOWING(1)),
		CODE_SEQUENCE_FOR(0x003b + 0x1000 * N);

		WRITEOUT1(pkptrout);
		WRITEOUT1(datatotmpa);
		WRITEOUT1(regtoaddr(N));
		WRITEOUT1(tmpatodata);
		WRITEOUT1(writeRAM);
		WRITEOUT1(incrementpk);
		WRITEOUT1(done);

		// set *rN to *constant
		//~ 0x003c, REGISTER | POINTER, FIRST_NIBBLE, CONSTANT | POINTER,
		//FOLLOWING(1)),
		CODE_SEQUENCE_FOR(0x003c + 0x1000 * N);

		WRITEOUT1(pkptrout);
		WRITEOUT1(datatotmpa);
		WRITEOUT1(regtoaddr(N));
		WRITEOUT1(tmpaptodata);
		WRITEOUT1(writeRAM);
		WRITEOUT1(incrementpk);
		WRITEOUT1(done);

		// set *(rN+offs) to constant
		//~ 0x0043, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE |
		//OFFSET_FOLLOWING(1), CONSTANT,           FOLLOWING(2)),
		CODE_SEQUENCE_FOR(0x0043 + 0x1000 * N);
		// implement

		// set *(rN+offs) to *constant
		//~ 0x0143, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE |
		//OFFSET_FOLLOWING(1), CONSTANT | POINTER, FOLLOWING(2)),
		CODE_SEQUENCE_FOR(0x0143);
		// implement

		for (M = 0; M < N_REGISTERS; M++) {
			// set rN to rM
			//~ 0x0035, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x0035 + 0x1000 * N + 0x0100 * M);

			WRITEOUT1(0x1000 * M + 0x0003);
			WRITEOUT1(datatoreg(N));
			WRITEOUT1(done);

			// set rN to *rM
			//~ 0x0036, REGISTER, FIRST_NIBBLE, REGISTER | POINTER, SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x0036 + 0x1000 * N + 0x0100 * M);

			WRITEOUT1(regptodata(M));
			WRITEOUT1(datatoreg(N));
			WRITEOUT1(done);

			// set rN to *(rM+offs)
			//~ 0x0037, REGISTER, FIRST_NIBBLE, REGISTER | POINTER | W_OFFSET,
			//SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
			CODE_SEQUENCE_FOR(0x0037 + 0x1000 * N + 0x0100 * M);

			WRITEOUT1(pkptrout);
			WRITEOUT1(datatooffs);
			WRITEOUT1(incrementpk);
			WRITEOUT1(regptodatao(M));
			WRITEOUT1(datatoreg(N));
			WRITEOUT1(done);

			// set *rN to rM
			//~ 0x003d, REGISTER | POINTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x003d + 0x1000 * N + 0x0100 * M);

			WRITEOUT1(regtodata(M));
			WRITEOUT1(holddatalong);
			WRITEOUT1(regtoaddr(N));
			WRITEOUT1(writeRAM);
			WRITEOUT1(done);

			// set *rN to *rM
			//~ 0x003e, REGISTER | POINTER, FIRST_NIBBLE, REGISTER | POINTER,
			//SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x003e + 0x1000 * N + 0x0100 * M);

			WRITEOUT1(regptodata(M));
			WRITEOUT1(holddatalong);
			WRITEOUT1(regtoaddr(N));
			WRITEOUT1(writeRAM);
			WRITEOUT1(done);

			// set *rN to *(rM+offs)
			//~ 0x003f, REGISTER | POINTER, FIRST_NIBBLE, REGISTER | POINTER |
			//W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
			CODE_SEQUENCE_FOR(0x003f + 0x1000 * N + 0x0100 * M);

			WRITEOUT1(pkptrout);
			WRITEOUT1(datatooffs);
			WRITEOUT1(regptodatao(M));
			WRITEOUT1(holddatalong);
			WRITEOUT1(regtoaddr(N));
			WRITEOUT1(writeRAM);
			WRITEOUT1(incrementpk);
			WRITEOUT1(done);

			// set *(rN+offs) to rM
			//~ 0x0044, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE |
			//OFFSET_FOLLOWING(1), REGISTER,                      SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x0044 + 0x1000 * N + 0x0100 * M);

			// set *(rN+offs) to *rM
			//~ 0x0045, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE |
			//OFFSET_FOLLOWING(1), REGISTER | POINTER,            SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x0045 + 0x1000 * N + 0x0100 * M);

			// set *(rN+offs) to *(rM+offs)
			//~ 0x0046, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE |
			//OFFSET_FOLLOWING(1), REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE |
			//OFFSET_FOLLOWING(2)),
			CODE_SEQUENCE_FOR(0x0046 + 0x1000 * N + 0x0100 * M);
		}

		for (M = 0; M < N_SPECIAL_REGS; M++) {
			// set rN to srM
			//~ 0x0038, REGISTER, FIRST_NIBBLE, S_REGISTER, SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x0038 + 0x1000 * N + 0x0100 * M);

			WRITEOUT1(specialtodata(M));
			WRITEOUT1(datatoreg(N));
			WRITEOUT1(done);

			// set rN to *srM
			//~ 0x0039, REGISTER, FIRST_NIBBLE, S_REGISTER | POINTER, SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x0039 + 0x1000 * N + 0x0100 * M);

			WRITEOUT1(specialptodata(M));
			WRITEOUT1(datatoreg(N));
			WRITEOUT1(done);

			// set rN to *(srM+offs)
			//~ 0x003a, REGISTER, FIRST_NIBBLE, S_REGISTER | POINTER | W_OFFSET,
			//SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
			CODE_SEQUENCE_FOR(0x003a + 0x1000 * N + 0x0100 * M);

			WRITEOUT1(pkptrout);
			WRITEOUT1(datatooffs);
			WRITEOUT1(specialptodatao(M));
			WRITEOUT1(datatoreg(N));
			WRITEOUT1(incrementpk);
			WRITEOUT1(done);

			// set *rN to srM
			//~ 0x0040, REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER, SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x0040 + 0x1000 * N + 0x0100 * M);

			WRITEOUT1(specialtodata(M));
			WRITEOUT1(holddatalong);
			WRITEOUT1(regtoaddr(N));
			WRITEOUT1(writeRAM);
			WRITEOUT1(done);

			// set *rN to *srM
			//~ 0x0041, REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER | POINTER,
			//SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x0041 + 0x1000 * N + 0x0100 * M);

			WRITEOUT1(specialptodata(M));
			WRITEOUT1(holddatalong);
			WRITEOUT1(regtoaddr(N));
			WRITEOUT1(writeRAM);
			WRITEOUT1(done);

			// set *rN to *(srM+offs)
			//~ 0x0042, REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER | POINTER |
			//W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
			CODE_SEQUENCE_FOR(0x0042 + 0x1000 * N + 0x0100 * M);

			WRITEOUT1(pkptrout);
			WRITEOUT1(datatooffs);
			WRITEOUT1(specialptodatao(M));
			WRITEOUT1(holddatalong);
			WRITEOUT1(regtoaddr(N));
			WRITEOUT1(writeRAM);
			WRITEOUT1(incrementpk);
			WRITEOUT1(done);

			// set *(rN+offs) to srM
			//~ 0x0047, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE |
			//OFFSET_FOLLOWING(1), S_REGISTER,                      SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x0047 + 0x1000 * N + 0x0100 * M);

			// set *(rN+offs) to *srM
			//~ 0x0048, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE |
			//OFFSET_FOLLOWING(1), S_REGISTER | POINTER,            SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x0048 + 0x1000 * N + 0x0100 * M);

			// set *(rN+offs) to *(srM+offs)
			//~ 0x0049, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE |
			//OFFSET_FOLLOWING(1), S_REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE |
			//OFFSET_FOLLOWING(2)),
			CODE_SEQUENCE_FOR(0x0049 + 0x1000 * N + 0x0100 * M);
		}
	}

	for (N = 0; N < N_SPECIAL_REGS; N++) {
		// set srN to constant
		//~ 0x004a, S_REGISTER, FIRST_NIBBLE, CONSTANT,           FOLLOWING(1)),
		CODE_SEQUENCE_FOR(0x004a + 0x1000 * N);

		// set srN to *constant
		//~ 0x014a, S_REGISTER, FIRST_NIBBLE, CONSTANT | POINTER, FOLLOWING(1)),
		CODE_SEQUENCE_FOR(0x014a + 0x1000 * N);

		// set *srN to constant
		//~ 0x0051, S_REGISTER | POINTER, FIRST_NIBBLE, CONSTANT, FOLLOWING(1)),
		CODE_SEQUENCE_FOR(0x0051 + 0x1000 * N);

		// set *srN to *constant
		//~ 0x0151, S_REGISTER | POINTER, FIRST_NIBBLE, CONSTANT | POINTER,
		//FOLLOWING(1)),
		CODE_SEQUENCE_FOR(0x0151 + 0x1000 * N);

		// set *(srN+offs) to constant
		//~ 0x0058, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE |
		//OFFSET_FOLLOWING(1), CONSTANT,           FOLLOWING(2)),
		CODE_SEQUENCE_FOR(0x0058 + 0x1000 * N);

		WRITEOUT1(pkptrout);
		WRITEOUT1(datatooffs);
		WRITEOUT1(incrementpk);
		WRITEOUT1(pkptrout);
		WRITEOUT1(holddatalong);
		WRITEOUT1(specialtoaddr(N));
		WRITEOUT1(writeRAMo);
		WRITEOUT1(incrementpk);
		WRITEOUT1(done);

		// set *(srN+offs) to *constant
		//~ 0x0059, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE |
		//OFFSET_FOLLOWING(1), CONSTANT | POINTER, FOLLOWING(2)),
		CODE_SEQUENCE_FOR(0x0059 + 0x1000 * N);

		WRITEOUT1(pkptrout);
		WRITEOUT1(datatooffs);
		WRITEOUT1(incrementpk);
		WRITEOUT1(pkptrout);
		WRITEOUT1(datatotmpa);
		WRITEOUT1(tmpaptodata);
		WRITEOUT1(holddatalong);
		WRITEOUT1(specialtoaddr(N));
		WRITEOUT1(writeRAMo);
		WRITEOUT1(incrementpk);
		WRITEOUT1(done);

		for (M = 0; M < N_REGISTERS; M++) {
			// set srN to rM
			//~ 0x004b, S_REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x004b + 0x1000 * N + 0x0100 * M);

			// set srN to *rM
			//~ 0x004c, S_REGISTER, FIRST_NIBBLE, REGISTER | POINTER, SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x004c + 0x1000 * N + 0x0100 * M);

			// set srN to *(rM+offs)
			//~ 0x004d, S_REGISTER, FIRST_NIBBLE, REGISTER | POINTER | W_OFFSET,
			//SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
			CODE_SEQUENCE_FOR(0x004d + 0x1000 * N + 0x0100 * M);

			// set *srN to rM
			//~ 0x0052, S_REGISTER | POINTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x0052 + 0x1000 * N + 0x0100 * M);

			// set *srN to *rM
			//~ 0x0053, S_REGISTER | POINTER, FIRST_NIBBLE, REGISTER | POINTER,
			//SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x0053 + 0x1000 * N + 0x0100 * M);

			// set *srN to *(rM+offs)
			//~ 0x0054, S_REGISTER | POINTER, FIRST_NIBBLE, REGISTER | POINTER |
			//W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
			CODE_SEQUENCE_FOR(0x0054 + 0x1000 * N + 0x0100 * M);

			// set *(srN+offs) to rM
			//~ 0x005a, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE |
			//OFFSET_FOLLOWING(1), REGISTER,                      SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x005a + 0x1000 * N + 0x0100 * M);

			WRITEOUT1(pkptrout);
			WRITEOUT1(datatooffs);
			WRITEOUT1(specialtoaddr(N));
			WRITEOUT1(regtodata(M));
			WRITEOUT1(writeRAMo);
			WRITEOUT1(incrementpk);
			WRITEOUT1(done);

			// set *(srN+offs) to *rM
			//~ 0x005b, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE |
			//OFFSET_FOLLOWING(1), REGISTER | POINTER,            SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x005b + 0x1000 * N + 0x0100 * M);

			// set *(srN+offs) to *(rM+offs)
			//~ 0x005c, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE |
			//OFFSET_FOLLOWING(1), REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE |
			//OFFSET_FOLLOWING(2)),
			CODE_SEQUENCE_FOR(0x005c + 0x1000 * N + 0x0100 * M);
		}

		for (M = 0; M < N_SPECIAL_REGS; M++) {
			// set srN to srM
			//~ 0x004e, S_REGISTER, FIRST_NIBBLE, S_REGISTER, SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x004e + 0x1000 * N + 0x0100 * M);

			// set *srN to *srM
			//~ 0x004f, S_REGISTER, FIRST_NIBBLE, S_REGISTER | POINTER,
			//SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x004f + 0x1000 * N + 0x0100 * M);

			// set *srN to *(srM+offs)
			//~ 0x0050, S_REGISTER, FIRST_NIBBLE, S_REGISTER | POINTER | W_OFFSET,
			//SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
			CODE_SEQUENCE_FOR(0x0050 + 0x1000 * N + 0x0100 * M);

			// set *srN to srM
			//~ 0x0055, S_REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER,
			//SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x0055 + 0x1000 * N + 0x0100 * M);

			// set *srN to *srM
			//~ 0x0056, S_REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER | POINTER,
			//SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x0056 + 0x1000 * N + 0x0100 * M);

			// set *srN to *(srM+offs)
			//~ 0x0057, S_REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER | POINTER |
			//W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
			CODE_SEQUENCE_FOR(0x0057 + 0x1000 * N + 0x0100 * M);

			// set *(srN+offs) to srM
			//~ 0x005d, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE |
			//OFFSET_FOLLOWING(1), S_REGISTER,                      SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x005d + 0x1000 * N + 0x0100 * M);

			// set *(srN+offs) to *srM
			//~ 0x005e, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE |
			//OFFSET_FOLLOWING(1), S_REGISTER | POINTER,            SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x005e + 0x1000 * N + 0x0100 * M);

			// set *(srN+offs) to *(srM+offs)
			//~ 0x005f, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE |
			//OFFSET_FOLLOWING(1), S_REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE |
			//OFFSET_FOLLOWING(2)),
			CODE_SEQUENCE_FOR(0x005f + 0x1000 * N + 0x0100 * M);
		}
	}
	
	for (int N = 0; N < N_PINS; N++) {
		CODE_SEQUENCE_FOR(0x00a0 + 0x1000 * N);
		WRITEOUT1(pinmodein(N));
		WRITEOUT1(done);
		
		CODE_SEQUENCE_FOR(0x00a1 + 0x1000 * N);
		WRITEOUT1(pinmodeout(N));
		WRITEOUT1(done);
		
		CODE_SEQUENCE_FOR(0x00a2 + 0x1000 * N);
		WRITEOUT1(setpinlow(N));
		WRITEOUT1(done);
		
		CODE_SEQUENCE_FOR(0x00a3 + 0x1000 * N);
		WRITEOUT1(setpinhigh(N));
		WRITEOUT1(done);
		
		for (int M = 0; M < N_REGISTERS; M++) {
			CODE_SEQUENCE_FOR(0x00a4 + 0x1000 * N + 0x0100 * M);
			WRITEOUT1(pintodata(N));
			WRITEOUT1(datatoreg(M));
			WRITEOUT1(done);
			
			CODE_SEQUENCE_FOR(0x00a5 + 0x1000 * N + 0x0100 * M);
			WRITEOUT1(regtodata(M));
			WRITEOUT1(datatopin(N));
			WRITEOUT1(done);
		}
	}

	fwrite(mr1_data, 2, NWORDS, &mainROM1[0]);
	fclose(mainROM1);
	fwrite(mr2_data, 2, NWORDS, &mainROM2[0]);
	fclose(mainROM2);
	fwrite(dr_data, 2, NWORDS, &decoderROM[0]);
	fclose(decoderROM);

	return EXIT_SUCCESS;
}
