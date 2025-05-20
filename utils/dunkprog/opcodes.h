#ifndef DUNKPROG_INSTR_H
#define DUNKPROG_INSTR_H

#define CHILL 					0x00
#define GOTO 					0x01
#define GOTO_IF_ZERO 			0x02
#define GOTO_IF_NONZERO 		0x03
#define GOTO_IF_NEGATIVE 		0x04
#define GOTO_IF_NONNEGATIVE 	0x05
#define GOTO_IF_POSITIVE 		0x06
#define GOTO_IF_NONPOSITIVE 	0x07
#define GOTO_IF_EQUAL 			0x08 // unimplemented
#define GOTO_IF_UNEQUAL 		0x09 // unimplemented
#define GOTO_IF_LESS 			0x0a // unimplemented
#define GOTO_IF_GREATER 		0x0b // unimplemented
#define GOTO_IF_LEQ 			0x0c // unimplemented
#define GOTO_IF_GEQ 			0x0d // unimplemented
#define GOTO_IF_EQUAL 			0x0e // unimplemented
#define GOTO_IF_UNEQUAL 		0x0f // unimplemented
#define GOTO_IF_LESS 			0x10 // unimplemented
#define GOTO_IF_GREATER 		0x11 // unimplemented
#define GOTO_IF_LEQ 			0x12 // unimplemented
#define GOTO_IF_GEQ 			0x13 // unimplemented
#define SWAP_R_R 				0x14
#define SET_PC_C 				0x15
#define SET_PC_PC 				0x16
#define SET_PC_R 				0x17
#define SET_PC_PR 				0x18
#define SET_PC_PRO 				0x19
#define SET_PC_SR 				0x1a
#define SET_PC_PSR 				0x1b
#define SET_PC_PSRO 			0x1c
#define SET_R_C 				0x1d
#define SET_R_PC 				0x1e
#define SET_R_R 				0x1f
#define SET_R_PR 				0x20
#define SET_R_PRO 				0x21
#define SET_R_SR 				0x22
#define SET_R_PSR 				0x23
#define SET_R_PSRO 				0x24
#define SET_PR_C 				0x25
#define SET_PR_PC 				0x26
#define SET_PR_R 				0x27
#define SET_PR_PR 				0x28
#define SET_PR_PRO 				0x29
#define SET_PR_SR 				0x2a
#define SET_PR_PSR 				0x2b
#define SET_PR_PSRO 			0x2c
#define SET_PRO_C 				0x2d
#define SET_PRO_PC 				0x2e
#define SET_PRO_R 				0x2f
#define SET_PRO_PR 				0x30
#define SET_PRO_PRO 			0x31
#define SET_PRO_SR 				0x32
#define SET_PRO_PSR 			0x33
#define SET_PRO_PSRO 			0x34
#define SET_SR_C 				0x35
#define SET_SR_PC 				0x36
#define SET_SR_R 				0x37
#define SET_SR_PR 				0x38
#define SET_SR_PRO 				0x39
#define SET_SR_SR 				0x3a
#define SET_SR_PSR 				0x3b
#define SET_SR_PSRO 			0x3c
#define SET_PSR_C 				0x3d
#define SET_PSR_PC 				0x3e
#define SET_PSR_R 				0x3f
#define SET_PSR_PR 				0x40
#define SET_PSR_PRO 			0x41
#define SET_PSR_SR 				0x42
#define SET_PSR_PSR 			0x43
#define SET_PSR_PSRO 			0x44
#define SET_PSRO_C 				0x45
#define SET_PSRO_PC 			0x46
#define SET_PSRO_R 				0x47
#define SET_PSRO_PR 			0x48
#define SET_PSRO_PRO 			0x49
#define SET_PSRO_SR 			0x4a
#define SET_PSRO_PSR 			0x4b
#define SET_PSRO_PSRO 			0x4c
#define ADD_R_C 				0x4d // unimplemented
#define SUB_R_C 				0x4e // unimplemented
#define MUL_R_C 				0x4f // unimplemented
#define INC_R 					0x50 // unimplemented
#define DEC_R 					0x51 // unimplemented
#define NEG_R 					0x52 // unimplemented
#define UCMP_R_C 				0x53 // unimplemented
#define CMP_R_C 				0x54 // unimplemented
#define SGN_R 					0x55 // unimplemented
#define AND_R_C 				0x56 // unimplemented
#define OR_R_C 					0x57 // unimplemented
#define XOR_R_C 				0x58 // unimplemented
#define LSHIFT_R 				0x5a // unimplemented
#define SHIFT_R_C 				0x5b // unimplemented
#define RSHIFT_R 				0x5c // unimplemented
#define ADD_R_R_C 				0x5d // unimplemented
#define SUB_R_R_C 				0x5e // unimplemented
#define MUL_R_R_C 				0x5f // unimplemented
#define INC_R_C 				0x60 // unimplemented
#define DEC_R_C 				0x61 // unimplemented
#define NEG_R_C 				0x62 // unimplemented
#define UCMP_R_R_C 				0x63 // unimplemented
#define CMP_R_R_C 				0x64 // unimplemented
#define SGN_R_C 				0x65 // unimplemented
#define AND_R_R_C 				0x66 // unimplemented
#define OR_R_R_C 				0x67 // unimplemented
#define XOR_R_R_C 				0x68 // unimplemented
#define NOT_R_R 				0x69 // unimplemented
#define LSHIFT_R_C 				0x6a // unimplemented
#define SHIFT_R_R_C 			0x6b // unimplemented
#define RSHIFT_R_C 				0x6c // unimplemented
#define ADD_R_R 				0x6d // unimplemented
#define SUB_R_R 				0x6e // unimplemented
#define MUL_R_R 				0x6f // unimplemented
#define INC_R 					0x70 // unimplemented
#define DEC_R 					0x71 // unimplemented
#define NEG_R 					0x72 // unimplemented
#define UCMP_R_R 				0x73 // unimplemented
#define CMP_R_R 				0x74 // unimplemented
#define SGN_R 					0x75 // unimplemented
#define AND_R_R 				0x76 // unimplemented
#define OR_R_R 					0x77 // unimplemented
#define XOR_R_R 				0x78 // unimplemented
#define LSHIFT_R 				0x7a // unimplemented
#define SHIFT_R_R 				0x7b // unimplemented
#define RSHIFT_R 				0x7c // unimplemented
#define ADD_R_R_R 				0x7d // unimplemented
#define SUB_R_R_R 				0x7e // unimplemented
#define MUL_R_R_R 				0x7f // unimplemented
#define INC_R_R 				0x80 // unimplemented
#define DEC_R_R 				0x81 // unimplemented
#define NEG_R_R 				0x82 // unimplemented
#define UCMP_R_R_R 				0x83 // unimplemented
#define CMP_R_R_R 				0x84 // unimplemented
#define SGN_R_R 				0x85 // unimplemented
#define AND_R_R_R 				0x86 // unimplemented
#define OR_R_R_R 				0x87 // unimplemented
#define XOR_R_R_R 				0x88 // unimplemented
#define NOT_R_R 				0x89 // unimplemented
#define LSHIFT_R_R 				0x8a // unimplemented
#define SHIFT_R_R_R 			0x8b // unimplemented
#define RSHIFT_R_R 				0x8c // unimplemented
#define PUSH_C 					0x8e
#define PUSH_PC 				0x8f
#define PUSH_R 					0x90
#define PUSH_PR 				0x91
#define PUSH_PRO 				0x92
#define PUSH_SR 				0x93
#define PUSH_PSR 				0x94
#define PUSH_PSRO 				0x95
#define POP_PC 					0x97
#define POP_R 					0x98
#define POP_PR 					0x99
#define POP_PRO 				0x9a
#define POP_SR 					0x9b
#define POP_PSR 				0x9c
#define POP_PSRO 				0x9d
#define CALL 					0x9e
#define RETURN 					0x9f
#define PINMODE_INPUT_C 		0xa0
#define PINMODE_OUTPUT_C 		0xa1
#define PINMODE_INPUT_R 		0xa2
#define PINMODE_OUTPUT_R 		0xa3
#define SET_PIN_LOW_C 			0xa4
#define SET_PIN_HIGH_C 			0xa5
#define SET_PIN_LOW_R 			0xa6
#define SET_PIN_HIGH_R 			0xa7
#define READ_PIN_C_R 			0xa8
#define WRITE_PIN_C_R 			0xa9
#define READ_PIN_R_R 			0xaa
#define WRITE_PIN_R_R 			0xab
#define HALT_AND_CATCH_FIRE 	0xff

#endif
