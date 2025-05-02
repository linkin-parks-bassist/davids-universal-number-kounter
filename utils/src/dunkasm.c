#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Hardcoded array sizes so I don't have to bother with dynamic memory **/
#define MAX_LINE_LENGTH 	1024
#define MAX_LABEL_LENGTH 	64
#define MAX_LABELS			1024
#define MAX_LABEL_REFS		1024
#define MAX_TOKENS 			100
#define MAX_N_ALIASES 		512
#define BINSIZE 			2 * 64 * 1024

#define numer number //I make this type so often I'm tired of fixing it

/** Macros for classifying parameter types **/
#define CONSTANT 	1
#define REGISTER 	2
#define S_REGISTER 	3
#define INVALID 	0xffff

#define POINTER 	4
#define W_OFFSET	8

#define FIRST_NIBBLE 		0b10000000000
#define SECOND_NIBBLE 		0b01000000000
#define FOLLOWING(N) 		0b00100000000 + N
#define OFFSET_FOLLOWING(N) 0b00010000000 + N
#define FOLLOWING_MASK 		0b00111111111

#define N_ARGUMENT_ALIASES 9
#define N_DEFAULT_ALIASES 4 + N_ARGUMENT_ALIASES

#define MAX_PARAMS 3

/** A macro to make it easy to append machine code to output-in-progress **/
#define WRITEOUT(X)                                                            \
	do {                                                                         \
		printf("WRITEOUT 0x%04x at position %i=0x%x\n", X, current_position, current_position);           \
		word_ptr = (uint16_t*)(&binary_output[2 * current_position]);                  \
		*word_ptr = X;                                                             \
		current_position += 1;                                                     \
		word_ptr = (uint16_t*)(&binary_output[2 * current_position]);                  \
	} while (0);

/** TYPEDEFS **/

typedef struct
{
	const char* name;
	uint16_t code;
	int n_args;
	int arg_types[4];
	int arg_positions[4];
} dunk_instr;

typedef struct
{
	char* replacee;
	char* replacer;
} alias;

typedef struct
{
	const char *fname;
	char label[MAX_LABEL_LENGTH];
	uint16_t position;
	int line_number;
} label_ref;

typedef struct
{
	int line_number;
	char** tokens;
	int token_count;
} line_data_str;

typedef struct line_data_node
{
	line_data_str* data;
	struct line_data_node* next;
} line_data_node;

/** GLOBAL VARIABLES **/

uint8_t* binary_output;
int current_position = 0;

int n_labels = 0;
int n_label_refs = 0;

uint16_t* word_ptr;

char labels[MAX_LABELS][MAX_LABEL_LENGTH];
uint16_t label_positions[MAX_LABELS];
label_ref label_refs[MAX_LABEL_REFS];

int num_aliases;
alias aliases[MAX_N_ALIASES];

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

#define N_INSTR 100

