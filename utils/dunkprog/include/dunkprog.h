#include <stdint.h>

#ifndef DUNKPROG_H
#define DUNKPROG_H

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

#endif
