#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024
#define MAX_LABEL_LENGTH 32
#define MAX_TOKENS 100

#define BINSIZE 2 * 64 * 1024

#define N_COMMANDS 10
#define numer number

#define N_ARG_TYPES 4

#define CONSTANT 1
#define REGISTER 2
#define S_REGISTER 3
#define INVALID -1

#define POINTER 4
#define W_OFFSET 8

#define FIRST_NIBBLE 0b10000000000
#define SECOND_NIBBLE 0b01000000000
#define FOLLOWING(N) 0b00100000000 + N
#define OFFSET_FOLLOWING(N) 0b00010000000 + N
#define FOLLOWING_MASK 0b00111111111

#define WRITEOUT(X)                                                            \
	do {                                                                         \
		printf("WRITEOUT 0x%04x at position %i\n", X, current_position);           \
		word_ptr = (uint16_t*)(&byte_data[2 * current_position]);                  \
		*word_ptr = X;                                                             \
		current_position += 1;                                                     \
		word_ptr = (uint16_t*)(&byte_data[2 * current_position]);                  \
	} while (0);

typedef struct
{
	const char* name;
	uint16_t code;
	int n_args;
	int arg_types[4];
	int arg_positions[4];
} command;

#define MAX_N_ALIASES 512

typedef struct
{
	char* replacee;
	char* replacer;
} alias;

int num_aliases = 4;
alias aliases[MAX_N_ALIASES];

#define _R___R__                                                               \
	2, { REGISTER, REGISTER, 0, 0 }, { FIRST_NIBBLE, SECOND_NIBBLE, 0, 0 }

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

#define N_CMDS 120