dunk_instr dunk_instrs[N_INSTR] = {
	NULLARY_COMMAND("chill", 0x0000),

	//**ALU**//
	BINARY__COMMAND("add", 		0x0010, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("sub", 		0x0011, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),	
	BINARY__COMMAND("subtract", 0x0011, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("mul", 		0x0012, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("multiply", 0x0012, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),

	UNARY___COMMAND("increment", 0x0013, REGISTER, FIRST_NIBBLE | SECOND_NIBBLE),
	UNARY___COMMAND("decrement", 0x0014, REGISTER, FIRST_NIBBLE | SECOND_NIBBLE),
	UNARY___COMMAND("negate", 	 0x0015, REGISTER, FIRST_NIBBLE | SECOND_NIBBLE),

	BINARY__COMMAND("compare_unsigned", 0x0016, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("compare",			0x0017, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),

	UNARY___COMMAND("sign", 0x0018, REGISTER, FIRST_NIBBLE | SECOND_NIBBLE),

	BINARY__COMMAND("and",  0x0019, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("or",	0x001a, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("xor",	0x001b, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("not",	0x001c, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),

	UNARY___COMMAND("lshift", 0x001d, REGISTER, FIRST_NIBBLE | SECOND_NIBBLE),
	BINARY__COMMAND("shift",  0x001d, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
	UNARY___COMMAND("rshift", 0x001e, REGISTER, FIRST_NIBBLE | SECOND_NIBBLE),

	//**stack stuff**//
	NULLARY_COMMAND("return", 0x0005),

	// pushes
	NULLARY_COMMAND("push", 0x0006),
	UNARY___COMMAND("push", 0x0106, CONSTANT, FOLLOWING(1)),
	UNARY___COMMAND("push", 0x0206, CONSTANT | POINTER, FOLLOWING(1)),
	UNARY___COMMAND("push", 0x0306, REGISTER, FIRST_NIBBLE),
	UNARY___COMMAND("push", 0x0406, REGISTER | POINTER, FIRST_NIBBLE),
	UNARY___COMMAND("push", 0x0506, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1)),
	UNARY___COMMAND("push", 0x0606, S_REGISTER, FIRST_NIBBLE),
	UNARY___COMMAND("push", 0x0706, S_REGISTER | POINTER, FIRST_NIBBLE),
	UNARY___COMMAND("push", 0x0806, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1)),

	// pops
	NULLARY_COMMAND("pop", 0x0007),
	UNARY___COMMAND("pop", 0x0107, CONSTANT, FOLLOWING(1)),
	UNARY___COMMAND("pop", 0x0207, CONSTANT | POINTER, FOLLOWING(1)),
	UNARY___COMMAND("pop", 0x0307, REGISTER, FIRST_NIBBLE),
	UNARY___COMMAND("pop", 0x0407, REGISTER | POINTER, FIRST_NIBBLE),
	UNARY___COMMAND("pop", 0x0507, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1)),
	UNARY___COMMAND("pop", 0x0707, S_REGISTER, FIRST_NIBBLE),
	UNARY___COMMAND("pop", 0x0707, S_REGISTER | POINTER, FIRST_NIBBLE),
	UNARY___COMMAND("pop", 0x0807, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1)),

	//**setting**//
	BINARY__COMMAND("set", 0x0030, CONSTANT | POINTER, FOLLOWING(1), CONSTANT, FOLLOWING(2)),
	BINARY__COMMAND("set", 0x0130, CONSTANT | POINTER, FOLLOWING(1), CONSTANT | POINTER, FOLLOWING(2)),

	BINARY__COMMAND("set", 0x0031, CONSTANT | POINTER, FOLLOWING(1), REGISTER, FIRST_NIBBLE),
	BINARY__COMMAND("set", 0x0131, CONSTANT | POINTER, FOLLOWING(1), REGISTER | POINTER, FIRST_NIBBLE),
	BINARY__COMMAND("set", 0x0231, CONSTANT | POINTER, FOLLOWING(1), REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | FOLLOWING(2)),

	BINARY__COMMAND("set", 0x0331, CONSTANT | POINTER, FOLLOWING(1), S_REGISTER, FIRST_NIBBLE),
	BINARY__COMMAND("set", 0x0431, CONSTANT | POINTER, FOLLOWING(1), S_REGISTER | POINTER, FIRST_NIBBLE),
	BINARY__COMMAND("set", 0x0531, CONSTANT | POINTER, FOLLOWING(1), S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | FOLLOWING(2)),

	BINARY__COMMAND("set", 0x0033, REGISTER, FIRST_NIBBLE, CONSTANT, FOLLOWING(1)),
	BINARY__COMMAND("set", 0x0034, REGISTER, FIRST_NIBBLE, CONSTANT | POINTER, FOLLOWING(1)),

	BINARY__COMMAND("set", 0x0035, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x0036, REGISTER, FIRST_NIBBLE, REGISTER | POINTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x0037, REGISTER, FIRST_NIBBLE, REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),

	BINARY__COMMAND("set", 0x0038, REGISTER, FIRST_NIBBLE, S_REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x0039, REGISTER, FIRST_NIBBLE, S_REGISTER | POINTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x003a, REGISTER, FIRST_NIBBLE, S_REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),

	BINARY__COMMAND("set", 0x003b, REGISTER | POINTER, FIRST_NIBBLE, CONSTANT, FOLLOWING(1)),
	BINARY__COMMAND("set", 0x003c, REGISTER | POINTER, FIRST_NIBBLE, CONSTANT | POINTER, FOLLOWING(1)),

	BINARY__COMMAND("set", 0x003d, REGISTER | POINTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x003e, REGISTER | POINTER, FIRST_NIBBLE, REGISTER | POINTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x003f, REGISTER | POINTER, FIRST_NIBBLE, REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),

	BINARY__COMMAND("set", 0x0040, REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x0041, REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER | POINTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x0042, REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),

	BINARY__COMMAND("set", 0x0043, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), CONSTANT, FOLLOWING(2)),
	BINARY__COMMAND("set", 0x0143, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), CONSTANT | POINTER, FOLLOWING(2)),

	BINARY__COMMAND("set", 0x0044, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x0045, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), REGISTER | POINTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x0046, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(2)),

	BINARY__COMMAND("set", 0x0047, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), S_REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x0048, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), S_REGISTER | POINTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x0049, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), S_REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(2)),

	BINARY__COMMAND("set", 0x004a, S_REGISTER, FIRST_NIBBLE, CONSTANT, FOLLOWING(1)),
	BINARY__COMMAND("set", 0x014a, S_REGISTER, FIRST_NIBBLE, CONSTANT | POINTER, FOLLOWING(1)),

	BINARY__COMMAND("set", 0x004b, S_REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x004c, S_REGISTER, FIRST_NIBBLE, REGISTER | POINTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x004d, S_REGISTER, FIRST_NIBBLE, REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),

	BINARY__COMMAND("set", 0x004e, S_REGISTER, FIRST_NIBBLE, S_REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x004f, S_REGISTER, FIRST_NIBBLE, S_REGISTER | POINTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x0050, S_REGISTER, FIRST_NIBBLE, S_REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),

	BINARY__COMMAND("set", 0x0051, S_REGISTER | POINTER, FIRST_NIBBLE, CONSTANT, FOLLOWING(1)),
	BINARY__COMMAND("set", 0x0151, S_REGISTER | POINTER, FIRST_NIBBLE, CONSTANT | POINTER, FOLLOWING(1)),

	BINARY__COMMAND("set", 0x0052, S_REGISTER | POINTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x0053, S_REGISTER | POINTER, FIRST_NIBBLE, REGISTER | POINTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x0054, S_REGISTER | POINTER, FIRST_NIBBLE, REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),

	BINARY__COMMAND("set", 0x0055, S_REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x0056, S_REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER | POINTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x0057, S_REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),

	BINARY__COMMAND("set", 0x0058, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), CONSTANT, FOLLOWING(2)),
	BINARY__COMMAND("set", 0x0059, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), CONSTANT | POINTER, FOLLOWING(2)),

	BINARY__COMMAND("set", 0x005a, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x005b, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), REGISTER | POINTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x005c, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(2)),

	BINARY__COMMAND("set", 0x005d, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), S_REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x005e, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), S_REGISTER | POINTER, SECOND_NIBBLE),
	BINARY__COMMAND("set", 0x005f, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), S_REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(2)),
	
	// I/O pins
	UNARY___COMMAND("pinmode_input",  0x00a0, CONSTANT, FIRST_NIBBLE),
	UNARY___COMMAND("pinmode_output", 0x00a1, CONSTANT, FIRST_NIBBLE),
	
	UNARY___COMMAND("set_pin_low",  0x00a2, CONSTANT, FIRST_NIBBLE),
	UNARY___COMMAND("set_pin_high", 0x00a3, CONSTANT, FIRST_NIBBLE),
	
	BINARY__COMMAND("read_pin",  0x00a3, CONSTANT, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("write_pin", 0x00a4, CONSTANT, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE)
};

