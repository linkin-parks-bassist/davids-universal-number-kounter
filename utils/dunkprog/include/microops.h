#ifndef MICRO_OPS_H
#define MICRO_OPS_H

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
#define tmpatosreg 		0x2007

#define tmpbtodata 		0x3002
#define tmpbtoaddr 		0x3003
#define tmpbptodata 	0x3004
#define tmpbptodata_o 	0x3005
#define datatotmpb 		0x3006
#define tmpbtosreg 		0x3006

#define tmpctodata 		0x4002
#define tmpctoaddr 		0x4003
#define tmpcptodata 	0x4004
#define tmpcptodata_o 	0x4005
#define datatotmpc 		0x4006
#define tmpctosreg 		0x4010

#define offstodata 		0x5002
#define offstoaddr 		0x5003
#define offsptodata 	0x5004
#define offsptodata_o 	0x5005
#define datatooffs 		0x5006

#define incrementpk 	0x0010
#define pkptroutinc		0x0011

#define decrementpk 	0x0012

#define tmpatopk 		0x0013
#define tmptopk 		0x0014

#define tmptopkifz 		0x0015
#define tmptopkifnz 	0x0016

#define tmptopkifn 		0x0017
#define tmptopkifnn 	0x0018

#define tmptopkifp 		0x0019
#define tmptopkifnp 	0x001a

#define it_to_pk		0x001b


#define incrementsp 	0x1010
#define decrementsp 	0x1011
#define predecsptoaddr 	0x1012


#define regtodata(X) 	(0x0020 + 0x1000 * X) | MIX_IN
#define regtoaddr(X) 	(0x0021 + 0x0100 * X) | MIX_IN
#define regptodata(X) 	(0x0022 + 0x0100 * X) | MIX_IN
#define regptodata_o(X) (0x0023 + 0x0100 * X) | MIX_IN
#define datatoreg(X) 	(0x0024 + 0x1000 * X) | MIX_IN

#define writeRAM 		0x0025
#define writeRAM_o 		0x0026

#define readRAM 		0x0027
#define readRAM_o 		0x0028

#define datatoALUa		0x002f
#define datatoALUb		0x102f

#define pinmodein(N)  	(0x0030 + 0x1000 * N) | MIX_IN
#define pinmodeout(N) 	(0x0031 + 0x1000 * N) | MIX_IN
#define setpinlow(N)  	(0x0032 + 0x1000 * N) | MIX_IN
#define setpinhigh(N) 	(0x0033 + 0x1000 * N) | MIX_IN
#define pintodata(N) 	(0x0034 + 0x1000 * N) | MIX_IN
#define datatopin(N)  	(0x0035 + 0x1000 * N) | MIX_IN

#define ALUop(X, Y, o) 	(0x0040 + 0x1000 * X + 0x0100 * Y + 0x0001 * o) | MIX_IN

#define ALUadd(X, Y) 	ALUop(0x0)
#define ALUsub(X, Y) 	ALUop(0x1)
#define ALUmul(X, Y) 	ALUop(0x2)
#define ALUinc(X, Y) 	ALUop(0x3)
#define ALUdec(X, Y) 	ALUop(0x4)
#define ALUucmp(X, Y) 	ALUop(0x5)
#define ALUcmp(X, Y) 	ALUop(0x6)
#define ALUsgn(X, Y) 	ALUop(0x7)
#define ALUand(X, Y) 	ALUop(0x8)
#define ALUor(X, Y)  	ALUop(0x9)
#define ALUxor(X, Y)    ALUop(0xa)
#define ALUnot(X, Y)    ALUop(0xb)
#define ALUlshift(X, Y) ALUop(0xc)
#define ALUrshift(X, Y) ALUop(0xd)
#define ALUshift(X, Y)  ALUop(0xe)

#define holddata 		0x0050
#define holdaddr	 	0x0051

#define holdaddrdata 	0x0052
#define holddataaddr 	0x0053

#define stacking  		0x0060
#define unstacking 		0x0061

#define done  0xfffe
#define reset 0xffff

#endif
