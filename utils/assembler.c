#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define MAX_LINE_LENGTH 1024
#define MAX_LABEL_LENGTH 32
#define MAX_TOKENS 100

#define BINSIZE 1024

#define N_COMMANDS 10
#define numer number

#define N_ARG_TYPES 4

#define CONSTANT    1
#define REGISTER    2
#define S_REGISTER  3
#define INVALID    -1

#define POINTER 4
#define W_OFFSET  8

#define FIRST_NIBBLE        0b10000000000
#define SECOND_NIBBLE 	    0b01000000000
#define FOLLOWING(N)        0b00100000000 + N
#define OFFSET_FOLLOWING(N) 0b00010000000 + N
#define FOLLOWING_MASK      0b00111111111

#define WRITEOUT(X) \
	do {printf("WRITEOUT 0x%02x at position %i\n", X, current_position);\
		word_ptr = &byte_data[current_position];\
		*word_ptr = X;\
		current_position += 1;\
		word_ptr = &byte_data[current_position];} while (0);

#define WRITEOUT_SHORT(X) WRITEOUT((uint8_t)(X >> 8)); WRITEOUT((uint8_t)((X << 8) >> 8));

typedef struct {
	const char *name;
	uint16_t code;
	int n_args;
	int arg_types[4];
	int arg_positions[4];
} command;

#define MAX_N_ALIASES 512

typedef struct {
	char *replacee;
	char *replacer;
} alias;

int num_aliases = 2;
alias aliases[MAX_N_ALIASES];


#define _R___R__   2, {REGISTER, REGISTER, 0, 0}, {FIRST_NIBBLE,     SECOND_NIBBLE, 0, 0}


#define NULLARY_COMMAND(name, code)                                     {name, code, 0, {0,       0,    0, 0}, {0,       0,    0, 0}}
#define UNARY___COMMAND(name, code, a1_t, a1_p)                         {name, code, 1, {a1_t,    0,    0, 0}, {a1_p,    0,    0, 0}}
#define BINARY__COMMAND(name, code, a1_t, a1_p, a2_t, a2_p)             {name, code, 2, {a1_t, a2_t,    0, 0}, {a1_p, a2_p,    0, 0}}
#define BINARY__COMMAND(name, code, a1_t, a1_p, a2_t, a2_p)             {name, code, 2, {a1_t, a2_t,    0, 0}, {a1_p, a2_p,    0, 0}}
#define TERNARY_COMMAND(name, code, a1_t, a1_p, a2_t, a2_p, a3_t, a3_p) {name, code, 2, {a1_t, a2_t, a3_t, 0}, {a1_p, a2_p, a3_p, 0}}

#define N_CMDS 120