void free_line_data(line_data_str* line_data)
{
	for (int i = 0; i < line_data->token_count; i++) {
		free(line_data->tokens[i]);
	}
	free(line_data->tokens);
	free(line_data);
}

void free_list(line_data_node* head)
{
	line_data_node* temp;
	while (head) {
		temp = head;
		head = head->next;
		free_line_data(temp->data);
		free(temp);
	}
}

// Function to tokenize a line into words
line_data_str *tokenize_line(char* line, int line_number)
{
	line_data_str *line_data = malloc(sizeof(line_data_str));
	if (!line_data) {
		perror("Memory allocation failed");
		exit(EXIT_FAILURE);
	}
	line_data->line_number = line_number;
	line_data->tokens = malloc(MAX_TOKENS * sizeof(char*));
	line_data->token_count = 0;

	char* token = strtok(line, " \t\n,;");
	while (token && line_data->token_count < MAX_TOKENS) {
		line_data->tokens[line_data->token_count] = strdup(token);
		line_data->token_count++;
		token = strtok(NULL, " \t\n,;");
	}
	return line_data;
}

// Function to check if the string is a number (constant)
int is_number(const char* str)
{
	if (str[0] == '-') {
		str++; // skip the minus sign
	}
	if (str[0] == '0') {
		if (str[1] == 'x' || str[1] == 'X') { // hexadecimal
			str += 2;
			while (*str) {
				if (!isxdigit(*str))
					return 0;
				str++;
			}
			return 1;
		} else if (str[1] == 'b' || str[1] == 'B') { // binary
			str += 2;
			while (*str) {
				if (*str != '0' && *str != '1')
					return 0;
				str++;
			}
			return 1;
		}
	}
	while (*str) {
		if (!isdigit(*str))
			return 0;
		str++;
	}
	return 1;
}

