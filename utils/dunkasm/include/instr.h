#include <stdint.h>

#ifndef DUNK_INSTR_H
#define DUNK_INSTR_H

typedef struct
{
	const char* name;
	uint16_t code;
	int n_args;
	int arg_types[4];
	int arg_positions[4];
} dunk_instr;


#define NULLARY_COMMAND(name, code)                                            \
	{ name, code, 0, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }
#define UNARY___COMMAND(name, code, a1_t, a1_p)                                \
	{ name, code, 1, { a1_t, 0, 0, 0 }, { a1_p, 0, 0, 0 } }
#define BINARY__COMMAND(name, code, a1_t, a1_p, a2_t, a2_p)                    \
	{ name, code, 2, { a1_t, a2_t, 0, 0 }, { a1_p, a2_p, 0, 0 } }
#define BINARY__COMMAND(name, code, a1_t, a1_p, a2_t, a2_p)                    \
	{ name, code, 2, { a1_t, a2_t, 0, 0 }, { a1_p, a2_p, 0, 0 } }
#define TERNARY_COMMAND(name, code, a1_t, a1_p, a2_t, a2_p, a3_t, a3_p)        \
	{ name, code, 2, { a1_t, a2_t, a3_t, 0 }, { a1_p, a2_p, a3_p, 0 } }

#define N_INSTR 107

// These instructions are handled separately, using macros

#define FUNCTION_CALL 		0x80

#define PLAIN_GOTO 			0x01
#define GOTO_C_ZERO 		0x02
#define GOTO_C_NONZERO 		0x03
#define GOTO_C_NEGATIVE 	0x04
#define GOTO_C_NONNEGATIVE 	0x05
#define GOTO_C_POSITIVE 	0x06
#define GOTO_C_NONPOSITIVE 	0x07

extern dunk_instr dunk_instrs[N_INSTR];

#endif
