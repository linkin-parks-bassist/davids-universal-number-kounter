#include <stdint.h>

#ifndef DUNK_INSTR_H
#define DUNK_INSTR_H

#define MAX_N_PARAMS 9

typedef struct
{
	const char* name;
	uint16_t code;
	int n_parameters;
	int parameter_types[MAX_N_PARAMS];
	int parameter_positions[MAX_N_PARAMS];
} dunk_instr;


#define NULLARY_INSTR(name, code)																	\
	(dunk_instr){ name, code, 0, {0,	   0,	0, 0, 0, 0, 0, 0, 0}, {0,	   0,	0, 0, 0, 0, 0, 0, 0} }


#define UNARY___INSTR(name, code, a1_t, a1_p)							   			 			\
	(dunk_instr){ name, code, 1, {a1_t,	0,	0, 0, 0, 0, 0, 0, 0}, {a1_p,	0,	0, 0, 0, 0, 0, 0, 0} }


#define BINARY__INSTR(name, code, a1_t, a1_p, a2_t, a2_p)				   			 			\
	(dunk_instr){ name, code, 2, {a1_t, a2_t,	0, 0, 0, 0, 0, 0, 0}, {a1_p, a2_p,	0, 0, 0, 0, 0, 0, 0} }


#define TERNARY_INSTR(name, code, a1_t, a1_p, a2_t, a2_p, a3_t, a3_p)	   					 	\
	(dunk_instr){ name, code, 3, {a1_t, a2_t, a3_t, 0, 0, 0, 0, 0, 0}, {a1_p, a2_p, a3_p, 0, 0, 0, 0, 0, 0} }

#define N_INSTR 170

extern const dunk_instr dunk_instrs[N_INSTR];

#endif