int is_dnumber(const char* str)
{
	if (str[0] == '-') {
		str = &str[1];
	}
	for (int i = 0; i < strlen(str); i++) {
		if (!(str[i] >= '0' && str[i] <= '9')) {
			return 0;
		}
	}
	return 1;
}

int is_hnumber(const char* str)
{
	for (int i = 0; i < strlen(str); i++) {
		if (!(str[i] >= '0' && str[i] <= '9') &&
				!(str[i] >= 'a' && str[i] <= 'f')) {
			return 0;
		}
	}
	return 1;
}

int is_bnumber(const char* str)
{
	for (int i = 0; i < strlen(str); i++) {
		if (!(str[i] >= '0' && str[i] <= '1')) {
			return 0;
		}
	}
	return 1;
}

long parse_number(const char* str)
{
	if (str[0] == '-')
		return -parse_number(&str[1]);

	if (str[0] == '0') {
		if (str[1] == 'x' && is_hnumber(&str[2])) {
			return strtol(&str[2], NULL, 16);
		} else if (str[1] == 'b' && is_bnumber(&str[2])) {
			return strtol(&str[2], NULL, 2);
		}
	} else if (is_dnumber(str)) {
		return strtol(str, NULL, 10);
	}
}

typedef struct
{
	int type;
	uint16_t value;
	uint16_t offset;
} parameter;

parameter parse_parameter(const char* input)
{
	parameter result = { 0, 0, 0 }; // Initialize result with default values
	char tmp;
	char* plus = NULL;
	const char* inside = NULL;
	const char* close_paren = NULL;

	size_t len, leng;     // = close_paren - inside;
	char* sub_exp;        // = strndup(inside, len);
	parameter sub_result; // = parse_string(sub_expr);

	printf("Parsing parameter ``%s\"\n", input);

	if (input[0] == '*') {
		printf("It's a pointer, ");
		if (input[1] == '(') {
			printf("parenthesised, with inner expression");
			inside = &input[2];
			close_paren = strchr(inside, ')');

			if (close_paren == NULL) {
				printf("... but not closing paren :(\n");
				result.type = INVALID;
				return result;
			}
			if (close_paren[1] != 0) {
				printf("... but does not end after its closing paren :(\n");

				result.type = INVALID;
				return result;
			}

			len = close_paren - inside;
			sub_exp = strndup(inside, len);

			printf(" \"%s\", ", sub_exp);

			plus = &sub_exp[strcspn(sub_exp, "+-")];

			if (plus != NULL) {
				if (plus[0] == '+') {
					sub_result = parse_parameter(&plus[1]);
				} else {
					sub_result = parse_parameter(plus);
				}

				if (sub_result.type != CONSTANT) {
					printf("... but it contains an offset which is not a constant :(\n");

					result.type = INVALID;
					return result;
				}
				if (sub_result.offset != 0) {
					printf("... but it contains a double-offset :(\n");

					result.type = INVALID;
					return result;
				}

				result.offset = sub_result.value;
				printf(" which has offset %i\n", result.offset);
				*plus = 0;
			}

		} else {
			printf("non-parenthesised, ");
			inside = &input[1];
			len = strlen(inside);
			sub_exp = strndup(inside, len);
		}

		printf("with sub-expression %s\n", sub_exp);
		sub_result = parse_parameter(sub_exp);
		free(sub_exp);

		if (input[1] != '(' && sub_result.offset != 0) {
			printf("... which has an offset, which is obviously not valid without parenthesisation >:/\n", sub_exp);
			result.type = INVALID;
			return result;
		}

		result.type = sub_result.type | POINTER;
		result.value = sub_result.value;
	} else {
		printf("It's not a pointer... ");
		if (input[0] == 's' && input[1] == 'r' && is_number(&input[2])) {
			// "srN" form
			result.type = S_REGISTER;
			result.value = parse_number(&input[2]);
			printf("it's a special register !\n");
		} else if (input[0] == 'r' && is_number(&input[1])) {
			// "rN" form
			printf("it's a register !\n");
			result.type = REGISTER;
			result.value = parse_number(&input[1]);
		} else if (is_number(input)) {
			result.type = CONSTANT;
			result.value = parse_number(input);
			printf("it's a number !\n");
		} else {
			for (int i = 0; i < num_aliases; i++) {
				printf("Checking aliases...\n");
				if (strcmp(input, aliases[i].replacee) == 0) {
					printf("Ah! an alias! I will parse %s in place of %s\n",
                 aliases[i].replacer,
                 input);
					return parse_parameter(aliases[i].replacer);
				}
			}
			printf("idk what this is...\n");
			result.type = INVALID;
		}
	}

	if (result.offset != 0) {
		result.type = result.type | W_OFFSET;
	}

	return result;
}

