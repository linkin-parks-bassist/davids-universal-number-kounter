#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define N_REGISTERS 16
#define N_SPECIAL_REGS 5

#define N_ALU_OPS 8

#define do_nothing WRITEOUT(0x00); WRITEOUT(0x00);

#define begini WRITEOUT(0x00); WRITEOUT(0x01);

#define specialtodata(X)   WRITEOUT(0x10 * X);        WRITEOUT(0x02);
#define specialtoaddr(X)   WRITEOUT(0x10 * X + 0x01); WRITEOUT(0x02);
#define specialptodata(X)  WRITEOUT(0x10 * X + 0x02); WRITEOUT(0x02);
#define specialptodatao(X) WRITEOUT(0x10 * X + 0x03); WRITEOUT(0x02);
#define datatospecial(X)   WRITEOUT(0x10 * X + 0x04); WRITEOUT(0x02);

#define pktodata  WRITEOUT(0x00); WRITEOUT(0x02);
#define pktoaddr  WRITEOUT(0x01); WRITEOUT(0x02);
#define pkptrout  WRITEOUT(0x02); WRITEOUT(0x02);
#define pkoptrout WRITEOUT(0x03); WRITEOUT(0x02);
#define datatopk  WRITEOUT(0x04); WRITEOUT(0x02);

#define sptodata   WRITEOUT(0x10); WRITEOUT(0x02);
#define sptoaddr   WRITEOUT(0x11); WRITEOUT(0x02);
#define spptodata  WRITEOUT(0x12); WRITEOUT(0x02);
#define spptodatao WRITEOUT(0x13); WRITEOUT(0x02);
#define datatosp   WRITEOUT(0x14); WRITEOUT(0x02);

#define tmpatodata   WRITEOUT(0x20); WRITEOUT(0x02);
#define tmpatoaddr   WRITEOUT(0x21); WRITEOUT(0x02);
#define tmpaptodata  WRITEOUT(0x22); WRITEOUT(0x02);
#define tmpaptodatao WRITEOUT(0x23); WRITEOUT(0x02);
#define datatotmpa   WRITEOUT(0x24); WRITEOUT(0x02);

#define tmpbtodata   WRITEOUT(0x30); WRITEOUT(0x02);
#define tmpbtoaddr   WRITEOUT(0x31); WRITEOUT(0x02);
#define tmpbptodata  WRITEOUT(0x32); WRITEOUT(0x02);
#define tmpbptodatao WRITEOUT(0x33); WRITEOUT(0x02);
#define datatotmpb   WRITEOUT(0x34); WRITEOUT(0x02);

#define offstodata   WRITEOUT(0x40); WRITEOUT(0x02);
#define offstoaddr   WRITEOUT(0x41); WRITEOUT(0x02);
#define offsptodata  WRITEOUT(0x42); WRITEOUT(0x02);
#define offsptodatao WRITEOUT(0x43); WRITEOUT(0x02);
#define datatooffs   WRITEOUT(0x44); WRITEOUT(0x02);

#define setoffs(X)   WRITEOUT(0x40 + 0x01 * X); WRITEOUT(0x02);

#define incrementpk WRITEOUT(0x05); WRITEOUT(0x02);
#define incrementsp WRITEOUT(0x15); WRITEOUT(0x02);
#define tmpatopk    WRITEOUT(0x25); WRITEOUT(0x02);
#define tmpbtopk    WRITEOUT(0x35); WRITEOUT(0x02);

#define decrementsp WRITEOUT(0x16); WRITEOUT(0x02);

#define tmpatopkifz  WRITEOUT(0x06); WRITEOUT(0x02);
#define tmpatopkifnz WRITEOUT(0x07); WRITEOUT(0x02);

#define tmpatopkifn  WRITEOUT(0x08); WRITEOUT(0x02);
#define tmpatopkifnn WRITEOUT(0x09); WRITEOUT(0x02);

#define tmpatopkifp  WRITEOUT(0x0a); WRITEOUT(0x02);
#define tmpatopkifnp WRITEOUT(0x0b); WRITEOUT(0x02);

