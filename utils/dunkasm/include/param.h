#ifndef DUNK_PARAM_H
#define DUNK_PARAM_H

/** Hardcoded array sizes so I don't have to bother with dynamic memory **/
#define MAX_TOKENS 			100
#define MAX_N_ALIASES 		512
#define MAX_BIN_SIZE 		2 * 64 * 1024

/** Macros for classifying parameter types **/
#define CONSTANT 	1
#define REGISTER 	2
#define S_REGISTER 	3
#define STRING_P	16
#define INVALID 	0xffff

#define POINTER 	4
#define W_OFFSET	8

#define FIRST_NIBBLE 		0b10000000000
#define SECOND_NIBBLE 		0b01000000000
#define FOLLOWING(N) 		0b00100000000 + N
#define OFFSET_FOLLOWING(N) 0b00010000000 + N
#define FOLLOWING_MASK 		0b00111111111

#define MAX_PARAMS 3

typedef struct
{
	int type;
	uint16_t value;
	uint16_t offset;
} parameter;

parameter parse_parameter(dasm_context *cxt, const char* input);

#endif