#define PLAIN_GOTO 0x0001
#define FUNCTION_CALL 0x0004

#define GOTO_C_ZERO 		0x0002
#define GOTO_C_NONZERO 		0x0003
#define GOTO_C_NEGATIVE 	0x0102
#define GOTO_C_NONNEGATIVE 	0x0103
#define GOTO_C_POSITIVE 	0x0202
#define GOTO_C_NONPOSITIVE 	0x0203

uint16_t encode_goto(line_data_str line)
{
	uint16_t code = INVALID;

	if (strcmp(line.tokens[0], "goto") == 0) {
		code = PLAIN_GOTO;
	} else if (strcmp(line.tokens[0], "call") == 0) {
		code = FUNCTION_CALL;
	} else if (strncmp(line.tokens[0], "goto_if_", 8) == 0) {
		if (strcmp(line.tokens[0], "goto_if_zero") == 0) {
			code = GOTO_C_ZERO;
		} else if (strcmp(line.tokens[0], "goto_if_nonzero") == 0) {
			code = GOTO_C_NONZERO;
		} else if (strcmp(line.tokens[0], "goto_if_negative") == 0) {
			code = GOTO_C_NEGATIVE;
		} else if (strcmp(line.tokens[0], "goto_if_nonnegative") == 0) {
			code = GOTO_C_NONNEGATIVE;
		} else if (strcmp(line.tokens[0], "goto_if_positive") == 0) {
			code = GOTO_C_POSITIVE;
		} else if (strcmp(line.tokens[0], "goto_if_nonpositive") == 0) {
			code = GOTO_C_NONPOSITIVE;
		}

		if (code == INVALID) {
			fprintf(stderr,
							"Error: invalid ``goto\" code statement ``%s\" on line %i.\n",
							line.tokens[0],
							line.line_number);
			exit(EXIT_FAILURE);
		}

		if (line.token_count < 3) {
			fprintf(stderr,
							"Error: too few arguments for ``%s\" on line %i.\n",
							line.tokens[0],
							line.line_number);
			exit(EXIT_FAILURE);
		}

		parameter param = parse_parameter(line.tokens[1]);

		if (param.type != REGISTER || param.offset != 0) {
			if (line.token_count < 3) {
				fprintf(
					stderr,
					"Error: conditional ``goto\"-type statements only take registers as "
					"arguments, but ``%s\" on line %i is not a register.\n",
					line.tokens[1],
					line.line_number);
				exit(EXIT_FAILURE);
			}
		}
		
		printf("Encoding goto of type 0x%x with parameter value %d at 0x%x\n", code, param.value, code + 0x1000 * param.value);
		code += 0x1000 * param.value;
	}

	return code;
}

line_data_node *tokenize_file(const char *input_path) {
	FILE* input_file = fopen(input_path, "r");
	if (!input_file) {
		perror("Error opening input file");
		exit(EXIT_FAILURE);
	}

	line_data_node *head = NULL, *tail = NULL;
	char line_str[MAX_LINE_LENGTH];
	int line_number = 1;

	while (fgets(line_str, sizeof(line_str), input_file)) {
		char* trimmed_line = line_str;
		while (isspace((unsigned char)*trimmed_line))
			trimmed_line++;
		if (*trimmed_line == '\0') {
			line_number++;
			continue;
		}

		line_data_str* line_data = tokenize_line(trimmed_line, line_number);

		line_data_node* new_node = malloc(sizeof(line_data_node));
		new_node->data = line_data;
		new_node->next = NULL;
		if (!head) {
			head = tail = new_node;
		} else {
			tail->next = new_node;
			tail = new_node;
		}

		line_number++;
	}
	
	fclose(input_file);
	return head;
}

