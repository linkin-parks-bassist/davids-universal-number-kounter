#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_REGISTERS 16
#define N_SPECIAL_REGS 5

#define N_ALU_OPS 8

#define begini 0x0001

#define specialtodata(X) 0x0002 + 0x1000 * X
#define specialtoaddr(X) 0x0102 + 0x1000 * X
#define specialptodata(X) 0x0202 + 0x1000 * X
#define specialptodatao(X) 0x0302 + 0x1000 * X
#define datatospecial(X) 0x0402 + 0x1000 * X

#define pktodata 0x0002
#define pktoaddr 0x0102
#define pkptrout 0x0202
#define pkoptrout 0x0302
#define datatopk 0x0402

#define sptodata 0x1002
#define sptoaddr 0x1102
#define spptodata 0x1202
#define spptodatao 0x1302
#define datatosp 0x1402

#define tmpatodata 0x2002
#define tmpatoaddr 0x2102
#define tmpaptodata 0x2202
#define tmpaptodatao 0x2302
#define datatotmpa 0x2402

#define tmpbtodata 0x3002
#define tmpbtoaddr 0x3102
#define tmpbptodata 0x3202
#define tmpbptodatao 0x3302
#define datatotmpb 0x3402

#define offstodata 0x4002
#define offstoaddr 0x4102
#define offsptodata 0x4202
#define offsptodatao 0x4302
#define datatooffs 0x4402

#define incrementpk 0x0502
#define incrementsp 0x1502
#define tmpatopk 0x2502
#define tmpbtopk 0x3502

#define decrementsp 0x1602

#define tmpatopkifz 0x0602
#define tmpatopkifnz 0x0702

#define tmpatopkifn 0x0802
#define tmpatopkifnn 0x0902

#define tmpatopkifp 0x0a02
#define tmpatopkifnp 0x0b02

#define regtodata(X) 0x0003 + 0x1000 * X
#define regtoaddr(X) 0x0103 + 0x1000 * X
#define regptodata(X) 0x0203 + 0x1000 * X
#define regptodatao(X) 0x0303 + 0x1000 * X
#define datatoreg(X) 0x0403 + 0x1000 * X

#define writeRAM 0x0004
#define writeRAMo 0x0104
#define readRAM 0x0005
#define readRAMo 0x0105

#define ALUop(X, Y, Z) 0x0010 + 0x1000 * X + 0x0100 * Y + 0x0001 * Z

#define echodata 0x0020
#define echodataaddr 0x0021
#define echoaddrdata 0x0022
#define echoaddraddr 0x0023
#define echoaddraddrlong 0x0024
#define echodatalong 0x0025
#define echoaddrdatalong 0x0026

#define readfrom_onebitport 0x00a0
#define writeto_onebitport 0x00a1
#define readfrom_sixtbitport 0x00a2
#define writeto_sixtbitport 0x00a3

#define done 0xfffe
#define reset 0xffff

#define BINSIZE 2 * 64 * 1024
#define NWORDS 64 * 1024

#define WRITEOUT(X)                                                            \
	mr_data[current_position] = X;                                               \
	current_position += 1;
#define CODE_SEQUENCE_FOR(X) dr_data[X] = (uint16_t)current_position;

