#include <stdint.h>

#ifndef DUNK_INSTR_H
#define DUNK_INSTR_H

#define MAX_N_PARAMS 9

typedef struct
{
	const char* name;
	uint16_t code;
	int n_args;
	int arg_types[9];
	int arg_positions[9];
} dunk_instr;


#define NULLARY_COMMAND(name, code)                                            						\
	(dunk_instr){ name, code, 0, {0,       0,    0, 0, 0, 0, 0, 0, 0}, {0,       0,    0, 0, 0, 0, 0, 0, 0} }


#define UNARY___COMMAND(name, code, a1_t, a1_p)                               			 			\
	(dunk_instr){ name, code, 1, {a1_t,    0,    0, 0, 0, 0, 0, 0, 0}, {a1_p,    0,    0, 0, 0, 0, 0, 0, 0} }


#define BINARY__COMMAND(name, code, a1_t, a1_p, a2_t, a2_p)                   			 			\
	(dunk_instr){ name, code, 2, {a1_t, a2_t,    0, 0, 0, 0, 0, 0, 0}, {a1_p, a2_p,    0, 0, 0, 0, 0, 0, 0} }


#define TERNARY_COMMAND(name, code, a1_t, a1_p, a2_t, a2_p, a3_t, a3_p)       					 	\
	(dunk_instr){ name, code, 2, {a1_t, a2_t, a3_t, 0, 0, 0, 0, 0, 0}, {a1_p, a2_p, a3_p, 0, 0, 0, 0, 0, 0} }


#define CALL_COMMAND(name, code, n) \
	(dunk_instr){ name, code, n + 1, {CONSTANT, 	REGISTER, 		 REGISTER, 		  REGISTER,		   REGISTER, 		REGISTER,	 	 REGISTER, 		  REGISTER,		   REGISTER}, \
								 {FOLLOWING(1), FOLLOWING_FN(2), FOLLOWING_SN(2), FOLLOWING_TN(2), FOLLOWING_LN(2), FOLLOWING_FN(3), FOLLOWING_SN(3), FOLLOWING_TN(3), FOLLOWING_LN(3)} }

#define RET_COMMAND(name, code, n) \
	(dunk_instr){ name, code, n, {REGISTER, 		 REGISTER, 		  REGISTER,		   REGISTER, 		REGISTER,	 	 REGISTER, 		  REGISTER,		   REGISTER, 0}, \
								 {FOLLOWING_FN(1), FOLLOWING_SN(1), FOLLOWING_TN(1), FOLLOWING_LN(1), FOLLOWING_FN(2), FOLLOWING_SN(2), FOLLOWING_TN(2), FOLLOWING_LN(2), 0} }

#define N_INSTR 123

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