#define regtodata(X)   WRITEOUT(0x10 * X); WRITEOUT(0x03);
#define regtoaddr(X)   WRITEOUT(0x10 * X + 0x01); WRITEOUT(0x03);
#define regptodata(X)  WRITEOUT(0x10 * X + 0x02); WRITEOUT(0x03);
#define regptodatao(X) WRITEOUT(0x10 * X + 0x03); WRITEOUT(0x03);
#define datatoreg(X)   WRITEOUT(0x10 * X + 0x04); WRITEOUT(0x03);

#define writeRAM       WRITEOUT(0x00); WRITEOUT(0x04);
#define writeRAMo      WRITEOUT(0x01); WRITEOUT(0x04);
#define writeBytes(X)  WRITEOUT(0x10 * X + 0x00); WRITEOUT(0x04);
#define writeByteso(X) WRITEOUT(0x10 * X + 0x01); WRITEOUT(0x04);

#define readRAM        WRITEOUT(0x00); WRITEOUT(0x05);
#define readRAMo       WRITEOUT(0x01); WRITEOUT(0x05);
#define readBytes(X)   WRITEOUT(0x10 * X + 0x00); WRITEOUT(0x05);
#define readByteso(X)  WRITEOUT(0x10 * X + 0x01); WRITEOUT(0x05);

#define ALUop(X, Y, Z) WRITEOUT(0x10 * X + 0x01 * Y); WRITEOUT(0x10 + 0x01 * Z);

#define echodata         WRITEOUT(0x00); WRITEOUT(0x20);
#define echodataaddr     WRITEOUT(0x00); WRITEOUT(0x21);
#define echoaddrdata     WRITEOUT(0x00); WRITEOUT(0x22);
#define echoaddraddr     WRITEOUT(0x00); WRITEOUT(0x23);
#define echoaddraddrlong WRITEOUT(0x00); WRITEOUT(0x24);
#define echodatalong     WRITEOUT(0x00); WRITEOUT(0x25);
#define echoaddrdatalong WRITEOUT(0x00); WRITEOUT(0x26);

#define readbitport    WRITEOUT(0x00); WRITEOUT(0xa0);
#define writebitport   WRITEOUT(0x00); WRITEOUT(0xa1);
#define readbitportb   WRITEOUT(0x00); WRITEOUT(0xa2);
#define writebitportb  WRITEOUT(0x00); WRITEOUT(0xa3);
#define readwideport   WRITEOUT(0x00); WRITEOUT(0xa4);
#define writewideport  WRITEOUT(0x00); WRITEOUT(0xa5);

#define biosromtodata  WRITEOUT(0x00); WRITEOUT(0xb0);
#define setrunbitlow   WRITEOUT(0xff); WRITEOUT(0x00);
#define setrunbithigh  WRITEOUT(0xff); WRITEOUT(0x01);

#define fetch_instruction WRITEOUT(0xff); WRITEOUT(0x0f);
#define done  			  WRITEOUT(0xff); WRITEOUT(0xfe);
#define reset 			  WRITEOUT(0xff); WRITEOUT(0xff);

#define BINSIZE 2*64*1024
#define NWORDS  2*64*1024

#define WRITEOUT(X) mr_data[current_position] = X; current_position += 1;
#define CODE_SEQUENCE_FOR(X, Y) dr_data[2 * (Y + X * 0x10)] = (current_position >> 9); dr_data[2 * (Y + X * 0x10) + 1] = (current_position << 8) >> 9;