// Main function to handle input arguments
int
main(int argc, char* argv[])
{
	// Write processed data to output file
	FILE* mainROM = fopen("../roms/main_rom", "wb");
	if (!mainROM) {
		perror("Error opening output file");
		exit(EXIT_FAILURE);
	}

	FILE* decoderROM = fopen("../roms/decoder_rom", "wb");
	if (!decoderROM) {
		perror("Error opening output file");
		exit(EXIT_FAILURE);
	}

	uint16_t* mr_data = malloc(BINSIZE);
	for (int i = 0; i < NWORDS; i++) {
		mr_data[i] = 0;
	}
	uint16_t* dr_data = malloc(BINSIZE);
	for (int i = 0; i < NWORDS; i++) {
		dr_data[i] = 0;
	}

	int current_position = 0;
	int N;
	int M;
	int k;
	uint16_t* word_ptr = (uint16_t*)(&mr_data[current_position]);

	WRITEOUT(pkptrout);
	WRITEOUT(begini);

	CODE_SEQUENCE_FOR(0x0000); // chill

	WRITEOUT(0x0000);
	WRITEOUT(done);

	CODE_SEQUENCE_FOR(0x0001); // goto

	WRITEOUT(pkptrout);
	WRITEOUT(0x0402);
	WRITEOUT(0xfffe);

	// the various conditional gotos
	for (N = 0; N < N_REGISTERS; N++) {
		for (int j = 0; j < 3; j++) {
			for (int i = 0; i < 2; i++) {
				CODE_SEQUENCE_FOR(0x1000 * N + 0x0100 * j + 0x0002 + 0x0001 * i);

				WRITEOUT(pkptrout);
				WRITEOUT(datatotmpa);
				WRITEOUT(regtodata(N));
				WRITEOUT(0x0602 + 0x0100 * (i + 2 * j));
				WRITEOUT(done);
			}
		}
	}

	// ALU stuff
	for (N = 0; N < N_REGISTERS; N++) {
		for (M = 0; M < N_REGISTERS; M++) {
			for (k = 0; k < N_ALU_OPS; k++) {
				CODE_SEQUENCE_FOR(0x0010 + 0x1000 * N + 0x0100 * M + 0x0001 * k);

				WRITEOUT(ALUop(N, M, k));
				WRITEOUT(datatoreg(N));
				WRITEOUT(done);
			}
		}
	}

	//** FUNCTION STUFF **//

	// call
	CODE_SEQUENCE_FOR(0x0004);

	WRITEOUT(pkptrout);
	WRITEOUT(datatotmpa);
	WRITEOUT(incrementpk);
	WRITEOUT(decrementsp);
	WRITEOUT(sptoaddr);
	WRITEOUT(pktodata);
	WRITEOUT(writeRAM);
	WRITEOUT(tmpatodata);
	WRITEOUT(datatopk);
	WRITEOUT(done);

	// return
	CODE_SEQUENCE_FOR(0x0005);

	WRITEOUT(spptodata);
	WRITEOUT(datatopk);
	WRITEOUT(incrementsp);
	WRITEOUT(done);

	////////////////
	//************//
	//***PUSHES***//
	//************//
	////////////////

	// blank push
	CODE_SEQUENCE_FOR(0x0006);

	WRITEOUT(decrementsp);
	WRITEOUT(done);

	// push constant onto the stack
	CODE_SEQUENCE_FOR(0x0106);

	WRITEOUT(decrementsp);
	WRITEOUT(pkptrout);
	WRITEOUT(echodatalong);
	WRITEOUT(sptoaddr);
	WRITEOUT(writeRAM);
	WRITEOUT(incrementpk);
	WRITEOUT(done);

	// push *constant onto the stack
	CODE_SEQUENCE_FOR(0x0206);

	WRITEOUT(decrementsp);
	WRITEOUT(pkptrout);
	WRITEOUT(echodataaddr);
	WRITEOUT(readRAM);
	WRITEOUT(echodatalong);
	WRITEOUT(sptoaddr);
	WRITEOUT(writeRAM);
	WRITEOUT(incrementpk);
	WRITEOUT(done);

	for (N = 0; N < N_REGISTERS; N++) {
		// push register onto the stack
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0306);

		WRITEOUT(decrementsp);
		WRITEOUT(sptoaddr);
		WRITEOUT(regtodata(N));
		WRITEOUT(writeRAM);
		WRITEOUT(done);

		// push *register onto the stack
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0406);

		WRITEOUT(decrementsp);
		WRITEOUT(regptodata(N));
		WRITEOUT(echodatalong);
		WRITEOUT(sptoaddr);
		WRITEOUT(writeRAM);
		WRITEOUT(done);

		// push *(rN+offs)
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0506);

		WRITEOUT(pkptrout);
		WRITEOUT(datatooffs);
		WRITEOUT(incrementpk);
		WRITEOUT(decrementsp);
		WRITEOUT(regptodatao(N));
		WRITEOUT(echodatalong);
		WRITEOUT(sptoaddr);
		WRITEOUT(writeRAM);
		WRITEOUT(done);
	}

	for (N = 0; N < N_SPECIAL_REGS; N++) {
		// push srN onto the stack
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0306);

		WRITEOUT(decrementsp);
		WRITEOUT(sptoaddr);
		WRITEOUT(regtodata(N));
		WRITEOUT(writeRAM);
		WRITEOUT(done);

		// push *srN onto the stack
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0406);

		WRITEOUT(decrementsp);
		WRITEOUT(specialptodata(N));
		WRITEOUT(echodatalong);
		WRITEOUT(sptoaddr);
		WRITEOUT(writeRAM);
		WRITEOUT(done);

		// push *(srN+offs)
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0506);

		WRITEOUT(pkptrout);
		WRITEOUT(datatooffs);
		WRITEOUT(incrementpk);
		WRITEOUT(decrementsp);
		WRITEOUT(specialptodatao(N));
		WRITEOUT(echodatalong);
		WRITEOUT(sptoaddr);
		WRITEOUT(writeRAM);
		WRITEOUT(done);
	}

	//////////////
	//**********//
	//***POPS***//
	//**********//
	//////////////

	// pop to nowhere
	CODE_SEQUENCE_FOR(0x0007);

	WRITEOUT(incrementsp);
	WRITEOUT(done);

	// pop to *constant
	CODE_SEQUENCE_FOR(0x0207);

	WRITEOUT(pkptrout);
	WRITEOUT(datatotmpa);
	WRITEOUT(spptodata);
	WRITEOUT(echodatalong);
	WRITEOUT(tmpatoaddr);
	WRITEOUT(writeRAM);
	WRITEOUT(incrementpk);
	WRITEOUT(incrementsp);
	WRITEOUT(done);

	for (N = 0; N < N_REGISTERS; N++) {
		// pop top of stack into rN
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0307);

		WRITEOUT(spptodata);
		WRITEOUT(datatoreg(N));
		WRITEOUT(incrementsp);
		WRITEOUT(done);

		// pop top of stack into *rN
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0407);

		WRITEOUT(spptodata);
		WRITEOUT(echodatalong);
		WRITEOUT(regtoaddr(N));
		WRITEOUT(writeRAM);
		WRITEOUT(incrementsp);
		WRITEOUT(done);

		// pop top of stack into *(rN+offs)
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0507);

		WRITEOUT(pkptrout);
		WRITEOUT(datatooffs);
		WRITEOUT(incrementpk);
		WRITEOUT(spptodata);
		WRITEOUT(echodatalong);
		WRITEOUT(regtoaddr(N));
		WRITEOUT(writeRAMo);
		WRITEOUT(incrementsp);
		WRITEOUT(done);
	}

	for (N = 0; N < N_REGISTERS; N++) {
		// pop top of stack into srN
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0607);

		WRITEOUT(spptodata);
		WRITEOUT(datatospecial(N));
		WRITEOUT(incrementsp);
		WRITEOUT(done);

		// pop top of stack into *srN
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0707);

		WRITEOUT(spptodata);
		WRITEOUT(echodatalong);
		WRITEOUT(specialtoaddr(N));
		WRITEOUT(writeRAM);
		WRITEOUT(incrementsp);
		WRITEOUT(done);

		// pop top of stack into *(srN+offs)
		CODE_SEQUENCE_FOR(0x1000 * N + 0x0807);

		WRITEOUT(pkptrout);
		WRITEOUT(datatooffs);
		WRITEOUT(incrementpk);
		WRITEOUT(spptodata);
		WRITEOUT(echodatalong);
		WRITEOUT(specialtoaddr(N));
		WRITEOUT(writeRAMo);
		WRITEOUT(incrementsp);
		WRITEOUT(done);
	}

	/////////////////
	//*************//
	//***SETTING***//
	//*************//
	/////////////////

	//~ 0x0030, CONSTANT | POINTER, FOLLOWING(1),   CONSTANT, FOLLOWING(2)), set
	// *constant to constant
	CODE_SEQUENCE_FOR(0x0030);

	WRITEOUT(pkptrout);
	WRITEOUT(datatotmpa);
	WRITEOUT(incrementpk);
	WRITEOUT(pkptrout);
	WRITEOUT(echodatalong)
	WRITEOUT(tmpatoaddr);
	WRITEOUT(writeRAM);
	WRITEOUT(incrementpk);
	WRITEOUT(done);

	//~ 0x0130, CONSTANT | POINTER, FOLLOWING(1),   CONSTANT | POINTER,
	//FOLLOWING(2)), set *constant to *constant
	CODE_SEQUENCE_FOR(0x0130);

	WRITEOUT(pkptrout);
	WRITEOUT(datatotmpa);
	WRITEOUT(incrementpk);
	WRITEOUT(pkptrout);
	WRITEOUT(echodataaddr);
	WRITEOUT(readRAM);
	WRITEOUT(echodatalong);
	WRITEOUT(tmpatoaddr);
	WRITEOUT(writeRAM);
	WRITEOUT(done);

	for (N = 0; N < N_REGISTERS; N++) {
		// set *constant to rN
		//~ 0x0031, CONSTANT | POINTER, FOLLOWING(1),   REGISTER, FIRST_NIBBLE)
		CODE_SEQUENCE_FOR(0x0031 + 0x1000 * N);

		WRITEOUT(pkptrout);
		WRITEOUT(echodataaddr);
		WRITEOUT(regtodata(N));
		WRITEOUT(writeRAM);
		WRITEOUT(incrementpk);
		WRITEOUT(done);

		// set *constant to *rN
		//~ 0x0131, CONSTANT | POINTER, FOLLOWING(1),   REGISTER | POINTER,
		//FIRST_NIBBLE)
		CODE_SEQUENCE_FOR(0x0131 + 0x1000 * N);

		WRITEOUT(pkptrout);
		WRITEOUT(datatotmpa);
		WRITEOUT(regptodata(N));
		WRITEOUT(echodatalong);
		WRITEOUT(tmpatoaddr);
		WRITEOUT(writeRAM);
		WRITEOUT(done);

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

		WRITEOUT(pkptrout);
		WRITEOUT(echodataaddr);
		WRITEOUT(specialtodata(N));
		WRITEOUT(writeRAM);
		WRITEOUT(incrementpk);
		WRITEOUT(done);

		// set *constant to srN
		//~ 0x0431, CONSTANT | POINTER, FOLLOWING(1), S_REGISTER | POINTER,
		//FIRST_NIBBLE)
		CODE_SEQUENCE_FOR(0x0431 + 0x1000 * N);

		WRITEOUT(pkptrout);
		WRITEOUT(datatotmpa);
		WRITEOUT(specialptodata(N));
		WRITEOUT(echodatalong);
		WRITEOUT(tmpatoaddr);
		WRITEOUT(writeRAM);
		WRITEOUT(done);

		// set *constant to *(srN+offs)
		//~ 0x0531, CONSTANT | POINTER, FOLLOWING(1), S_REGISTER | POINTER |
		//W_OFFSET, FIRST_NIBBLE | FOLLOWING(2))
		CODE_SEQUENCE_FOR(0x0531 + 0x1000 * N);
		// implement
	}

	for (N = 0; N < N_REGISTERS; N++) {
		// set rN to constant
		//~ 0x0033, REGISTER, FIRST_NIBBLE, CONSTANT,           FOLLOWING(1)),
		CODE_SEQUENCE_FOR(0x0033 + 0x1000 * N);

		WRITEOUT(pkptrout);
		WRITEOUT(datatoreg(N));
		WRITEOUT(incrementpk);
		WRITEOUT(done);

		// set rN to *constant
		//~ 0x0034, REGISTER, FIRST_NIBBLE, CONSTANT | POINTER, FOLLOWING(1)),
		CODE_SEQUENCE_FOR(0x0034 + 0x1000 * N);

		WRITEOUT(pkptrout);
		WRITEOUT(echodataaddr);
		WRITEOUT(readRAM);
		WRITEOUT(datatoreg(N));
		WRITEOUT(incrementpk);
		WRITEOUT(done);

		// set *rN to constant
		//~ 0x003b, REGISTER | POINTER, FIRST_NIBBLE, CONSTANT, FOLLOWING(1)),
		CODE_SEQUENCE_FOR(0x003b + 0x1000 * N);

		WRITEOUT(pkptrout);
		WRITEOUT(datatotmpa);
		WRITEOUT(regtoaddr(N));
		WRITEOUT(tmpatodata);
		WRITEOUT(writeRAM);
		WRITEOUT(incrementpk);
		WRITEOUT(done);

		// set *rN to *constant
		//~ 0x003c, REGISTER | POINTER, FIRST_NIBBLE, CONSTANT | POINTER,
		//FOLLOWING(1)),
		CODE_SEQUENCE_FOR(0x003c + 0x1000 * N);

		WRITEOUT(pkptrout);
		WRITEOUT(datatotmpa);
		WRITEOUT(regtoaddr(N));
		WRITEOUT(tmpaptodata);
		WRITEOUT(writeRAM);
		WRITEOUT(incrementpk);
		WRITEOUT(done);

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

			WRITEOUT(0x1000 * M + 0x0003);
			WRITEOUT(datatoreg(N));
			WRITEOUT(done);

			// set rN to *rM
			//~ 0x0036, REGISTER, FIRST_NIBBLE, REGISTER | POINTER, SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x0036 + 0x1000 * N + 0x0100 * M);

			WRITEOUT(regptodata(M));
			WRITEOUT(datatoreg(N));
			WRITEOUT(done);

			// set rN to *(rM+offs)
			//~ 0x0037, REGISTER, FIRST_NIBBLE, REGISTER | POINTER | W_OFFSET,
			//SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
			CODE_SEQUENCE_FOR(0x0037 + 0x1000 * N + 0x0100 * M);

			WRITEOUT(pkptrout);
			WRITEOUT(datatooffs);
			WRITEOUT(incrementpk);
			WRITEOUT(regptodatao(M));
			WRITEOUT(datatoreg(N));
			WRITEOUT(done);

			// set *rN to rM
			//~ 0x003d, REGISTER | POINTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x003d + 0x1000 * N + 0x0100 * M);

			WRITEOUT(regtodata(M));
			WRITEOUT(echodatalong);
			WRITEOUT(regtoaddr(N));
			WRITEOUT(writeRAM);
			WRITEOUT(done);

			// set *rN to *rM
			//~ 0x003e, REGISTER | POINTER, FIRST_NIBBLE, REGISTER | POINTER,
			//SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x003e + 0x1000 * N + 0x0100 * M);

			WRITEOUT(regptodata(M));
			WRITEOUT(echodatalong);
			WRITEOUT(regtoaddr(N));
			WRITEOUT(writeRAM);
			WRITEOUT(done);

			// set *rN to *(rM+offs)
			//~ 0x003f, REGISTER | POINTER, FIRST_NIBBLE, REGISTER | POINTER |
			//W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
			CODE_SEQUENCE_FOR(0x003f + 0x1000 * N + 0x0100 * M);

			WRITEOUT(pkptrout);
			WRITEOUT(datatooffs);
			WRITEOUT(regptodatao(M));
			WRITEOUT(echodatalong);
			WRITEOUT(regtoaddr(N));
			WRITEOUT(writeRAM);
			WRITEOUT(incrementpk);
			WRITEOUT(done);

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

			WRITEOUT(specialtodata(M));
			WRITEOUT(datatoreg(N));
			WRITEOUT(done);

			// set rN to *srM
			//~ 0x0039, REGISTER, FIRST_NIBBLE, S_REGISTER | POINTER, SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x0039 + 0x1000 * N + 0x0100 * M);

			WRITEOUT(specialptodata(M));
			WRITEOUT(datatoreg(N));
			WRITEOUT(done);

			// set rN to *(srM+offs)
			//~ 0x003a, REGISTER, FIRST_NIBBLE, S_REGISTER | POINTER | W_OFFSET,
			//SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
			CODE_SEQUENCE_FOR(0x003a + 0x1000 * N + 0x0100 * M);

			WRITEOUT(pkptrout);
			WRITEOUT(datatooffs);
			WRITEOUT(specialptodatao(M));
			WRITEOUT(datatoreg(N));
			WRITEOUT(incrementpk);
			WRITEOUT(done);

			// set *rN to srM
			//~ 0x0040, REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER, SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x0040 + 0x1000 * N + 0x0100 * M);

			WRITEOUT(specialtodata(M));
			WRITEOUT(echodatalong);
			WRITEOUT(regtoaddr(N));
			WRITEOUT(writeRAM);
			WRITEOUT(done);

			// set *rN to *srM
			//~ 0x0041, REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER | POINTER,
			//SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x0041 + 0x1000 * N + 0x0100 * M);

			WRITEOUT(specialptodata(M));
			WRITEOUT(echodatalong);
			WRITEOUT(regtoaddr(N));
			WRITEOUT(writeRAM);
			WRITEOUT(done);

			// set *rN to *(srM+offs)
			//~ 0x0042, REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER | POINTER |
			//W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
			CODE_SEQUENCE_FOR(0x0042 + 0x1000 * N + 0x0100 * M);

			WRITEOUT(pkptrout);
			WRITEOUT(datatooffs);
			WRITEOUT(specialptodatao(M));
			WRITEOUT(echodatalong);
			WRITEOUT(regtoaddr(N));
			WRITEOUT(writeRAM);
			WRITEOUT(incrementpk);
			WRITEOUT(done);

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

		WRITEOUT(pkptrout);
		WRITEOUT(datatooffs);
		WRITEOUT(incrementpk);
		WRITEOUT(pkptrout);
		WRITEOUT(echodatalong);
		WRITEOUT(specialtoaddr(N));
		WRITEOUT(writeRAMo);
		WRITEOUT(incrementpk);
		WRITEOUT(done);

		// set *(srN+offs) to *constant
		//~ 0x0059, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE |
		//OFFSET_FOLLOWING(1), CONSTANT | POINTER, FOLLOWING(2)),
		CODE_SEQUENCE_FOR(0x0059 + 0x1000 * N);

		WRITEOUT(pkptrout);
		WRITEOUT(datatooffs);
		WRITEOUT(incrementpk);
		WRITEOUT(pkptrout);
		WRITEOUT(datatotmpa);
		WRITEOUT(tmpaptodata);
		WRITEOUT(echodatalong);
		WRITEOUT(specialtoaddr(N));
		WRITEOUT(writeRAMo);
		WRITEOUT(incrementpk);
		WRITEOUT(done);

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

			WRITEOUT(pkptrout);
			WRITEOUT(datatooffs);
			WRITEOUT(specialtoaddr(N));
			WRITEOUT(regtodata(M));
			WRITEOUT(writeRAMo);
			WRITEOUT(incrementpk);
			WRITEOUT(done);

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

	fwrite(mr_data, 2, NWORDS, mainROM);
	fclose(mainROM);
	fwrite(dr_data, 2, NWORDS, decoderROM);
	fclose(decoderROM);

	free(mr_data);
	free(dr_data);

	return EXIT_SUCCESS;
}
