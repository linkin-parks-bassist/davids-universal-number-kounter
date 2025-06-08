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

#define tmpctooffs		0x5010

#define incrementpk(x) 	0x0010 + (0x0100 * x)
#define pkptroutinc(x)	0x0011 + (0x0100 * x)

#define decrementpk(x) 	0x0012 + (0x0100 * x)

#define tmpatopk 		0x0013
#define tmptopk 		0x0014

#define tmptopkifz 		0x0015
#define tmptopkifnz 	0x0016

#define tmptopkifn 		0x0017
#define tmptopkifnn 	0x0018

#define tmptopkifp 		0x0019
#define tmptopkifnp 	0x001a

#define it_to_pk		0x001b

#define incrementsp(x) 	0x1010 + (0x0100 * x)
#define decrementsp(x) 	0x1011 + (0x0100 * x)
#define predecsptoaddr 	0x1012
#define postincspptrout 0x1013


#define regtodata(X) 	(0x0020 + 0x1000 * X) | MIX_IN
#define regtoaddr(X) 	(0x0021 + 0x0100 * X) | MIX_IN
#define regptodata(X) 	(0x0022 + 0x0100 * X) | MIX_IN
#define regptodata_o(X) (0x0023 + 0x0100 * X) | MIX_IN
#define datatoreg(X) 	(0x0024 + 0x1000 * X) | MIX_IN

#define writeRAM 		0x0025
#define writeRAM_o 		0x0026

#define readRAM 		0x0027
#define readRAM_o 		0x0028

#define tmpcasoffs		0x0029
#define dataasoffs		0x002a
#define dataasaddr		0x002b

#define datatoALUa		0x002f
#define datatoALUb		0x102f

#define pinmodein(N)  	(0x0030 + 0x1000 * N) | MIX_IN
#define pinmodeout(N) 	(0x0031 + 0x1000 * N) | MIX_IN
#define setpinlow(N)  	(0x0032 + 0x1000 * N) | MIX_IN
#define setpinhigh(N) 	(0x0033 + 0x1000 * N) | MIX_IN
#define pintodata(N) 	(0x0034 + 0x1000 * N) | MIX_IN
#define datatopin(N)  	(0x0035 + 0x1000 * N) | MIX_IN

#define ALU_ADD 	0x0
#define ALU_SUB 	0x1
#define ALU_MUL 	0x2
#define ALU_DIV 	0x3
#define ALU_REM 	0x4
#define ALU_INC 	0x5
#define ALU_DEC 	0x6
#define ALU_UCMP 	0x7
#define ALU_CMP 	0x8
#define ALU_AND 	0x9
#define ALU_OR 		0xa
#define ALU_XOR 	0xb
#define ALU_NOT 	0xc
#define ALU_LSHIFT 	0xd
#define ALU_RSHIFT 	0xe
#define ALU_SHIFT  	0xf

#define ALUop(X, Y, o) 	(((o < 16) ? 0x0040 : 0x0050) + 0x1000 * X + 0x0100 * Y + (0b1111 & o)) | MIX_IN

#define ALUadd(X, Y) 	ALUop(X, Y, ALU_ADD)
#define ALUsub(X, Y) 	ALUop(X, Y, ALU_SUB)
#define ALUmul(X, Y) 	ALUop(X, Y, ALU_MUL)
#define ALUdiv(X, Y) 	ALUop(X, Y, ALU_DIV)
#define ALUrem(X, Y) 	ALUop(X, Y, ALU_REM)
#define ALUinc(X, Y) 	ALUop(X, Y, ALU_INC)
#define ALUdec(X, Y) 	ALUop(X, Y, ALU_DEC)
#define ALUucmp(X, Y) 	ALUop(X, Y, ALU_UCMP)
#define ALUcmp(X, Y) 	ALUop(X, Y, ALU_CMP)
#define ALUsgn(X, Y) 	ALUop(X, Y, ALU_SGN)
#define ALUand(X, Y) 	ALUop(X, Y, ALU_AND)
#define ALUor(X, Y)  	ALUop(X, Y, ALU_OR)
#define ALUxor(X, Y)    ALUop(X, Y, ALU_XOR)
#define ALUnot(X, Y)    ALUop(X, Y, ALU_NOT)
#define ALUlshift(X, Y) ALUop(X, Y, ALU_LSHIFT)
#define ALUrshift(X, Y) ALUop(X, Y, ALU_RSHIFT)
#define ALUshift(X, Y)  ALUop(X, Y, ALU_SHIFT)

#define holddata 		0x0060
#define holdaddr	 	0x0061

#define holdaddrdata 	0x0062
#define holddataaddr 	0x0063

#define stacking  		0x0064
#define unstacking 		0x0065
#define preunstacking 	0x0066
#define refreshctrlbus 	0x0067

#define sctaddrout		0x007a

#define hold  0x00fd

#define done  0xfffe
#define reset 0xffff

#endif
