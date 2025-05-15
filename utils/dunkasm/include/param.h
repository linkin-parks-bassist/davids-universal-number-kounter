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
#define LABEL_P		17
#define INVALID 	0xffff

#define POINTER 	4
#define W_OFFSET	8

#define FIRST_NIBBLE 		0b10000000000000
#define SECOND_NIBBLE 		0b01000000000000
#define THIRD_NIBBLE 		0b00100000000000
#define FOURTH_NIBBLE 		0b00010000000000

#define FOLLOWING_FN(N) 	0b00001000000000 + N
#define FOLLOWING_SN(N) 	0b00000100000000 + N
#define FOLLOWING_TN(N) 	0b00000010000000 + N
#define FOLLOWING_LN(N) 	0b00000001000000 + N
#define FOLLOWING(N) 		0b00000000100000 + N
#define OFFSET_FOLLOWING(N) 0b00000000010000 + N

#define FOLLOWING_MASK 		0b00001111111111
#define FOLLOWING_POS_MASK  0b00000000001111

#define MAX_PARAMS 3

typedef struct
{
	int type;
	uint16_t value;
	uint16_t offset;
} parameter;

parameter parse_parameter(dasm_context *cxt, const char* input);

int handle_string_parameter(dasm_context *cxt, dasm_file *file, parameter *param, const char *str, unsigned int line_number);
int handle_label_parameter(dasm_context *cxt, dasm_file *file, parameter *param, const char *str, unsigned int line_number);

#endif