command commands[N_CMDS] = {
	NULLARY_COMMAND("chill",0x0000),
	
	//**ALU**//
	BINARY__COMMAND("add",0x1000, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("subtract",0x1100, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("multiply",0x1200, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
	
	UNARY___COMMAND("increment",0x1300, REGISTER, FIRST_NIBBLE | SECOND_NIBBLE),
	UNARY___COMMAND("decrement",0x1400, REGISTER, FIRST_NIBBLE | SECOND_NIBBLE),
	
	BINARY__COMMAND("compare_unsigned",0x1600, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("compare",0x1700, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
	
	UNARY___COMMAND("sign",0x1800, REGISTER, FIRST_NIBBLE | SECOND_NIBBLE),
	UNARY___COMMAND("cmp0",    0x0018, REGISTER, FIRST_NIBBLE | SECOND_NIBBLE),
	
	BINARY__COMMAND("and",0x1900, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("or",0x1a00, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("xor",0x1b00, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
	BINARY__COMMAND("not",0x1c00, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
	
	UNARY___COMMAND("lshift",0x1d00, REGISTER, FIRST_NIBBLE | SECOND_NIBBLE),
	UNARY___COMMAND("rshift",0x1e00, REGISTER, FIRST_NIBBLE | SECOND_NIBBLE),
	BINARY__COMMAND("shift",0x1e00, REGISTER, FIRST_NIBBLE, REGISTER, SECOND_NIBBLE),
	
	//**stack stuff**//
	NULLARY_COMMAND("return",0x0500),
	
	//pushes
	NULLARY_COMMAND("push",0x0600),
	UNARY___COMMAND("push",0x0601,   CONSTANT,                        FOLLOWING(1)),
	UNARY___COMMAND("push",0x0602,   CONSTANT | POINTER,              FOLLOWING(1)),
	UNARY___COMMAND("push",0x0603,   REGISTER,                        FIRST_NIBBLE),
	UNARY___COMMAND("push",0x0604,   REGISTER | POINTER,              FIRST_NIBBLE),
	UNARY___COMMAND("push",0x0605,   REGISTER | POINTER | W_OFFSET,   FIRST_NIBBLE | OFFSET_FOLLOWING(1)),
	UNARY___COMMAND("push",0x0606, S_REGISTER,                        FIRST_NIBBLE),
	UNARY___COMMAND("push",0x0607, S_REGISTER | POINTER,              FIRST_NIBBLE),
	UNARY___COMMAND("push",0x0608, S_REGISTER | POINTER | W_OFFSET,   FIRST_NIBBLE | OFFSET_FOLLOWING(1)),
	
	//pops
	NULLARY_COMMAND("pop",0x0700),
	UNARY___COMMAND("pop",0x0701,   CONSTANT,                        FOLLOWING(1)),
	UNARY___COMMAND("pop",0x0702,   CONSTANT | POINTER,              FOLLOWING(1)),
	UNARY___COMMAND("pop",0x0703,   REGISTER,                        FIRST_NIBBLE),
	UNARY___COMMAND("pop",0x0704,   REGISTER | POINTER,              FIRST_NIBBLE),
	UNARY___COMMAND("pop",0x0705,   REGISTER | POINTER | W_OFFSET,   FIRST_NIBBLE | OFFSET_FOLLOWING(1)),
	UNARY___COMMAND("pop",0x0707, S_REGISTER,                        FIRST_NIBBLE),
	UNARY___COMMAND("pop",0x0707, S_REGISTER | POINTER,              FIRST_NIBBLE),
	UNARY___COMMAND("pop",0x0708, S_REGISTER | POINTER | W_OFFSET,   FIRST_NIBBLE | OFFSET_FOLLOWING(1)),
	
	
	//**setting**//
	BINARY__COMMAND("set",0x3000, CONSTANT | POINTER, FOLLOWING(1),   CONSTANT,                      FOLLOWING(2)),
	BINARY__COMMAND("set",0x3001, CONSTANT | POINTER, FOLLOWING(1),   CONSTANT | POINTER,            FOLLOWING(2)),
	
	BINARY__COMMAND("set",0x3100, CONSTANT | POINTER, FOLLOWING(1),   REGISTER,                      FIRST_NIBBLE),
	BINARY__COMMAND("set",0x3101, CONSTANT | POINTER, FOLLOWING(1),   REGISTER | POINTER,            FIRST_NIBBLE),
	BINARY__COMMAND("set",0x3102, CONSTANT | POINTER, FOLLOWING(1),   REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | FOLLOWING(2)),
	
	BINARY__COMMAND("set",0x3103, CONSTANT | POINTER, FOLLOWING(1), S_REGISTER,                      FIRST_NIBBLE),
	BINARY__COMMAND("set",0x3104, CONSTANT | POINTER, FOLLOWING(1), S_REGISTER | POINTER,            FIRST_NIBBLE),
	BINARY__COMMAND("set",0x3105, CONSTANT | POINTER, FOLLOWING(1), S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | FOLLOWING(2)),
	
	
	BINARY__COMMAND("set",0x3300, REGISTER, FIRST_NIBBLE, CONSTANT,           FOLLOWING(1)),
	BINARY__COMMAND("set",0x3400, REGISTER, FIRST_NIBBLE, CONSTANT | POINTER, FOLLOWING(1)),
	
	BINARY__COMMAND("set",0x3500, REGISTER, FIRST_NIBBLE, REGISTER,                      SECOND_NIBBLE),
	BINARY__COMMAND("set",0x3600, REGISTER, FIRST_NIBBLE, REGISTER | POINTER,            SECOND_NIBBLE),
	BINARY__COMMAND("set",0x3700, REGISTER, FIRST_NIBBLE, REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
	
	BINARY__COMMAND("set",0x3800, REGISTER, FIRST_NIBBLE, S_REGISTER,                      SECOND_NIBBLE),
	BINARY__COMMAND("set",0x3900, REGISTER, FIRST_NIBBLE, S_REGISTER | POINTER,            SECOND_NIBBLE),
	BINARY__COMMAND("set",0x3a00, REGISTER, FIRST_NIBBLE, S_REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
	
	BINARY__COMMAND("set",0x3b00, REGISTER | POINTER, FIRST_NIBBLE, CONSTANT,           FOLLOWING(1)),
	BINARY__COMMAND("set",0x3c00, REGISTER | POINTER, FIRST_NIBBLE, CONSTANT | POINTER, FOLLOWING(1)),
	
	BINARY__COMMAND("set",0x3d00, REGISTER | POINTER, FIRST_NIBBLE, REGISTER,           SECOND_NIBBLE),
	BINARY__COMMAND("set",0x3e00, REGISTER | POINTER, FIRST_NIBBLE, REGISTER | POINTER, SECOND_NIBBLE),
	BINARY__COMMAND("set",0x3f00, REGISTER | POINTER, FIRST_NIBBLE, REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
	
	BINARY__COMMAND("set",0x4000, REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER,           SECOND_NIBBLE),
	BINARY__COMMAND("set",0x4100, REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER | POINTER, SECOND_NIBBLE),
	BINARY__COMMAND("set",0x4200, REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
	
	
	BINARY__COMMAND("set",0x4300, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), CONSTANT,           FOLLOWING(2)),
	BINARY__COMMAND("set",0x4301, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), CONSTANT | POINTER, FOLLOWING(2)),
	
	BINARY__COMMAND("set",0x4400, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), REGISTER,                      SECOND_NIBBLE),
	BINARY__COMMAND("set",0x4500, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), REGISTER | POINTER,            SECOND_NIBBLE),
	BINARY__COMMAND("set",0x4600, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(2)),
	
	BINARY__COMMAND("set",0x4700, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), S_REGISTER,                      SECOND_NIBBLE),
	BINARY__COMMAND("set",0x4800, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), S_REGISTER | POINTER,            SECOND_NIBBLE),
	BINARY__COMMAND("set",0x4900, REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), S_REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(2)),
	
	BINARY__COMMAND("set",0x4a00, S_REGISTER, FIRST_NIBBLE, CONSTANT,           FOLLOWING(1)),
	BINARY__COMMAND("set",0x4a01, S_REGISTER, FIRST_NIBBLE, CONSTANT | POINTER, FOLLOWING(1)),
	
	BINARY__COMMAND("set",0x4b00, S_REGISTER, FIRST_NIBBLE, REGISTER,                      SECOND_NIBBLE),
	BINARY__COMMAND("set",0x4c00, S_REGISTER, FIRST_NIBBLE, REGISTER | POINTER,            SECOND_NIBBLE),
	BINARY__COMMAND("set",0x4d00, S_REGISTER, FIRST_NIBBLE, REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
	
	BINARY__COMMAND("set",0x4e00, S_REGISTER, FIRST_NIBBLE, S_REGISTER,                      SECOND_NIBBLE),
	BINARY__COMMAND("set",0x4f00, S_REGISTER, FIRST_NIBBLE, S_REGISTER | POINTER,            SECOND_NIBBLE),
	BINARY__COMMAND("set",0x5000, S_REGISTER, FIRST_NIBBLE, S_REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
	
	BINARY__COMMAND("set",0x5100, S_REGISTER | POINTER, FIRST_NIBBLE, CONSTANT,           FOLLOWING(1)),
	BINARY__COMMAND("set",0x5101, S_REGISTER | POINTER, FIRST_NIBBLE, CONSTANT | POINTER, FOLLOWING(1)),
	
	BINARY__COMMAND("set",0x5200, S_REGISTER | POINTER, FIRST_NIBBLE, REGISTER,           SECOND_NIBBLE),
	BINARY__COMMAND("set",0x5300, S_REGISTER | POINTER, FIRST_NIBBLE, REGISTER | POINTER, SECOND_NIBBLE),
	BINARY__COMMAND("set",0x5400, S_REGISTER | POINTER, FIRST_NIBBLE, REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
	
	BINARY__COMMAND("set",0x5500, S_REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER,           SECOND_NIBBLE),
	BINARY__COMMAND("set",0x5600, S_REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER | POINTER, SECOND_NIBBLE),
	BINARY__COMMAND("set",0x5700, S_REGISTER | POINTER, FIRST_NIBBLE, S_REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(1)),
	
	
	BINARY__COMMAND("set",0x5800, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), CONSTANT,           FOLLOWING(2)),
	BINARY__COMMAND("set",0x5900, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), CONSTANT | POINTER, FOLLOWING(2)),
	
	BINARY__COMMAND("set",0x5a00, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), REGISTER,                      SECOND_NIBBLE),
	BINARY__COMMAND("set",0x5b00, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), REGISTER | POINTER,            SECOND_NIBBLE),
	BINARY__COMMAND("set",0x5c00, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(2)),
	
	BINARY__COMMAND("set",0x5d00, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), S_REGISTER,                      SECOND_NIBBLE),
	BINARY__COMMAND("set",0x5e00, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), S_REGISTER | POINTER,            SECOND_NIBBLE),
	BINARY__COMMAND("set",0x5f00, S_REGISTER | POINTER | W_OFFSET, FIRST_NIBBLE | OFFSET_FOLLOWING(1), S_REGISTER | POINTER | W_OFFSET, SECOND_NIBBLE | OFFSET_FOLLOWING(2)),
};

typedef struct
{
	char label[MAX_LABEL_LENGTH];
	uint8_t position;
	int line_number;
} GotoInstance;

typedef struct {
    int line_number;
    char **tokens;
    int token_count;
} LineData;

typedef struct Node {
    LineData *data;
    struct Node *next;
} Node;

void free_line_data(LineData *line_data) {
    for (int i = 0; i < line_data->token_count; i++) {
        free(line_data->tokens[i]);
    }
    free(line_data->tokens);
    free(line_data);
}

void free_list(Node *head) {
    Node *temp;
    while (head) {
        temp = head;
        head = head->next;
        free_line_data(temp->data);
        free(temp);
    }
}

// Function to tokenize a line into words
LineData *tokenize_line(char *line, int line_number) {
    LineData *line_data = malloc(sizeof(LineData));
    if (!line_data) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    line_data->line_number = line_number;
    line_data->tokens = malloc(MAX_TOKENS * sizeof(char *));
    line_data->token_count = 0;

    char *token = strtok(line, " \t\n,;");
    while (token && line_data->token_count < MAX_TOKENS) {
        line_data->tokens[line_data->token_count] = strdup(token);
        line_data->token_count++;
        token = strtok(NULL, " \t\n,;");
    }
    return line_data;
}

// Function to check if the string is a number (constant)
int is_number(const char* str) {
    if (str[0] == '-') {
        str++;  // skip the minus sign
    }
    if (str[0] == '0') {
        if (str[1] == 'x' || str[1] == 'X') { // hexadecimal
            str += 2;
            while (*str) {
                if (!isxdigit(*str)) return 0;
                str++;
            }
            return 1;
        } else if (str[1] == 'b' || str[1] == 'B') { // binary
            str += 2;
            while (*str) {
                if (*str != '0' && *str != '1') return 0;
                str++;
            }
            return 1;
        }
    }
    while (*str) {
        if (!isdigit(*str)) return 0;
        str++;
    }
    return 1;
}

int is_dnumber(const char *str)
{
	if (str[0] == '-') {
		str = &str[1];
	}
	for (int i = 0; i < strlen(str); i++)
	{
		if (!(str[i] >= '0' && str[i] <= '9'))
		{
			return 0;
		}
	}
	return 1;
}

int is_hnumber(const char *str)
{
	for (int i = 0; i < strlen(str); i++)
	{
		if (!(str[i] >= '0' && str[i] <= '9') && !(str[i] >= 'a' && str[i] <= 'f'))
		{
			return 0;
		}
	}
	return 1;
}

int is_bnumber(const char *str)
{
	for (int i = 0; i < strlen(str); i++)
	{
		if (!(str[i] >= '0' && str[i] <= '1'))
		{
			return 0;
		}
	}
	return 1;
}

long parse_number(const char* str) {
    if (str[0] == '-') return -parse_number(&str[1]);
    
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

// Placeholder function for processing the line data
LineData *process_line_data(LineData *input) {
    // User-defined function should process the line data
    // For now, returning the input data unchanged
    return input;
}

typedef struct {
	int type;
	uint8_t value;
	uint8_t offset;
} parameter;

parameter parse_parameter(const char *input)
{
    parameter result = {0, 0, 0}; // Initialize result with default values
    char tmp;
    char* plus = NULL;
    const char *inside = NULL;
    const char* close_paren = NULL;
    
    size_t len, leng;// = close_paren - inside;
	char* sub_exp;// = strndup(inside, len);
	parameter sub_result;// = parse_string(sub_expr);
  
    
    //printf("Parsing parameter ``%s\"\n", input);
    
    if (input[0] == '*') {
		//printf("It's a pointer, ");
		if (input[1] == '(') {
			//printf("parenthesised, with inner expression");
			inside = &input[2];
			close_paren = strchr(inside, ')');
			
			if (close_paren == NULL) {
				//printf("... but not closing paren :(\n");
				result.type = INVALID;
				return result;
			}
			if (close_paren[1] != 0) {
				//printf("... but does not end after its closing paren :(\n");
				
				result.type = INVALID;
				return result;
			}
			
			len = close_paren - inside;
			sub_exp = strndup(inside, len);
			
			//printf(" \"%s\", ", sub_exp);
			
			plus = &sub_exp[strcspn(sub_exp, "+-")];
			
			if (plus != NULL) {
				if (plus[0] == '+') {sub_result = parse_parameter(&plus[1]);}
				else {sub_result = parse_parameter(plus);}
				
				
				if (sub_result.type != CONSTANT) {
					//printf("... but it contains an offset which is not a constant :(\n");
				
					result.type = INVALID;
					return result;
				}
				if (sub_result.offset != 0) {
					//printf("... but it contains a double-offset :(\n");
					
					result.type = INVALID;
					return result;
				}
				
				result.offset = sub_result.value;
				//printf(" which has offset %i\n", result.offset);
				*plus = 0;
			}
			
		} else {
			//printf("non-parenthesised, ");
			inside = &input[1];
			len = strlen(inside);
			sub_exp = strndup(inside, len);
		}
		
		//printf("with sub-expression %s\n", sub_exp);
		sub_result = parse_parameter(sub_exp);
		free(sub_exp);
		
		if (input[1] != '(' && sub_result.offset != 0) {
			//printf("... which has an offset, which is obviously not valid without parenthesisation >:/\n", sub_exp);
			result.type = INVALID;
			return result;
		}
		
		result.type  = sub_result.type | POINTER;
		result.value = sub_result.value;
    }
    else {
		//printf("It's not a pointer... ");
		if (input[0] == 's' && input[1] == 'r' && is_number(&input[2])) {
			// "srN" form
			result.type = S_REGISTER;
			result.value = parse_number(&input[2]);
			//printf("it's a special register !\n");
		} else if (input[0] == 'r' && is_number(&input[1])) {
			// "rN" form
			//printf("it's a register !\n");
			result.type = REGISTER;
			result.value = parse_number(&input[1]);
		} else if (is_number(input)) {
			result.type = CONSTANT;
			result.value = parse_number(input);
			//printf("it's a number !\n");
		} else {
			for (int i = 0; i < num_aliases; i++) {
				if (strcmp(input, aliases[i].replacee) == 0) {
					printf("Ah! an alias! I will parse %s in place of %s\n", aliases[i].replacer, input);
					return parse_parameter(aliases[i].replacer);
				}
			}
			//printf("idk what this is...\n");
			result.type = INVALID;
		}
    }
    
    if (result.offset != 0) { result.type = result.type | W_OFFSET; }
    
    return result;
}

#define PLAIN_GOTO         0x0100
#define FUNCTION_CALL      0x0400

#define GOTO_C_ZERO        0x0200
#define GOTO_C_NONZERO     0x0300
#define GOTO_C_NEGATIVE    0x0201
#define GOTO_C_NONNEGATIVE 0x0301
#define GOTO_C_POSITIVE    0x0202
#define GOTO_C_NONPOSITIVE 0x0302


uint16_t encode_goto(LineData line) {
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
			fprintf(stderr, "Error: invalid ``goto\" code statement ``%s\" on line %i.\n", line.tokens[0], line.line_number);
			exit(EXIT_FAILURE);
		}
		
		if (line.token_count < 3) {
			fprintf(stderr, "Error: too few arguments for ``%s\" on line %i.\n", line.tokens[0], line.line_number);
			exit(EXIT_FAILURE);
		}
		
		parameter param = parse_parameter(line.tokens[1]);
		
		if (param.type != REGISTER || param.offset != 0) {
			if (line.token_count < 3) {
				fprintf(stderr, "Error: conditional ``goto\"-type statements only take registers as arguments, but ``%s\" on line %i is not a register.\n", line.tokens[1], line.line_number);
				exit(EXIT_FAILURE);
			}
		}
		
		code += 0x0010 * param.value;
	}
	
	printf("goto code 0x%04x\n", code);
	
	return code;
}

// Function to process file input and generate output
void process_file(const char *input_path, const char *output_path) {
    FILE *input_file = fopen(input_path, "r");
    if (!input_file) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    Node *head = NULL, *tail = NULL;
    char line[MAX_LINE_LENGTH];
    int line_number = 1;

    while (fgets(line, sizeof(line), input_file)) {
        // Trim leading and trailing whitespace
        char *trimmed_line = line;
        while (isspace((unsigned char)*trimmed_line)) trimmed_line++;
        if (*trimmed_line == '\0') {
            line_number++;
            continue;
        }

        LineData *line_data = tokenize_line(trimmed_line, line_number);

        Node *new_node = malloc(sizeof(Node));
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
    FILE *output_file = fopen(output_path, "wb");
    if (!output_file) {
        perror("Error opening output file");
        free_list(head);
        exit(EXIT_FAILURE);
    }

	uint8_t *byte_data = malloc(BINSIZE);
	for (int i = 0; i<BINSIZE; i++)
	{
		byte_data[i] = 0;
	}
	
	int current_position = 0;
	uint8_t *word_ptr;
	
	int n_labels = 0;
	int n_gotos = 0;
	
	char labels[line_number][MAX_LABEL_LENGTH];
	uint8_t label_positions[line_number];
	GotoInstance goto_instances[line_number];
	
	for (int i = 0; i<line_number; i++)
	{
		for (int j = 0; j < MAX_LABEL_LENGTH; j++)
		{
			labels[j][i] = 0;
			goto_instances[i].label[j] = 0;
		}
	}
	
    Node *current = head;
    int length;
    uint16_t word;
    uint32_t addr, c;
    int reg1, reg2;
    parameter pms[10];
    int found = 0;
    int index;
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
	
	for (int i = num_aliases; i < MAX_N_ALIASES; i++) {
		aliases[i].replacee = NULL;
		aliases[i].replacer = NULL;
	}
    
    while (current) {
		//printf("Line %i\n", current->data->line_number);
		length = strlen(current->data->tokens[0]);
		word_ptr = (uint8_t*)(&byte_data[2 * current_position]);
		
		printf("Assembling line %d, which has %d tokens, which are\n    ``%s\"", current->data->line_number, current->data->token_count, current->data->tokens[0]);
		
		for (int i = 1; i < current->data->token_count; ++i) {
			printf(", ``%s\"", current->data->tokens[i]);
		}
		putchar('\n');
		
		if (current->data->tokens[0][0] == '%') {
			//ignore commented lines
		} else if (strcmp(current->data->tokens[0], "alias") == 0) {
			if (current->data->token_count != 3) {
				fprintf(stderr, "Error: syntax error defining alias on line %i; usage is alias [replacee] [replacer].\n", current->data->line_number);
				exit(EXIT_FAILURE);
			}
			
			aliases[num_aliases].replacee = malloc(strlen(current->data->tokens[1]) + 1);
			strcpy(aliases[num_aliases].replacee, current->data->tokens[1]);
			
			aliases[num_aliases].replacer = malloc(strlen(current->data->tokens[2]) + 1);
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
							aliases[j].replacee = aliases[j+1].replacee;
							aliases[j].replacer = aliases[j+1].replacer;
						}
					}
				}
			}
		}
        else if (current->data->token_count == 1 && current->data->tokens[0][length-1] == ':')
        {
			strncpy(labels[n_labels], current->data->tokens[0], length-1);
			labels[n_labels][length - 1] = 0;
			label_positions[n_labels] = current_position;
			n_labels += 1;
		}
		else if (strncmp(current->data->tokens[0], "goto", 4) == 0 || strcmp(current->data->tokens[0], "call") == 0)
		{
			code = encode_goto(*current->data);
			
			if (code == INVALID) {
				fprintf(stderr, "Error: nonsense ``goto\"-type statement `on line %i.\n", current->data->tokens[0], current->data->line_number);
					exit(EXIT_FAILURE);
			}
			
			if (code == PLAIN_GOTO || code == FUNCTION_CALL) {
				printf("h\n");
				strncpy(goto_instances[n_gotos].label, current->data->tokens[1], MAX_LABEL_LENGTH);
			} else {
				strncpy(goto_instances[n_gotos].label, current->data->tokens[2], MAX_LABEL_LENGTH);
			}
			
			printf("\n");
			printf("\n");
			
			
			goto_instances[n_gotos].position = current_position;
			goto_instances[n_gotos].line_number = current->data->line_number;
			n_gotos += 1;
			
			WRITEOUT_SHORT(code);
			WRITEOUT_SHORT(0);
			WRITEOUT_SHORT(0);
		}
		else
		{
			for (int i = 0; i < num_aliases; i++) {
				for (int j = 0; j < current->data->token_count; j++) {
					if (strcmp(current->data->tokens[j], aliases[i].replacee) == 0) {
						current->data->tokens[j] = realloc(current->data->tokens[j], strlen(aliases[i].replacer) + 1);
						strcpy(current->data->tokens[j], aliases[i].replacer);
					}
				}
			}
			
			argc = current->data->token_count - 1;
			for (int i = 0; i < argc; ++i) {
				pms[i] = parse_parameter(current->data->tokens[i + 1]);
				printf("argument ``%s\", type %i\n", current->data->tokens[i + 1], pms[i].type);
				if (pms[i].type == INVALID) {
					fprintf(stderr, "Error: invalid argument ``%s\" for command ``%s\" on line %i.\n", current->data->tokens[i+1], current->data->tokens[0], current->data->line_number);
					exit(EXIT_FAILURE);
				}
			}
			
			found = 0;
			for (index = 0; !found && index < N_CMDS; index++) {
				if (strcmp(current->data->tokens[0], commands[index].name) == 0 &&
					current->data->token_count - 1 == commands[index].n_args)
				{
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
				fprintf(stderr, "Error: no command ``%s\" with the matching type signature (%i", current->data->tokens[0], pms[0].type);
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
						code += pms[j].value * 0x0010;
					}
					if (commands[index].arg_positions[j] & SECOND_NIBBLE) {
						code += pms[j].value * 0x0001;
					}
				}
				
				WRITEOUT_SHORT(code);
				
				printf("Command %s, %i arguments:\n",commands[index].name, argc);
				for (int j = 0; j < argc; j++) {
					printf("    type %i in position 0b%16b\n", commands[index].arg_types[j], commands[index].arg_positions[j]);
				}
				
				for (int k = 1; k < argc + 1; k++) {
					for (int j = 0; j < argc; j++) {
						printf("Check if argument %i follows in position %i, by comparing 0b%16b with 0b%16b and 0b%16b\n", j, k, commands[index].arg_positions[j], FOLLOWING(k), OFFSET_FOLLOWING(k));
						if ((commands[index].arg_positions[j] & FOLLOWING_MASK) == FOLLOWING(k)) {
							printf("it does.\n", j, k);
							WRITEOUT_SHORT(pms[j].value);
						}
						else if ((commands[index].arg_positions[j] & FOLLOWING_MASK) == OFFSET_FOLLOWING(k)) {
							
							printf("its offset does.\n", j, k);
							WRITEOUT_SHORT(pms[j].offset);
						}
					}
				}
			}
		}
		
		current = current->next;
    }
    
    int label_index;
    
    for (int i = 0; i < n_gotos; i++)
    {
		found = 0;
		for (int j = 0; j < n_labels; j++)
		{
			if (strcmp(goto_instances[i].label, labels[j]) == 0)
			{
				label_index = j;
				found = 1;
				break;
			}
		}
		
		printf("Goto at position %i\n", goto_instances[i].position);
		
		if (found == 0)
		{
			fprintf(stderr, "Error: label ``%s\'\', referenced on line %i, was never defined.\n", goto_instances[i].label, goto_instances[i].line_number);
			exit(EXIT_FAILURE);
		}
		else
		{
			printf("Found label %s! Its position is %i\n", labels[label_index], label_positions[label_index]);
			current_position = goto_instances[i].position + 2;
			WRITEOUT_SHORT(0x0000);
			WRITEOUT_SHORT(label_positions[label_index]);
		}
	}
    
	fwrite(byte_data, 1, BINSIZE, output_file);
    fclose(output_file);
    free_list(head);
    free(byte_data);
}

// Main function to handle input arguments
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input file> [-o <output file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *input_path  = NULL;
    char *output_path = NULL;
    
    //~ for (int i = 0; i < N_NULLARIES; i++) { strcpy(nullaries[i].name, nullary_names[i]); }
	//~ for (int i = 0; i < N_UNARIES;   i++) { strcpy(  unaries[i].name, unary_names  [i]); }
	//~ for (int i = 0; i < N_BINARIES;  i++) { strcpy( binaries[i].name, binary_names [i]); }
	//~ for (int i = 0; i < N_TERNARIES; i++) { strcpy(ternaries[i].name, ternary_names[i]); }
	
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