// Main function to handle input arguments
int main(int argc, char *argv[]) {
    // Write processed data to output file
    FILE *mainROM = fopen("mainROM", "wb");
    if (!mainROM) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }
    
    FILE *decoderROM = fopen("decocerROM", "wb");
    if (!decoderROM) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }

	uint8_t *mr_data = malloc(BINSIZE);
	for (int i = 0; i<NWORDS; i++){
		mr_data[i] = 0;
	}
	uint8_t *dr_data = malloc(BINSIZE);
	for (int i = 0; i<NWORDS; i++){
		dr_data[i] = 0;
	}
	
	uint16_t current_position = 0;
	int N;
	int M;
	int k;
	uint8_t *word_ptr = &mr_data[current_position];
	
	//BIOS-LOAD
	WRITEOUT(0x00); WRITEOUT(0x11); //subtract register 0 from itself and put it on data bus
	datatoreg(0)                    //save data bus to register 0, ensuring it contains 0
	setrunbitlow                    //sure the run bit is 0
	
	//BIOS-LOAD LOOP
	printf("BIOS load loop start: 0x%x\n", current_position/2);
	
	regtoaddr(0)   					//use register 0 to index both the BIOS rom and the main memory
	biosromtodata  					//read the current byte of the BIOS rom to data
	writeRAM	   					//write it to memory
	WRITEOUT(0x00); WRITEOUT(0x13); //incrememebt register 0 and output it to data
	datatoreg(0)					//save it to the register
	done

	
	//BEGIN INSTRUCTION LOOP
	printf("run loop start: 0x%x\n", current_position/2);
	fetch_instruction
	incrementpk
	do_nothing
	begini
	
	//chill
	CODE_SEQUENCE_FOR(0x00,0x00); 
	
	WRITEOUT(0x00); WRITEOUT(0x00);
	done
	
	CODE_SEQUENCE_FOR(0x01,0x00); //goto
	
	pktoaddr
	readBytes(4)
	datatopk
	done
	
	//the various conditional gotos
	for (N = 0; N < N_REGISTERS; N++) {
		for (int j = 0; j < 3; j++) {
			for (int i = 0; i < 2; i++) {
				CODE_SEQUENCE_FOR(0x10 * N + 0x01 * j, 0x02 + 0x01 * i);
		
				pktoaddr readBytes(4)
				datatotmpa
				regtodata(N)
				WRITEOUT(0x06 + 0x01 * (i + 2 * j)); WRITEOUT(0x02);
				done
			}
		}
	}
	
	//ALU stuff
	for (N = 0; N<N_REGISTERS; N++) {
		for (M = 0; M<N_REGISTERS; M++) {
			for (k = 0; k<N_ALU_OPS; k++) {
				CODE_SEQUENCE_FOR(0x10 * N + 0x01 * M, 0x01 * k + 0x10);
				
				ALUop(N, M, k)
				datatoreg(N)
				done
			}
		}
	}
	
	//** FUNCTION STUFF **//
	
	//call
	CODE_SEQUENCE_FOR(0x04,0x00);
	
	pktoaddr readBytes(4)
	datatotmpa
	incrementpk
	do_nothing
	incrementpk
	do_nothing
	incrementpk 
	do_nothing
	incrementpk
    decrementsp
    do_nothing
    decrementsp
    do_nothing
    decrementsp
    do_nothing
    decrementsp
    sptoaddr
    pktodata
    writeBytes(0xf)
    decrementsp
    tmpatodata
    datatopk
    done
    
	//return
	CODE_SEQUENCE_FOR(0x05,0x00);
	
    incrementsp
    do_nothing
    incrementsp
    do_nothing
    incrementsp
    do_nothing
    incrementsp
    do_nothing
	sptoaddr
	readBytes(4)
	datatopk
	done
	
	////////////////
	//************//
	//***PUSHES***//
	//************//
	////////////////
	
	//blank push
	CODE_SEQUENCE_FOR(0x06,0x00);
	
	decrementsp
	done
	
	//push constant onto the stack
	CODE_SEQUENCE_FOR(0x06,0x01);
	
	decrementsp
	pkptrout
	echodatalong
	sptoaddr
	writeRAM
	incrementpk
	done
	
	//push *constant onto the stack
	CODE_SEQUENCE_FOR(0x06,0x02);
	
	decrementsp
	pkptrout
	echodataaddr
	readRAM
	echodatalong
	sptoaddr
	writeRAM
	incrementpk
	done
	
	for (N = 0; N<N_REGISTERS; N++) {
		//push register onto the stack
		CODE_SEQUENCE_FOR(0x06,0x10 * N + 0x03);
		
		decrementsp
		sptoaddr
		regtodata(N)
		writeRAM
		done
		
		//push *register onto the stack
		CODE_SEQUENCE_FOR(0x06,0x10 * N + 0x04);
		
		decrementsp
		regptodata(N)
		echodatalong
		sptoaddr
		writeRAM
		done
		
		//push *(rN+offs)
		CODE_SEQUENCE_FOR(0x06,0x10 * N + 0x05);
		
		pkptrout
		datatooffs
		incrementpk
		decrementsp
		regptodatao(N)
		echodatalong
		sptoaddr
		writeRAM
		done
	}
	
	for (N = 0; N < N_SPECIAL_REGS; N++) {
		//push srN onto the stack
		CODE_SEQUENCE_FOR(0x06,0x10 * N + 0x03);
		
		decrementsp
		sptoaddr
		regtodata(N)
		writeRAM
		done
		
		//push *srN onto the stack
		CODE_SEQUENCE_FOR(0x06,0x10 * N + 0x04);
		
		decrementsp
		specialptodata(N)
		echodatalong
		sptoaddr
		writeRAM
		done
		
		//push *(srN+offs)
		CODE_SEQUENCE_FOR(0x06,0x10 * N + 0x05);
		
		pkptrout
		datatooffs
		incrementpk
		decrementsp
		specialptodatao(N)
		echodatalong
		sptoaddr
		writeRAM
		done
	}
	
	printf("hi\n");
	
	//////////////
	//**********//
	//***POPS***//
	//**********//
	//////////////
	
	//pop to nowhere
	CODE_SEQUENCE_FOR(0x07,0x00);
	
	incrementsp
	done
	
	//pop to *constant
	CODE_SEQUENCE_FOR(0x07,0x02);
	
	pkptrout
	datatotmpa
	spptodata
	echodatalong
	tmpatoaddr
	writeRAM
	incrementpk
	incrementsp
	done
	
	for (N = 0; N<N_REGISTERS; N++) {
		//pop top of stack into rN
		CODE_SEQUENCE_FOR(0x07,0x10 * N + 0x03);
		
		spptodata
		datatoreg(N)
		incrementsp
		done
		
		//pop top of stack into *rN
		CODE_SEQUENCE_FOR(0x07,0x10 * N + 0x04);
		
		spptodata
		echodatalong
		regtoaddr(N)
		writeRAM
		incrementsp
		done
		
		//pop top of stack into *(rN+offs)
		CODE_SEQUENCE_FOR(0x07,0x10 * N + 0x05);
		
		pkptrout
		datatooffs
		incrementpk
		spptodata
		echodatalong
		regtoaddr(N)
		writeRAMo
		incrementsp
		done
	}
	
	for (N = 0; N<N_REGISTERS; N++) {
		//pop top of stack into srN
		CODE_SEQUENCE_FOR(0x07,0x10 * N + 0x06);
		
		spptodata
		datatospecial(N)
		incrementsp
		done
		
		//pop top of stack into *srN
		CODE_SEQUENCE_FOR(0x07,0x10 * N + 0x07);
		
		spptodata
		echodatalong
		specialtoaddr(N)
		writeRAM
		incrementsp
		done
		
		//pop top of stack into *(srN+offs)
		CODE_SEQUENCE_FOR(0x07,0x10 * N + 0x08);
		
		pkptrout
		datatooffs
		incrementpk
		spptodata
		echodatalong
		specialtoaddr(N)
		writeRAMo
		incrementsp
		done
	}
	
	/////////////////
	//*************//
	//***SETTING***//
	//*************//
	/////////////////
	
	//~ 0x0030, CONSTANT | POINTER, FOLLOWING(1),   CONSTANT,                      FOLLOWING(2)),
	//set *constant to constant
	CODE_SEQUENCE_FOR(0x30,0x00);
	
	pkptrout
	datatotmpa
	incrementpk
	pkptrout
	echodatalong
	tmpatoaddr
	writeRAM
	incrementpk
	done
	
	//~ 0x0130, CONSTANT | POINTER, FOLLOWING(1),   CONSTANT | POINTER,            FOLLOWING(2)),
	//set *constant to *constant
	CODE_SEQUENCE_FOR(0x30,0x01);
	
	pkptrout
	datatotmpa
	incrementpk
	pkptrout
	echodataaddr
	readRAM
	echodatalong
	tmpatoaddr
	writeRAM
	done
	
	for (N = 0; N<N_REGISTERS; N++) {
		//set *constant to rN
		//~ 0x0031, CONSTANT | POINTER, FOLLOWING(1),   REGISTER,                      FIRST_NIBBLE)
		CODE_SEQUENCE_FOR(0x31,0x10 * N);

		pkptrout
		echodataaddr
		regtodata(N)
		writeRAM
		incrementpk
		done
		
		
		//set *constant to *rN
		//~ 0x0131, CONSTANT | POINTER, FOLLOWING(1),   REGISTER | POINTER,            FIRST_NIBBLE)
		CODE_SEQUENCE_FOR(0x31,0x10 * N + 0x01);
		
		pkptrout
        datatotmpa
        regptodata(N)
        echodatalong
        tmpatoaddr
        writeRAM
        done
		
		//set *constant to *(rN+offs)
		//~ 0x0231, CONSTANT | POINTER, FOLLOWING(1),   REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | FOLLOWING(2))
		CODE_SEQUENCE_FOR(0x31,0x10 * N + 0x02);
		//implement
	}
	
	printf("hi\n");
	for (N = 0; N < N_SPECIAL_REGS; N++) {
		//set *constant to srN
		//~ 0x0331, CONSTANT | POINTER, FOLLOWING(1), S_REGISTER,                      FIRST_NIBBLE)
		CODE_SEQUENCE_FOR(0x31,0x10 * N + 0x03);
		
		pkptrout
		echodataaddr
		specialtodata(N)
		writeRAM
		incrementpk
		done
		
		//set *constant to srN
		//~ 0x0431, CONSTANT | POINTER, FOLLOWING(1), S_REGISTER | POINTER,            FIRST_NIBBLE)
		CODE_SEQUENCE_FOR(0x31,0x10 * N + 0x04);
		
		pkptrout
        datatotmpa
        specialptodata(N)
        echodatalong
        tmpatoaddr
        writeRAM
        done
		
		
		//set *constant to *(srN+offs)
		//~ 0x0531, CONSTANT | POINTER, FOLLOWING(1), S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | FOLLOWING(2))
		CODE_SEQUENCE_FOR(0x31,0x10 * N + 0x05);
		//implement
	}
	
	for (N = 0; N<N_REGISTERS; N++) {
		//set rN to constant
		//~ 0x0033, REGISTER, FIRST_NIBBLE, CONSTANT,           FOLLOWING(1)),
		CODE_SEQUENCE_FOR(0x33,0x10 * N);
		
		pkptrout
		datatoreg(N)
		incrementpk
		done
		
		//set rN to *constant
		//~ 0x0034, REGISTER, FIRST_NIBBLE, CONSTANT | POINTER, FOLLOWING(1)),
		CODE_SEQUENCE_FOR(0x34,0x10 * N);
		
		pkptrout
		echodataaddr
		readRAM
		datatoreg(N)
		incrementpk
		done
		
		//set *rN to constant
		//~ 0x003b, REGISTER | POINTER, FIRST_NIBBLE, CONSTANT,           FOLLOWING(1)),
		CODE_SEQUENCE_FOR(0x3b,0x10 * N);
		
		pkptrout
		datatotmpa
		regtoaddr(N)
		tmpatodata
		writeRAM
		incrementpk
		done
		
		//set *rN to *constant
		//~ 0x003c, REGISTER | POINTER, FIRST_NIBBLE, CONSTANT | POINTER, FOLLOWING(1)),
		CODE_SEQUENCE_FOR(0x3c,0x10 * N);
		
		pkptrout
		datatotmpa
		regtoaddr(N)
		tmpaptodata
		writeRAM
		incrementpk
		done
		
		//set *(rN+offs) to constant
		//~ 0x0043, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), CONSTANT,           FOLLOWING(2)),
		CODE_SEQUENCE_FOR(0x43,0x10 * N);
		//implement
		
		//set *(rN+offs) to *constant
		//~ 0x0143, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), CONSTANT | POINTER, FOLLOWING(2)),
		CODE_SEQUENCE_FOR(0x43,0x01);
		//implement
		
		for (M = 0; M < N_REGISTERS; M++) {
			//set rN to rM
			//~ 0x0035, REGISTER, FIRST_NIBBLE, REGISTER,                      SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x35,0x10 * N + 0x01 * M);
			
			WRITEOUT(0x10 * M); WRITEOUT(0x03);
			datatoreg(N)
			done
			
			//set rN to *rM
			//~ 0x0036, REGISTER, FIRST_NIBBLE, REGISTER | POINTER,            SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x36,0x10 * N + 0x01 * M);
			
			regptodata(M)
			datatoreg(N)
			done
			
			//set rN to *(rM+offs)
			//~ 0x0037, REGISTER, FIRST_NIBBLE, REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
			CODE_SEQUENCE_FOR(0x37,0x10 * N + 0x01 * M);
			
			pkptrout
			datatooffs
			incrementpk
			regptodatao(M)
			datatoreg(N)
			done
			
			//set *rN to rM
			//~ 0x003d, REGISTER | POINTER, FIRST_NIBBLE, REGISTER,           SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x3d,0x10 * N + 0x01 * M);
			
			regtodata(M)
			echodatalong
			regtoaddr(N)
			writeRAM
			done
			
			//set *rN to *rM
			//~ 0x003e, REGISTER | POINTER, FIRST_NIBBLE, REGISTER | POINTER, SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x3e,0x10 * N + 0x01 * M);
			
			regptodata(M)
			echodatalong
			regtoaddr(N)
			writeRAM
			done
			
			//set *rN to *(rM+offs)
			//~ 0x003f, REGISTER | POINTER, FIRST_NIBBLE, REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
			CODE_SEQUENCE_FOR(0x3f,0x10 * N + 0x01 * M);
			
			pkptrout
			datatooffs
			regptodatao(M)
			echodatalong
			regtoaddr(N)
			writeRAM
			incrementpk
			done
			
			//set *(rN+offs) to rM
			//~ 0x0044, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), REGISTER,                      SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x44,0x10 * N + 0x01 * M);
			
			
			//set *(rN+offs) to *rM
			//~ 0x0045, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), REGISTER | POINTER,            SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x45,0x10 * N + 0x01 * M);
			
			
			//set *(rN+offs) to *(rM+offs)
			//~ 0x0046, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(2)),
			CODE_SEQUENCE_FOR(0x46,0x10 * N + 0x01 * M);
			
		}
		
		for (M = 0; M < N_SPECIAL_REGS; M++) {
			//set rN to srM
			//~ 0x0038, REGISTER, FIRST_NIBBLE, S_REGISTER,                      SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x38,0x10 * N + 0x01 * M);
			
			specialtodata(M)
			datatoreg(N)
			done
			
			//set rN to *srM
			//~ 0x0039, REGISTER, FIRST_NIBBLE, S_REGISTER | POINTER,            SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x39,0x10 * N + 0x01 * M);
			
			specialptodata(M)
			datatoreg(N)
			done
			
			//set rN to *(srM+offs)
			//~ 0x003a, REGISTER, FIRST_NIBBLE, S_REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
			CODE_SEQUENCE_FOR(0x3a,0x10 * N + 0x01 * M);
			
			pkptrout
            datatooffs
            specialptodatao(M)
            datatoreg(N)
            incrementpk
            done
			
			//set *rN to srM
			//~ 0x0040, REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER,           SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x40,0x10 * N + 0x01 * M);
			
			specialtodata(M)
			echodatalong
			regtoaddr(N)
			writeRAM
			done
			
			//set *rN to *srM
			//~ 0x0041, REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER | POINTER, SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x41,0x10 * N + 0x01 * M);
			
			specialptodata(M)
			echodatalong
			regtoaddr(N)
			writeRAM
			done
			
			//set *rN to *(srM+offs)
			//~ 0x0042, REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
			CODE_SEQUENCE_FOR(0x42,0x10 * N + 0x01 * M);
			
			pkptrout
			datatooffs
			specialptodatao(M)
			echodatalong
			regtoaddr(N)
			writeRAM
			incrementpk
			done
			
			//set *(rN+offs) to srM
			//~ 0x0047, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), S_REGISTER,                      SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x47,0x10 * N + 0x01 * M);
			
			//set *(rN+offs) to *srM
			//~ 0x0048, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), S_REGISTER | POINTER,            SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x48,0x10 * N + 0x01 * M);
			
			//set *(rN+offs) to *(srM+offs)
			//~ 0x0049, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), S_REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(2)),
			CODE_SEQUENCE_FOR(0x49,0x10 * N + 0x01 * M);
		}
	}
	
	
	for (N = 0; N < N_SPECIAL_REGS; N++) {
		//set srN to constant
		//~ 0x004a, S_REGISTER, FIRST_NIBBLE, CONSTANT,           FOLLOWING(1)),
		CODE_SEQUENCE_FOR(0x4a,0x10 * N);
		
		
		
		//set srN to *constant
		//~ 0x014a, S_REGISTER, FIRST_NIBBLE, CONSTANT | POINTER, FOLLOWING(1)),
		CODE_SEQUENCE_FOR(0x4a,0x10 * N + 0x01);
		
		
		//set *srN to constant
		//~ 0x0051, S_REGISTER | POINTER, FIRST_NIBBLE, CONSTANT,           FOLLOWING(1)),
		CODE_SEQUENCE_FOR(0x51,0x10 * N);
		
		
		
		//set *srN to *constant
		//~ 0x0151, S_REGISTER | POINTER, FIRST_NIBBLE, CONSTANT | POINTER, FOLLOWING(1)),
		CODE_SEQUENCE_FOR(0x51,0x10 * N + 0x01);
		
		
		
		
		//set *(srN+offs) to constant
		//~ 0x0058, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), CONSTANT,           FOLLOWING(2)),
		CODE_SEQUENCE_FOR(0x58,0x10 * N);
		
		pkptrout
		datatooffs
		incrementpk
        pkptrout
		echodatalong
		specialtoaddr(N)
		writeRAMo
		incrementpk
        done
        
		
		
		//set *(srN+offs) to *constant
		//~ 0x0059, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), CONSTANT | POINTER, FOLLOWING(2)),
		CODE_SEQUENCE_FOR(0x59,0x10 * N);
		
		pkptrout
        datatooffs
        incrementpk
        pkptrout
        datatotmpa
        tmpaptodata
        echodatalong
        specialtoaddr(N)
        writeRAMo
        incrementpk
        done
		
		for (M = 0; M < N_REGISTERS; M++) {
			//set srN to rM
			//~ 0x004b, S_REGISTER, FIRST_NIBBLE, REGISTER,                      SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x4b,0x10 * N + 0x01 * M);
			
			
			
			//set srN to *rM
			//~ 0x004c, S_REGISTER, FIRST_NIBBLE, REGISTER | POINTER,            SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x4c,0x10 * N + 0x01 * M);
			
			
			
			//set srN to *(rM+offs)
			//~ 0x004d, S_REGISTER, FIRST_NIBBLE, REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
			CODE_SEQUENCE_FOR(0x4d,0x10 * N + 0x01 * M);
			
			
			
			//set *srN to rM
			//~ 0x0052, S_REGISTER | POINTER, FIRST_NIBBLE, REGISTER,           SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x52,0x10 * N + 0x01 * M);
			
			
			//set *srN to *rM
			//~ 0x0053, S_REGISTER | POINTER, FIRST_NIBBLE, REGISTER | POINTER, SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x53,0x10 * N + 0x01 * M);
			
			
			//set *srN to *(rM+offs)
			//~ 0x0054, S_REGISTER | POINTER, FIRST_NIBBLE, REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
			CODE_SEQUENCE_FOR(0x54,0x10 * N + 0x01 * M);
			
			
			//set *(srN+offs) to rM
			//~ 0x005a, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), REGISTER,                      SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x5a,0x10 * N + 0x01 * M);
			
			
			pkptrout
            datatooffs
            specialtoaddr(N)
            regtodata(M)
            writeRAMo
            incrementpk
            done
			
			
			//set *(srN+offs) to *rM
			//~ 0x005b, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), REGISTER | POINTER,            SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x5b,0x10 * N + 0x01 * M);
			
			
			//set *(srN+offs) to *(rM+offs)
			//~ 0x005c, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(2)),
			CODE_SEQUENCE_FOR(0x5c,0x10 * N + 0x01 * M);
			
		}
		
		for (M = 0; M < N_SPECIAL_REGS; M++) {
			//set srN to srM
			//~ 0x004e, S_REGISTER, FIRST_NIBBLE, S_REGISTER,                      SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x4e,0x10 * N + 0x01 * M);
			
			
			//set *srN to *srM
			//~ 0x004f, S_REGISTER, FIRST_NIBBLE, S_REGISTER | POINTER,            SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x4f,0x10 * N + 0x01 * M);
			
			
			//set *srN to *(srM+offs)
			//~ 0x0050, S_REGISTER, FIRST_NIBBLE, S_REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
			CODE_SEQUENCE_FOR(0x50,0x10 * N + 0x01 * M);
			
			//set *srN to srM
			//~ 0x0055, S_REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER,           SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x55,0x10 * N + 0x01 * M);
			
			
			//set *srN to *srM
			//~ 0x0056, S_REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER | POINTER, SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x56,0x10 * N + 0x01 * M);
			
			
			//set *srN to *(srM+offs)
			//~ 0x0057, S_REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
			CODE_SEQUENCE_FOR(0x57,0x10 * N + 0x01 * M);
			
			
			//set *(srN+offs) to srM
			//~ 0x005d, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), S_REGISTER,                      SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x5d,0x10 * N + 0x01 * M);
			
			//set *(srN+offs) to *srM
			//~ 0x005e, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), S_REGISTER | POINTER,            SECOND_NIBBLE),
			CODE_SEQUENCE_FOR(0x5e,0x10 * N + 0x01 * M);
			
			
			//set *(srN+offs) to *(srM+offs)
			//~ 0x005f, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), S_REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(2)),
			CODE_SEQUENCE_FOR(0x5f,0x10 * N + 0x01 * M);
		}
	}
	
	// hardwear :)
	for (N = 0; N<N_REGISTERS; N++) {
		// write bitport constant to rN
		CODE_SEQUENCE_FOR(0xa0,0x10 * N);
		
		pkptrout
		echodataaddr
		readbitport
		datatoreg(N)
		incrementpk
		done
		
		// write rN to bitport constant
		CODE_SEQUENCE_FOR(0xa1,0x10 * N);
		
		pkptrout
		echodataaddr
		regtodata(N)
		writebitport
		incrementpk
		done
		
		// write bitport constant to rN (bitwise addressing)
		CODE_SEQUENCE_FOR(0xa2,0x10 * N);
		
		pkptrout
		echodataaddr
		readbitportb
		datatoreg(N)
		incrementpk
		done
		
		// write rN to bitport constant (bitwise addressing)
		CODE_SEQUENCE_FOR(0xa3,0x10 * N);
		
		pkptrout
		echodataaddr
		regtodata(N)
		writebitportb
		incrementpk
		done
		
		// read wideport constant to rN
		CODE_SEQUENCE_FOR(0xa4,0x10 * N);
		
		pkptrout
		echodataaddr
		readwideport
		datatoreg(N)
		incrementpk
		done
		
		// write rN to wideport constant
		CODE_SEQUENCE_FOR(0xa5,0x10 * N);
		
		pkptrout
		echodataaddr
		regtodata(N)
		writewideport
		incrementpk
		done
		
		for (M = 0; M<N_REGISTERS; M++) {
			// write bitport rM to constant rN
			CODE_SEQUENCE_FOR(0xa6,0x10 * N);
		
			regtoaddr(M)
			readbitport
			datatoreg(N)
			done
			
			// write rN to bitport rM
			CODE_SEQUENCE_FOR(0xa7,0x10 * N);
			
			regtoaddr(M)
			regtodata(N)
			writebitport
			done
			
			// write bitport rM to rN (bitwise addressing)
			CODE_SEQUENCE_FOR(0xa8,0x10 * N);
			
			regtoaddr(M)
			readbitportb
			datatoreg(N)
			done
			
			// write rN to bitport rM (bitwise addressing)
			CODE_SEQUENCE_FOR(0xa9,0x10 * N);
			
			regtoaddr(M)
			regtodata(N)
			writebitportb
			done
			
			// read rM constant to rN
			CODE_SEQUENCE_FOR(0xaa,0x10 * N);
			
			regtoaddr(M)
			readwideport
			datatoreg(N)
			done
			
			// write rN to wideport rM
			CODE_SEQUENCE_FOR(0xab,0x10 * N);
			
			regtoaddr(M)
			regtodata(N)
			writewideport
			done
		}
	}
	
	
	
	fwrite(mr_data, 1, NWORDS, mainROM);
    fclose(mainROM);
	fwrite(dr_data, 1, NWORDS, decoderROM);
    fclose(decoderROM);
    
    free(mr_data);
    free(dr_data);
    
    return EXIT_SUCCESS;
}
