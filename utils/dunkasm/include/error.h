#ifndef DUNKASM_ERROR_H
#define DUNKASM_ERROR_H

#define CODE_ERROR 0b100000000
#define DEFAULT_ERROR_TOLERANCE 5

#define N_ERR_CODES 128

#define MEMORY_ERROR 				 0
#define IO_ERROR					 1
#define BAD_COMMAND_ARGS			 2
#define SYNTAX_ERROR 				 3 | CODE_ERROR
#define BAD_INSTRUCTION_ERROR		 4 | CODE_ERROR
#define UNDEFINED_REFERENCE_ERROR 	 5 | CODE_ERROR
#define MALFORMED_PARAM_ERROR 		 6 | CODE_ERROR
#define PARAM_TYPE_ERROR 			 7 | CODE_ERROR
#define INCLUDE_ERROR 				 8 | CODE_ERROR
#define DOUBLE_INCLUDE 				 9 | CODE_ERROR
#define ALIAS_RESERVED_ERR			10 | CODE_ERROR
#define ALIAS_BUILT_IN_ERR			11 | CODE_ERROR
#define ALIAS_TAKEN_ERR				12 | CODE_ERROR

#define WEPT_NO_ERROR	0
#define WEPT_WARNING 	1
#define WEPT_ERROR	 	2
#define WEPT_PERMIT	 	3
#define WEPT_TERMINATE 	4

struct dasm_error
{
	dasm_file *file;
	dasm_line line;
	int bad_token;
	int error_code;
	char msg[2048];
	
	void *additional_data;
};

typedef struct
{
	char *name;
	int index;
} dasm_error_opt;

typedef struct
{
	int levels[N_ERR_CODES];
} wept_config;

void init_wept_config(wept_config *opt);

void print_error(dasm_error err, wept_config *opt);

#define N_WEPT_OPTS 3
extern const dasm_error_opt wept_options[N_WEPT_OPTS];

#endif