void process_line(line_data_str line, const char* fname) {
	if (line.token_count == 0)
		return;
	
	int first_token_length = strlen(line.tokens[0]);
	
	if (strcmp(line.tokens[0], "alias") == 0) {
		if (line.token_count != 3) {
			fprintf(stderr, "Error: syntax error defining alias on line %i; usage is alias [replacee] [replacer].\n", line.line_number);
			exit(EXIT_FAILURE);
		}

		aliases[num_aliases].replacee = malloc(strlen(line.tokens[1]) + 1);
		strcpy(aliases[num_aliases].replacee, line.tokens[1]);

		aliases[num_aliases].replacer = malloc(strlen(line.tokens[2]) + 1);
		strcpy(aliases[num_aliases].replacer, line.tokens[2]);

		num_aliases += 1;
	} else if (strcmp(line.tokens[0], "dealias") == 0) {
		for (int k = 1; k < line.token_count; k++) {
			for (int i = 0; i < num_aliases; i++) {
				if (strcmp(line.tokens[k], aliases[i].replacee) == 0) {
					free(aliases[i].replacee);
					free(aliases[i].replacer);
					
					num_aliases -= 1;
					for (int j = i; j < num_aliases; j++) {
						aliases[j].replacee = aliases[j + 1].replacee;
						aliases[j].replacer = aliases[j + 1].replacer;
					}
				}
			}
		}
	} else if (line.token_count == 1 && line.tokens[0][first_token_length - 1] == ':') {
		strncpy(labels[n_labels], line.tokens[0], first_token_length - 1);
		// I have to manually null-terminate the string, since strncpy doesn't copy it over
		labels[n_labels][first_token_length - 1] = 0;
		label_positions[n_labels] = current_position;
		n_labels += 1;
	} else if (strncmp(line.tokens[0], "goto", 4) == 0 ||
		   strcmp(line.tokens[0], "call") == 0) {
		int code = encode_goto(line);

		if (code == INVALID) {
			fprintf(stderr,
							"Error: nonsense ``goto\"-type statement `on line %i.\n",
							line.tokens[0],
							line.line_number);
			exit(EXIT_FAILURE);
		}

		if (code == PLAIN_GOTO || code == FUNCTION_CALL) {
			strncpy(label_refs[n_label_refs].label,
							line.tokens[1],
							MAX_LABEL_LENGTH);
		} else {
			strncpy(label_refs[n_label_refs].label,
							line.tokens[2],
							MAX_LABEL_LENGTH);
		}

		label_refs[n_label_refs].position = current_position;
		label_refs[n_label_refs].line_number = line.line_number;
		label_refs[n_label_refs].fname = fname;
		n_label_refs += 1;

		WRITEOUT(code);
		WRITEOUT(0);
	} else {
		for (int i = 0; i < num_aliases; i++) {
			for (int j = 0; j < line.token_count; j++) {
				if (strcmp(line.tokens[j], aliases[i].replacee) == 0) {
					line.tokens[j] = realloc(line.tokens[j],
										   strlen(aliases[i].replacer) + 1);
					strcpy(line.tokens[j], aliases[i].replacer);
				}
			}
		}
		
		parameter params[MAX_PARAMS];
		int argc = line.token_count - 1;
		
		printf("Number of arguments: %d\n", argc);
		
		printf("I repeat: line %d, which has %d tokens, which are\n    ``%s\"", line.line_number, line.token_count, line.tokens[0]);

		for (int i = 1; i < line.token_count; ++i)
			printf(", ``%s\"", line.tokens[i]);
		
		putchar('\n');
		
		if (argc >= MAX_PARAMS) {
			fprintf(
					stderr,
					"Error: too many arguments on ``%s\", line %i.\n", fname, line.line_number);
				exit(EXIT_FAILURE);
		}
		
		for (int i = 0; i < argc; ++i) {
			params[i] = parse_parameter(line.tokens[i + 1]);
			printf("argument ``%s\", type %i\n", line.tokens[i + 1], params[i].type);
			if (params[i].type == INVALID) {
				fprintf(
					stderr,
					"Error: invalid argument ``%s\" for instruction ``%s\" on line %i.\n",
					line.tokens[i + 1],
					line.tokens[0],
					line.line_number);
				exit(EXIT_FAILURE);
			}
		}
		
		int found = 0;
		int index;
			
		for (index = 0; !found && index < N_INSTR; index++) {
			if (strcmp(line.tokens[0], dunk_instrs[index].name) == 0 &&
					line.token_count - 1 == dunk_instrs[index].n_args) {
				found = 1;
				
				for (int j = 0; j < argc; j++) {
					if (params[j].type != dunk_instrs[index].arg_types[j]) {
						found = 0;
						break;
					}
				}
			}
		}

		if (!found) {
			fprintf(stderr, "Error: no instruction ``%s\" with the matching type signature (%i", line.tokens[0], params[0].type);
			for (int j = 1; j < argc; j++) {
				fprintf(stderr, ", %i", params[j].type);
			}
			fprintf(stderr, ") on line %i.\n", line.line_number);
			exit(EXIT_FAILURE);
		} else {
			index--;

			int code = dunk_instrs[index].code;
			
			for (int j = 0; j < argc; j++) {
				if (dunk_instrs[index].arg_positions[j] & FIRST_NIBBLE) {
					code += params[j].value * 0x1000;
				}
				if (dunk_instrs[index].arg_positions[j] & SECOND_NIBBLE) {
					code += params[j].value * 0x0100;
				}
			}

			WRITEOUT(code);

			printf("Command %s, %i arguments:\n", dunk_instrs[index].name, argc);
			
			for (int j = 0; j < argc; j++) {
				printf("    type %i in position 0b%16b\n",
				dunk_instrs[index].arg_types[j],
				dunk_instrs[index].arg_positions[j]);
			}

			for (int k = 1; k < argc + 1; k++) {
				for (int j = 0; j < argc; j++) {
					printf("Check if argument %i follows in position %i, by comparing 0b%16b with 0b%16b and 0b%16b\n",
						j, k, dunk_instrs[index].arg_positions[j], FOLLOWING(k), OFFSET_FOLLOWING(k));
						
					if ((dunk_instrs[index].arg_positions[j] & FOLLOWING_MASK) == FOLLOWING(k)) {
						printf("it does.\n", j, k);
						WRITEOUT(params[j].value);
					} else if ((dunk_instrs[index].arg_positions[j] & FOLLOWING_MASK) == OFFSET_FOLLOWING(k)) {
						printf("its offset does.\n", j, k);
						WRITEOUT(params[j].offset);
					}
				}
			}
		}
	}
}

