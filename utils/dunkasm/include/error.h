#ifndef DUNKASM_ERROR_H
#define DUNKASM_ERROR_H

#define CODE_ERROR 0b100000000
#define DEFAULT_ERROR_TOLERANCE 5

#define MEMORY_ERROR 				0
#define IO_ERROR					1
#define SYNTAX_ERROR 				2 | CODE_ERROR
#define BAD_INSTRUCTION_ERROR		3 | CODE_ERROR
#define UNDEFINED_REFERENCE_ERROR 	4 | CODE_ERROR
#define MALFORMED_PARAM_ERROR 		5 | CODE_ERROR
#define PARAM_TYPE_ERROR 			6 | CODE_ERROR
#define INCLUDE_ERROR 				7 | CODE_ERROR

struct dasm_error {
	dasm_file *file;
	dasm_line line;
	int bad_token;
	int error_code;
	char msg[2048];
};

void print_error(dasm_error err);

#endif