command commands[N_CMDS] = {
	NULLARY_COMMAND("chill", 0x0000),

	//**ALU**//
	BINARY__COMMAND("add",
									0x0010,
									REGISTER,
									FIRST_NIBBLE,
									REGISTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("subtract",
									0x0011,
									REGISTER,
									FIRST_NIBBLE,
									REGISTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("multiply",
									0x0012,
									REGISTER,
									FIRST_NIBBLE,
									REGISTER,
									SECOND_NIBBLE),

	UNARY___COMMAND("increment", 0x0013, REGISTER, FIRST_NIBBLE | SECOND_NIBBLE),
	UNARY___COMMAND("decrement", 0x0014, REGISTER, FIRST_NIBBLE | SECOND_NIBBLE),
	UNARY___COMMAND("negate", 0x0015, REGISTER, FIRST_NIBBLE | SECOND_NIBBLE),

	BINARY__COMMAND("compare_unsigned",
									0x0016,
									REGISTER,
									FIRST_NIBBLE,
									REGISTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("compare",
									0x0017,
									REGISTER,
									FIRST_NIBBLE,
									REGISTER,
									SECOND_NIBBLE),

	UNARY___COMMAND("sign", 0x0018, REGISTER, FIRST_NIBBLE | SECOND_NIBBLE),

	BINARY__COMMAND("and",
									0x0019,
									REGISTER,
									FIRST_NIBBLE,
									REGISTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("or",
									0x001a,
									REGISTER,
									FIRST_NIBBLE,
									REGISTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("xor",
									0x001b,
									REGISTER,
									FIRST_NIBBLE,
									REGISTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("not",
									0x001c,
									REGISTER,
									FIRST_NIBBLE,
									REGISTER,
									SECOND_NIBBLE),

	UNARY___COMMAND("lshift", 0x001d, REGISTER, FIRST_NIBBLE | SECOND_NIBBLE),
	UNARY___COMMAND("rshift", 0x001e, REGISTER, FIRST_NIBBLE | SECOND_NIBBLE),
	BINARY__COMMAND("shift",
									0x001e,
									REGISTER,
									FIRST_NIBBLE,
									REGISTER,
									SECOND_NIBBLE),

	//**stack stuff**//
	NULLARY_COMMAND("return", 0x0005),

	// pushes
	NULLARY_COMMAND("push", 0x0006),
	UNARY___COMMAND("push", 0x0106, CONSTANT, FOLLOWING(1)),
	UNARY___COMMAND("push", 0x0206, CONSTANT | POINTER, FOLLOWING(1)),
	UNARY___COMMAND("push", 0x0306, REGISTER, FIRST_NIBBLE),
	UNARY___COMMAND("push", 0x0406, REGISTER | POINTER, FIRST_NIBBLE),
	UNARY___COMMAND("push",
									0x0506,
									REGISTER | POINTER | W_OFFSET,
									FIRST_NIBBLE | OFFSET_FOLLOWING(1)),
	UNARY___COMMAND("push", 0x0606, S_REGISTER, FIRST_NIBBLE),
	UNARY___COMMAND("push", 0x0706, S_REGISTER | POINTER, FIRST_NIBBLE),
	UNARY___COMMAND("push",
									0x0806,
									S_REGISTER | POINTER | W_OFFSET,
									FIRST_NIBBLE | OFFSET_FOLLOWING(1)),

	// pops
	NULLARY_COMMAND("pop", 0x0007),
	UNARY___COMMAND("pop", 0x0107, CONSTANT, FOLLOWING(1)),
	UNARY___COMMAND("pop", 0x0207, CONSTANT | POINTER, FOLLOWING(1)),
	UNARY___COMMAND("pop", 0x0307, REGISTER, FIRST_NIBBLE),
	UNARY___COMMAND("pop", 0x0407, REGISTER | POINTER, FIRST_NIBBLE),
	UNARY___COMMAND("pop",
									0x0507,
									REGISTER | POINTER | W_OFFSET,
									FIRST_NIBBLE | OFFSET_FOLLOWING(1)),
	UNARY___COMMAND("pop", 0x0707, S_REGISTER, FIRST_NIBBLE),
	UNARY___COMMAND("pop", 0x0707, S_REGISTER | POINTER, FIRST_NIBBLE),
	UNARY___COMMAND("pop",
									0x0807,
									S_REGISTER | POINTER | W_OFFSET,
									FIRST_NIBBLE | OFFSET_FOLLOWING(1)),

	//**setting**//
	BINARY__COMMAND("set",
									0x0030,
									CONSTANT | POINTER,
									FOLLOWING(1),
									CONSTANT,
									FOLLOWING(2)),
	BINARY__COMMAND("set",
									0x0130,
									CONSTANT | POINTER,
									FOLLOWING(1),
									CONSTANT | POINTER,
									FOLLOWING(2)),

	BINARY__COMMAND("set",
									0x0031,
									CONSTANT | POINTER,
									FOLLOWING(1),
									REGISTER,
									FIRST_NIBBLE),
	BINARY__COMMAND("set",
									0x0131,
									CONSTANT | POINTER,
									FOLLOWING(1),
									REGISTER | POINTER,
									FIRST_NIBBLE),
	BINARY__COMMAND("set",
									0x0231,
									CONSTANT | POINTER,
									FOLLOWING(1),
									REGISTER | POINTER | W_OFFSET,
									FIRST_NIBBLE | FOLLOWING(2)),

	BINARY__COMMAND("set",
									0x0331,
									CONSTANT | POINTER,
									FOLLOWING(1),
									S_REGISTER,
									FIRST_NIBBLE),
	BINARY__COMMAND("set",
									0x0431,
									CONSTANT | POINTER,
									FOLLOWING(1),
									S_REGISTER | POINTER,
									FIRST_NIBBLE),
	BINARY__COMMAND("set",
									0x0531,
									CONSTANT | POINTER,
									FOLLOWING(1),
									S_REGISTER | POINTER | W_OFFSET,
									FIRST_NIBBLE | FOLLOWING(2)),

	BINARY__COMMAND("set",
									0x0033,
									REGISTER,
									FIRST_NIBBLE,
									CONSTANT,
									FOLLOWING(1)),
	BINARY__COMMAND("set",
									0x0034,
									REGISTER,
									FIRST_NIBBLE,
									CONSTANT | POINTER,
									FOLLOWING(1)),

	BINARY__COMMAND("set",
									0x0035,
									REGISTER,
									FIRST_NIBBLE,
									REGISTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x0036,
									REGISTER,
									FIRST_NIBBLE,
									REGISTER | POINTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x0037,
									REGISTER,
									FIRST_NIBBLE,
									REGISTER | POINTER | W_OFFSET,
									SECOND_NIBBLE | OFFSET_FOLLOWING(1)),

	BINARY__COMMAND("set",
									0x0038,
									REGISTER,
									FIRST_NIBBLE,
									S_REGISTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x0039,
									REGISTER,
									FIRST_NIBBLE,
									S_REGISTER | POINTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x003a,
									REGISTER,
									FIRST_NIBBLE,
									S_REGISTER | POINTER | W_OFFSET,
									SECOND_NIBBLE | OFFSET_FOLLOWING(1)),

	BINARY__COMMAND("set",
									0x003b,
									REGISTER | POINTER,
									FIRST_NIBBLE,
									CONSTANT,
									FOLLOWING(1)),
	BINARY__COMMAND("set",
									0x003c,
									REGISTER | POINTER,
									FIRST_NIBBLE,
									CONSTANT | POINTER,
									FOLLOWING(1)),

	BINARY__COMMAND("set",
									0x003d,
									REGISTER | POINTER,
									FIRST_NIBBLE,
									REGISTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x003e,
									REGISTER | POINTER,
									FIRST_NIBBLE,
									REGISTER | POINTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x003f,
									REGISTER | POINTER,
									FIRST_NIBBLE,
									REGISTER | POINTER | W_OFFSET,
									SECOND_NIBBLE | OFFSET_FOLLOWING(1)),

	BINARY__COMMAND("set",
									0x0040,
									REGISTER | POINTER,
									FIRST_NIBBLE,
									S_REGISTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x0041,
									REGISTER | POINTER,
									FIRST_NIBBLE,
									S_REGISTER | POINTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x0042,
									REGISTER | POINTER,
									FIRST_NIBBLE,
									S_REGISTER | POINTER | W_OFFSET,
									SECOND_NIBBLE | OFFSET_FOLLOWING(1)),

	BINARY__COMMAND("set",
									0x0043,
									REGISTER | POINTER | W_OFFSET,
									FIRST_NIBBLE | OFFSET_FOLLOWING(1),
									CONSTANT,
									FOLLOWING(2)),
	BINARY__COMMAND("set",
									0x0143,
									REGISTER | POINTER | W_OFFSET,
									FIRST_NIBBLE | OFFSET_FOLLOWING(1),
									CONSTANT | POINTER,
									FOLLOWING(2)),

	BINARY__COMMAND("set",
									0x0044,
									REGISTER | POINTER | W_OFFSET,
									FIRST_NIBBLE | OFFSET_FOLLOWING(1),
									REGISTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x0045,
									REGISTER | POINTER | W_OFFSET,
									FIRST_NIBBLE | OFFSET_FOLLOWING(1),
									REGISTER | POINTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x0046,
									REGISTER | POINTER | W_OFFSET,
									FIRST_NIBBLE | OFFSET_FOLLOWING(1),
									REGISTER | POINTER | W_OFFSET,
									SECOND_NIBBLE | OFFSET_FOLLOWING(2)),

	BINARY__COMMAND("set",
									0x0047,
									REGISTER | POINTER | W_OFFSET,
									FIRST_NIBBLE | OFFSET_FOLLOWING(1),
									S_REGISTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x0048,
									REGISTER | POINTER | W_OFFSET,
									FIRST_NIBBLE | OFFSET_FOLLOWING(1),
									S_REGISTER | POINTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x0049,
									REGISTER | POINTER | W_OFFSET,
									FIRST_NIBBLE | OFFSET_FOLLOWING(1),
									S_REGISTER | POINTER | W_OFFSET,
									SECOND_NIBBLE | OFFSET_FOLLOWING(2)),

	BINARY__COMMAND("set",
									0x004a,
									S_REGISTER,
									FIRST_NIBBLE,
									CONSTANT,
									FOLLOWING(1)),
	BINARY__COMMAND("set",
									0x014a,
									S_REGISTER,
									FIRST_NIBBLE,
									CONSTANT | POINTER,
									FOLLOWING(1)),

	BINARY__COMMAND("set",
									0x004b,
									S_REGISTER,
									FIRST_NIBBLE,
									REGISTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x004c,
									S_REGISTER,
									FIRST_NIBBLE,
									REGISTER | POINTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x004d,
									S_REGISTER,
									FIRST_NIBBLE,
									REGISTER | POINTER | W_OFFSET,
									SECOND_NIBBLE | OFFSET_FOLLOWING(1)),

	BINARY__COMMAND("set",
									0x004e,
									S_REGISTER,
									FIRST_NIBBLE,
									S_REGISTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x004f,
									S_REGISTER,
									FIRST_NIBBLE,
									S_REGISTER | POINTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x0050,
									S_REGISTER,
									FIRST_NIBBLE,
									S_REGISTER | POINTER | W_OFFSET,
									SECOND_NIBBLE | OFFSET_FOLLOWING(1)),

	BINARY__COMMAND("set",
									0x0051,
									S_REGISTER | POINTER,
									FIRST_NIBBLE,
									CONSTANT,
									FOLLOWING(1)),
	BINARY__COMMAND("set",
									0x0151,
									S_REGISTER | POINTER,
									FIRST_NIBBLE,
									CONSTANT | POINTER,
									FOLLOWING(1)),

	BINARY__COMMAND("set",
									0x0052,
									S_REGISTER | POINTER,
									FIRST_NIBBLE,
									REGISTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x0053,
									S_REGISTER | POINTER,
									FIRST_NIBBLE,
									REGISTER | POINTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x0054,
									S_REGISTER | POINTER,
									FIRST_NIBBLE,
									REGISTER | POINTER | W_OFFSET,
									SECOND_NIBBLE | OFFSET_FOLLOWING(1)),

	BINARY__COMMAND("set",
									0x0055,
									S_REGISTER | POINTER,
									FIRST_NIBBLE,
									S_REGISTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x0056,
									S_REGISTER | POINTER,
									FIRST_NIBBLE,
									S_REGISTER | POINTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x0057,
									S_REGISTER | POINTER,
									FIRST_NIBBLE,
									S_REGISTER | POINTER | W_OFFSET,
									SECOND_NIBBLE | OFFSET_FOLLOWING(1)),

	BINARY__COMMAND("set",
									0x0058,
									S_REGISTER | POINTER | W_OFFSET,
									FIRST_NIBBLE | OFFSET_FOLLOWING(1),
									CONSTANT,
									FOLLOWING(2)),
	BINARY__COMMAND("set",
									0x0059,
									S_REGISTER | POINTER | W_OFFSET,
									FIRST_NIBBLE | OFFSET_FOLLOWING(1),
									CONSTANT | POINTER,
									FOLLOWING(2)),

	BINARY__COMMAND("set",
									0x005a,
									S_REGISTER | POINTER | W_OFFSET,
									FIRST_NIBBLE | OFFSET_FOLLOWING(1),
									REGISTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x005b,
									S_REGISTER | POINTER | W_OFFSET,
									FIRST_NIBBLE | OFFSET_FOLLOWING(1),
									REGISTER | POINTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x005c,
									S_REGISTER | POINTER | W_OFFSET,
									FIRST_NIBBLE | OFFSET_FOLLOWING(1),
									REGISTER | POINTER | W_OFFSET,
									SECOND_NIBBLE | OFFSET_FOLLOWING(2)),

	BINARY__COMMAND("set",
									0x005d,
									S_REGISTER | POINTER | W_OFFSET,
									FIRST_NIBBLE | OFFSET_FOLLOWING(1),
									S_REGISTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x005e,
									S_REGISTER | POINTER | W_OFFSET,
									FIRST_NIBBLE | OFFSET_FOLLOWING(1),
									S_REGISTER | POINTER,
									SECOND_NIBBLE),
	BINARY__COMMAND("set",
									0x005f,
									S_REGISTER | POINTER | W_OFFSET,
									FIRST_NIBBLE | OFFSET_FOLLOWING(1),
									S_REGISTER | POINTER | W_OFFSET,
									SECOND_NIBBLE | OFFSET_FOLLOWING(2)),
};

typedef struct
{
	char label[MAX_LABEL_LENGTH];
	uint16_t position;
	int line_number;
} symbol_reference;

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

	// printf("Parsing parameter ``%s\"\n", input);

	if (input[0] == '*') {
		// printf("It's a pointer, ");
		if (input[1] == '(') {
			// printf("parenthesised, with inner expression");
			inside = &input[2];
			close_paren = strchr(inside, ')');

			if (close_paren == NULL) {
				// printf("... but not closing paren :(\n");
				result.type = INVALID;
				return result;
			}
			if (close_paren[1] != 0) {
				// printf("... but does not end after its closing paren :(\n");

				result.type = INVALID;
				return result;
			}

			len = close_paren - inside;
			sub_exp = strndup(inside, len);

			// printf(" \"%s\", ", sub_exp);

			plus = &sub_exp[strcspn(sub_exp, "+-")];

			if (plus != NULL) {
				if (plus[0] == '+') {
					sub_result = parse_parameter(&plus[1]);
				} else {
					sub_result = parse_parameter(plus);
				}

				if (sub_result.type != CONSTANT) {
					// printf("... but it contains an offset which is not a constant
					// :(\n");

					result.type = INVALID;
					return result;
				}
				if (sub_result.offset != 0) {
					// printf("... but it contains a double-offset :(\n");

					result.type = INVALID;
					return result;
				}

				result.offset = sub_result.value;
				// printf(" which has offset %i\n", result.offset);
				*plus = 0;
			}

		} else {
			// printf("non-parenthesised, ");
			inside = &input[1];
			len = strlen(inside);
			sub_exp = strndup(inside, len);
		}

		// printf("with sub-expression %s\n", sub_exp);
		sub_result = parse_parameter(sub_exp);
		free(sub_exp);

		if (input[1] != '(' && sub_result.offset != 0) {
			// printf("... which has an offset, which is obviously not valid without
			// parenthesisation >:/\n", sub_exp);
			result.type = INVALID;
			return result;
		}

		result.type = sub_result.type | POINTER;
		result.value = sub_result.value;
	} else {
		// printf("It's not a pointer... ");
		if (input[0] == 's' && input[1] == 'r' && is_number(&input[2])) {
			// "srN" form
			result.type = S_REGISTER;
			result.value = parse_number(&input[2]);
			// printf("it's a special register !\n");
		} else if (input[0] == 'r' && is_number(&input[1])) {
			// "rN" form
			// printf("it's a register !\n");
			result.type = REGISTER;
			result.value = parse_number(&input[1]);
		} else if (is_number(input)) {
			result.type = CONSTANT;
			result.value = parse_number(input);
			// printf("it's a number !\n");
		} else {
			for (int i = 0; i < num_aliases; i++) {
				if (strcmp(input, aliases[i].replacee) == 0) {
					printf("Ah! an alias! I will parse %s in place of %s\n",
                 aliases[i].replacer,
                 input);
					return parse_parameter(aliases[i].replacer);
				}
			}
			// printf("idk what this is...\n");
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

#define GOTO_C_ZERO 0x0002
#define GOTO_C_NONZERO 0x0003
#define GOTO_C_NEGATIVE 0x0102
#define GOTO_C_NONNEGATIVE 0x0103
#define GOTO_C_POSITIVE 0x0202
#define GOTO_C_NONPOSITIVE 0x0203

uint16_t
encode_goto(line_data_str line)
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

		code += 0x1000 * param.value;
	}

	return code;
}

// Function to process file input and generate output
void process_file(const char* input_path, const char* output_path)
{
	FILE* input_file = fopen(input_path, "r");
	if (!input_file) {
		perror("Error opening input file");
		exit(EXIT_FAILURE);
	}

	line_data_node *head = NULL, *tail = NULL;
	char line[MAX_LINE_LENGTH];
	int line_number = 1;

	while (fgets(line, sizeof(line), input_file)) {
		// Trim leading and trailing whitespace
		char* trimmed_line = line;
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

	// Write processed data to output file
	FILE* output_file = fopen(output_path, "wb");
	if (!output_file) {
		perror("Error opening output file");
		free_list(head);
		exit(EXIT_FAILURE);
	}

	uint8_t* byte_data = malloc(BINSIZE);
	for (int i = 0; i < BINSIZE; i++) {
		byte_data[1] = 0;
	}

	int current_position = 0;
	uint16_t* word_ptr;

	int n_labels = 0;
	int n_gotos = 0;

	char labels[line_number][MAX_LABEL_LENGTH];
	uint16_t label_positions[line_number];
	symbol_reference goto_instances[line_number];

	for (int i = 0; i < line_number; i++) {
		for (int j = 0; j < MAX_LABEL_LENGTH; j++) {
			labels[j][i] = 0;
			goto_instances[i].label[j] = 0;
		}
	}

	line_data_node* current = head;
	int length;
	uint16_t word, addr, c;
	int reg1, reg2;
	parameter pms[10];
	int found = 0;
	int index;
	uint16_t bonus_writeouts[5];
	uint16_t writeout_mask[10];
	uint16_t code;
	int n_bwriteouts, k, temp;
	int argc;

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

	for (int i = num_aliases; i < MAX_N_ALIASES; i++) {
		aliases[i].replacee = NULL;
		aliases[i].replacer = NULL;
	}

	while (current) {
		// printf("Line %i\n", current->data->line_number);
		length = strlen(current->data->tokens[0]);
		word_ptr = (uint16_t*)(&byte_data[2 * current_position]);

		printf("Assembling line %d, which has %d tokens, which are\n    ``%s\"",
           current->data->line_number,
           current->data->token_count,
           current->data->tokens[0]);

		for (int i = 1; i < current->data->token_count; ++i) {
			printf(", ``%s\"", current->data->tokens[i]);
		}
		putchar('\n');

		if (current->data->tokens[0][0] == '%') {
			// ignore commented lines
		} else if (strcmp(current->data->tokens[0], "alias") == 0) {
			if (current->data->token_count != 3) {
				fprintf(stderr,
								"Error: syntax error defining alias on line %i; usage is alias "
								"[replacee] [replacer].\n",
								current->data->line_number);
				exit(EXIT_FAILURE);
			}

			aliases[num_aliases].replacee =
				malloc(strlen(current->data->tokens[1]) + 1);
			strcpy(aliases[num_aliases].replacee, current->data->tokens[1]);

			aliases[num_aliases].replacer =
				malloc(strlen(current->data->tokens[2]) + 1);
			strcpy(aliases[num_aliases].replacer, current->data->tokens[2]);

			num_aliases += 1;
		} else if (strcmp(current->data->tokens[0], "dealias") == 0) {
			for (int k = 1; k < current->data->token_count; k++) {
				for (int i = 0; i < num_aliases; i++) {
					if (strcmp(current->data->tokens[k], aliases[i].replacee) == 0) {
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
		} else if (current->data->token_count == 1 &&
               current->data->tokens[0][length - 1] == ':') {
			strncpy(labels[n_labels], current->data->tokens[0], length - 1);
			labels[n_labels][length - 1] = 0;
			label_positions[n_labels] = current_position;
			n_labels += 1;
		} else if (strncmp(current->data->tokens[0], "goto", 4) == 0 ||
               strcmp(current->data->tokens[0], "call") == 0) {
			code = encode_goto(*current->data);

			if (code == INVALID) {
				fprintf(stderr,
								"Error: nonsense ``goto\"-type statement `on line %i.\n",
								current->data->tokens[0],
								current->data->line_number);
				exit(EXIT_FAILURE);
			}

			if (code == PLAIN_GOTO || code == FUNCTION_CALL) {
				strncpy(goto_instances[n_gotos].label,
								current->data->tokens[1],
								MAX_LABEL_LENGTH);
			} else {
				strncpy(goto_instances[n_gotos].label,
								current->data->tokens[2],
								MAX_LABEL_LENGTH);
			}

			goto_instances[n_gotos].position = current_position;
			goto_instances[n_gotos].line_number = current->data->line_number;
			n_gotos += 1;

			WRITEOUT(code);
			WRITEOUT(0);
		} else {
			for (int i = 0; i < num_aliases; i++) {
				for (int j = 0; j < current->data->token_count; j++) {
					if (strcmp(current->data->tokens[j], aliases[i].replacee) == 0) {
						current->data->tokens[j] = realloc(current->data->tokens[j],
                                               strlen(aliases[i].replacer) + 1);
						strcpy(current->data->tokens[j], aliases[i].replacer);
					}
				}
			}

			argc = current->data->token_count - 1;
			for (int i = 0; i < argc; ++i) {
				pms[i] = parse_parameter(current->data->tokens[i + 1]);
				printf("argument ``%s\", type %i\n",
               current->data->tokens[i + 1],
               pms[i].type);
				if (pms[i].type == INVALID) {
					fprintf(
						stderr,
						"Error: invalid argument ``%s\" for command ``%s\" on line %i.\n",
						current->data->tokens[i + 1],
						current->data->tokens[0],
						current->data->line_number);
					exit(EXIT_FAILURE);
				}
			}

			found = 0;
			for (index = 0; !found && index < N_CMDS; index++) {
				if (strcmp(current->data->tokens[0], commands[index].name) == 0 &&
						current->data->token_count - 1 == commands[index].n_args) {
					found = 1;
					for (int j = 0; j < argc; j++) {
						if (pms[j].type != commands[index].arg_types[j]) {
							found = 0;
							break;
						}
					}
				}
			}

			if (!found) {
				fprintf(stderr,
								"Error: no command ``%s\" with the matching type signature (%i",
								current->data->tokens[0],
								pms[0].type);
				for (int j = 1; j < argc; j++) {
					fprintf(stderr, ", %i", pms[j].type);
				}
				fprintf(stderr, ") on line %i.\n", current->data->line_number);
				exit(EXIT_FAILURE);
			} else {
				index--;

				code = commands[index].code;
				for (int j = 0; j < argc; j++) {
					if (commands[index].arg_positions[j] & FIRST_NIBBLE) {
						code += pms[j].value * 0x1000;
					}
					if (commands[index].arg_positions[j] & SECOND_NIBBLE) {
						code += pms[j].value * 0x0100;
					}
				}

				WRITEOUT(code);

				printf("Command %s, %i arguments:\n", commands[index].name, argc);
				for (int j = 0; j < argc; j++) {
					printf("    type %i in position 0b%16b\n",
                 commands[index].arg_types[j],
                 commands[index].arg_positions[j]);
				}

				for (int k = 1; k < argc + 1; k++) {
					for (int j = 0; j < argc; j++) {
						printf("Check if argument %i follows in position %i, by comparing "
                   "0b%16b with 0b%16b and 0b%16b\n",
                   j,
                   k,
                   commands[index].arg_positions[j],
                   FOLLOWING(k),
                   OFFSET_FOLLOWING(k));
						if ((commands[index].arg_positions[j] & FOLLOWING_MASK) ==
								FOLLOWING(k)) {
							printf("it does.\n", j, k);
							WRITEOUT(pms[j].value);
						} else if ((commands[index].arg_positions[j] & FOLLOWING_MASK) ==
                       OFFSET_FOLLOWING(k)) {

							printf("its offset does.\n", j, k);
							WRITEOUT(pms[j].offset);
						}
					}
				}
			}
		}

		current = current->next;
	}

	int label_index;

	for (int i = 0; i < n_gotos; i++) {
		found = 0;
		for (int j = 0; j < n_labels; j++) {
			if (strcmp(goto_instances[i].label, labels[j]) == 0) {
				label_index = j;
				found = 1;
				break;
			}
		}

		if (found == 0) {
			fprintf(
				stderr,
				"Error: label ``%s\'\', referenced on line %i, was never defined.\n",
				goto_instances[i].label,
				goto_instances[i].line_number);
			exit(EXIT_FAILURE);
		} else {
			printf("Found label %s! Its position is %i\n",
             labels[label_index],
             label_positions[label_index]);
			current_position = goto_instances[i].position + 1;
			WRITEOUT(label_positions[label_index]);
		}
	}

	fwrite(byte_data, 1, BINSIZE, output_file);
	fclose(output_file);
	free_list(head);
	free(byte_data);
}

// Main function to handle input arguments
int main(int argc, char* argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <input file> [-o <output file>]\n", argv[0]);
		return EXIT_FAILURE;
	}

	char* input_path = NULL;
	char* output_path = NULL;

	//~ for (int i = 0; i < N_NULLARIES; i++) { strcpy(nullaries[i].name,
	//nullary_names[i]); } ~ for (int i = 0; i < N_UNARIES;   i++) { strcpy(
	//unaries[i].name, unary_names  [i]); } ~ for (int i = 0; i < N_BINARIES; i++)
	//{ strcpy( binaries[i].name, binary_names [i]); } ~ for (int i = 0; i <
	//N_TERNARIES; i++) { strcpy(ternaries[i].name, ternary_names[i]); }

	//~ //setup set
	//~ SET.codes    [REGISTER][REGISTER]      = 0x0009;
	//~ SET.positions[REGISTER][REGISTER][0]   = 0x0100;
	//~ SET.positions[REGISTER][REGISTER][1]   = 0x1000;

	//~ SET.codes    [REGISTER][REGISTER | POINTER]    = 0x000a;
	//~ SET.positions[REGISTER][REGISTER | POINTER][0] = 0x0100;
	//~ SET.positions[REGISTER][REGISTER | POINTER][1] = 0x1000;

	//~ SET.codes    [REGISTER][CONSTANT | POINTER]      = 0x0006;
	//~ SET.positions[REGISTER][CONSTANT | POINTER][0]   = 0x1000;
	//~ SET.positions[REGISTER][CONSTANT | POINTER][1]   = 1;

	//~ SET.codes    [REGISTER][CONSTANT | POINTER]    = 0x0007;
	//~ SET.positions[REGISTER][CONSTANT | POINTER][0] = 0x1000;
	//~ SET.positions[REGISTER][CONSTANT | POINTER][1] = 1;

	//~ SET.codes    [REGISTER | POINTER][REGISTER]      = 0x002a;
	//~ SET.positions[REGISTER | POINTER][REGISTER][0]   = 0x0100;
	//~ SET.positions[REGISTER | POINTER][REGISTER][1]   = 0x1000;

	//~ SET.codes    [REGISTER | POINTER][REGISTER | POINTER]    = 0;
	//~ //SET.positions[REGISTER | POINTER][REGISTER | POINTER][0] = 0x1000;
	//~ //SET.positions[REGISTER | POINTER][REGISTER | POINTER][1] = 0x0100;

	//~ SET.codes    [REGISTER | POINTER][CONSTANT | POINTER]      = 0;
	//~ //SET.positions[REGISTER | POINTER][CONSTANT | POINTER][0]   = 0x1000;
	//~ //SET.positions[REGISTER | POINTER][CONSTANT | POINTER][1]   = 1;

	//~ SET.codes    [REGISTER | POINTER][CONSTANT | POINTER]    = 0;
	//~ //SET.positions[REGISTER | POINTER][CONSTANT | POINTER][0] = 0x1000;
	//~ //SET.positions[REGISTER | POINTER][CONSTANT | POINTER][1] = 1;

	//~ SET.codes    [CONSTANT | POINTER][REGISTER]      = 0;
	//~ SET.codes    [CONSTANT | POINTER][REGISTER | POINTER]    = 0;
	//~ SET.codes    [CONSTANT | POINTER][CONSTANT | POINTER]      = 0;
	//~ SET.codes    [CONSTANT | POINTER][CONSTANT | POINTER]    = 0;

	//~ SET.codes    [CONSTANT | POINTER][REGISTER]      = 0x0008;
	//~ SET.positions[CONSTANT | POINTER][REGISTER][0]   = 1;
	//~ SET.positions[CONSTANT | POINTER][REGISTER][1]   = 0x1000;

	//~ SET.codes    [CONSTANT | POINTER][REGISTER | POINTER]    = 0x0028;
	//~ SET.positions[CONSTANT | POINTER][REGISTER | POINTER][0] = 1;
	//~ SET.positions[CONSTANT | POINTER][REGISTER | POINTER][1] = 0x1000;

	//~ SET.codes    [CONSTANT | POINTER][CONSTANT | POINTER]      = 0x0026;
	//~ SET.positions[CONSTANT | POINTER][CONSTANT | POINTER][0]   = 2;
	//~ SET.positions[CONSTANT | POINTER][CONSTANT | POINTER][1]   = 1;

	//~ SET.codes    [CONSTANT | POINTER][CONSTANT | POINTER]    = 0x0027;
	//~ SET.positions[CONSTANT | POINTER][CONSTANT | POINTER][0] = 2;
	//~ SET.positions[CONSTANT | POINTER][CONSTANT | POINTER][1] = 1;

	//~ //setup add
	//~ ADD.codes    [REGISTER][REGISTER][REGISTER]      = 0x0010;
	//~ ADD.positions[REGISTER][REGISTER][REGISTER][0]   = 0x1000;
	//~ ADD.positions[REGISTER][REGISTER][REGISTER][1]   = 0x0100;
	//~ ADD.positions[REGISTER][REGISTER][REGISTER][2]   = -1;

	//~ //add will only take three registers
	//~ for (int i = 0; i < N_ARG_TYPES; i++){
	//~ for (int j = 0; j < N_ARG_TYPES; j++){
	//~ for (int k = 0; k < N_ARG_TYPES; k++){
	//~ if (i + j + k != 0){
	//~ ADD.codes[i][j][k] = 0;
	//~ }
	//~ }
	//~ }
	//~ }

	// Parse arguments
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
			output_path = argv[++i];
		} else {
			input_path = argv[i];
		}
	}

	if (!input_path) {
		fprintf(stderr, "Error: Input file not specified.\n");
		return EXIT_FAILURE;
	}

	// Generate default output path if not provided
	if (!output_path) {
		size_t len = strlen(input_path) + 10;
		output_path = malloc(len);
		if (!output_path) {
			perror("Memory allocation failed");
			return EXIT_FAILURE;
		}
		snprintf(output_path, len, "%s-assembled", input_path);
	}

	process_file(input_path, output_path);

	if (output_path != argv[argc - 1]) {
		free(output_path);
	}

	return EXIT_SUCCESS;
}