// Function to process file input and generate output
void process_file(const char* input_path)
{
	printf("Assembling file %s.\n", input_path);

	line_data_node *head = tokenize_file(input_path);
	line_data_node* current = head;
	
	uint16_t word, addr, c;
	int reg1, reg2;
	int found = 0;
	uint16_t bonus_writeouts[5];
	uint16_t writeout_mask[10];
	int n_bwriteouts, k, temp;
	int argc;
	
	line_data_str line;

	while (current) {
		line = *current->data;

		printf("Assembling line %d, which has %d tokens, which are\n    ``%s\"", line.line_number, line.token_count, line.tokens[0]);

		for (int i = 1; i < line.token_count; ++i)
			printf(", ``%s\"", line.tokens[i]);
		
		putchar('\n');
		
		for (int i = 0; i < line.token_count; ++i) {
			if (line.tokens[i][0] == '%') {
				line.token_count = i;
				break;
			}
		}
		
		process_line(line, input_path);

		current = current->next;
	}

	/** De-allocation **/
	
	/* Don't allow declared aliases to carry over between files; free the malloc'd strings */
	for (int i = N_DEFAULT_ALIASES; i < num_aliases; i++) {
		free(aliases[i].replacee);
		free(aliases[i].replacer);
	}
	num_aliases = N_DEFAULT_ALIASES;
	
	free_list(head);
}

void init_global_variables() {
	binary_output = malloc(BINSIZE);
	
	for (int i = 0; i < BINSIZE; i++) {
		binary_output[1] = 0;
	}
	
	num_aliases = N_DEFAULT_ALIASES;
	
	aliases[0].replacee = malloc(strlen("pk") + 1);
	strcpy(aliases[0].replacee, "pk");

	aliases[0].replacer = malloc(strlen("sr0") + 1);
	strcpy(aliases[0].replacer, "sr0");

	aliases[1].replacee = malloc(strlen("sp") + 1);
	strcpy(aliases[1].replacee, "sp");

	aliases[1].replacer = malloc(strlen("sr1") + 1);
	strcpy(aliases[1].replacer, "sr1");

	aliases[2].replacee = malloc(strlen("argument") + 1);
	strcpy(aliases[2].replacee, "argument");

	aliases[2].replacer = malloc(strlen("*(sr1+1)") + 1);
	strcpy(aliases[2].replacer, "*(sr1+1)");

	aliases[3].replacee = malloc(strlen("result") + 1);
	strcpy(aliases[3].replacee, "result");

	aliases[3].replacer = malloc(strlen("*(sr1+1)") + 1);
	strcpy(aliases[3].replacer, "*(sr1+1)");

	int arslen = 9;
	char buf[arslen];
	for (int i = 0; i < N_ARGUMENT_ALIASES; i++) {
		sprintf(buf, "argument%d", i);
		aliases[i+4].replacee = malloc(sizeof(char) * arslen);
		strcpy(aliases[i+4].replacee, buf);
		
		sprintf(buf, "*(sr1+%d)", i);
		aliases[4].replacer = malloc(sizeof(char) * arslen);
		strcpy(aliases[4].replacer, buf);
	}

	for (int i = num_aliases; i < MAX_N_ALIASES; i++) {
		aliases[i].replacee = NULL;
		aliases[i].replacer = NULL;
	}
	
	for (int i = 0; i < MAX_LABELS; i++) {
		for (int j = 0; j < MAX_LABEL_LENGTH; j++) {
			labels[j][i] = 0;
		}
	}
	
	for (int i = 0; i < MAX_LABEL_REFS; i++) {
		for (int j = 0; j < MAX_LABEL_LENGTH; j++) {
			labels[j][i] = 0;
			label_refs[i].label[j] = 0;
		}
	}
}

void insert_label_addresses() {
	int label_index;
	int found;

	for (int i = 0; i < n_label_refs; i++) {
		found = 0;
		for (int j = 0; j < n_labels && !found; j++) {
			if (strcmp(label_refs[i].label, labels[j]) == 0) {
				label_index = j;
				found = 1;
			}
		}

		if (!found) {
			fprintf(
				stderr,
				"Error: label ``%s\'\', referenced on line %i, was never defined.\n",
				label_refs[i].label,
				label_refs[i].line_number);
			exit(EXIT_FAILURE);
		}
		
		current_position = label_refs[i].position + 1;
		WRITEOUT(label_positions[label_index]);
	}
}

// Main function to handle input arguments
int main(int argc, char* argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <input file(s)> [-o <output file>]\n", argv[0]);
		return EXIT_FAILURE;
	}

	char** input_paths = NULL;
	char* output_path = NULL;

	int n_files = 0;

	// Parse arguments
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
			output_path = argv[++i];
		} else {
			n_files++;
		}
	}
	
	if (n_files == 0) {
		fprintf(stderr, "Error: No input files specified.\n");
		return EXIT_FAILURE;
	}
	
	input_paths = malloc(sizeof(char*) * n_files);
	
	int j = 0;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-o") == 0) {
			i++;
		} else {
			input_paths[j] = argv[i];
			j++;
		}
	}

	// Generate default output path if not provided
	int output_path_generated = 0;
	if (!output_path) {
		size_t len = strlen(input_paths[0]) + 10;
		output_path = malloc(len);
		if (!output_path) {
			perror("Memory allocation failed");
			return EXIT_FAILURE;
		}
		snprintf(output_path, len, "%s-assembled", input_paths[0]);
		output_path_generated = 1;
	}
	
	init_global_variables();

	for (int i = 0; i < n_files; i++) {
		process_file(input_paths[i]);
	}
	
	// Save the writing position at the end of writing code to the binary_output array; 
	// The function insert_label_addresses() messes with this value, since it has to go
	// back to insert label addresses, and it writes to the binary_output array using
	// the WRITEOUT macro, which uses current_position. Not great practice, I admit,
	// but it works fine for a project on this scale.
	int words_generated = current_position;
	
	insert_label_addresses();
	
	FILE* output_file = fopen(output_path, "wb");
	if (!output_file) {
		perror("Error opening output file");
		exit(EXIT_FAILURE);
	}
	
	fwrite(binary_output, 2, words_generated, output_file);
	fclose(output_file);
	
	free(binary_output);
	free(input_paths);
	
	if (output_path_generated) {
		free(output_path);
	}

	return EXIT_SUCCESS;
}
