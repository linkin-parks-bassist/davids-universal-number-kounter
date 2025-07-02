#include <stdio.h>
#include <string.h>
#include "dunkasm.h"

const dasm_error_opt wept_options[N_WEPT_OPTS] = {
	{"doubleinclude", 		DOUBLE_INCLUDE},
	{"realias", 			ALIAS_TAKEN_ERR},
	{"realias-built-in", 	ALIAS_BUILT_IN_ERR}
};

const char *error_colour = "\e[01;31m";
const char *warning_colour = "\e[01;32m";
const char *highlight_colour = "\e[01;36m";
const char *reset_colour = "\e[0m";

const char *terminate_str = "Fatal error";
const char *error_str = "Error";
const char *warning_str = "Warning";

void init_wept_config(wept_config *opt)
{
	if (!opt)
		return;
	
	opt->levels[MEMORY_FAILURE]				= WEPT_TERMINATE;
	opt->levels[IO_ERROR] 					= WEPT_TERMINATE;
	opt->levels[BAD_COMMAND_ARGS] 			= WEPT_ERROR;
	opt->levels[SYNTAX_ERROR] 				= WEPT_ERROR;
	opt->levels[BAD_INSTRUCTION_ERROR] 		= WEPT_ERROR;
	opt->levels[UNDEFINED_REFERENCE_ERROR] 	= WEPT_ERROR;
	opt->levels[MALFORMED_PARAM_ERROR] 		= WEPT_ERROR;
	opt->levels[PARAM_TYPE_ERROR] 			= WEPT_ERROR;
	opt->levels[INCLUDE_ERROR] 				= WEPT_ERROR;
	opt->levels[DOUBLE_INCLUDE] 			= WEPT_WARNING;
	opt->levels[ALIAS_RESERVED_ERR] 		= WEPT_ERROR;
	opt->levels[ALIAS_BUILT_IN_ERR] 		= WEPT_ERROR;
	opt->levels[ALIAS_TAKEN_ERR] 			= WEPT_ERROR;
}

void print_err_line(dasm_file *file, dasm_line line, int bad_token, const char *colour)
{
	char buf[1024];
		
	int bad_token_start = 0;
	int bad_token_end = 0;
	int bad_token_middle;
	int position = 0;
	
	if (file)
		sprintf(buf, "    (%s:%d): ", file->given_path, line.line_number);
	else
		sprintf(buf, "    Line ");
	
	fprintf(stderr, buf);
	position = strlen(buf);
	
	for (int i = 0; i < line.n_tokens; i++)
	{
		if (i == bad_token || (bad_token == -2 && i > 0))
		{
			fprintf(stderr, colour);
			#ifdef UNDERLINE_BAD_TOKENS
			fprintf(stderr, "\e[4m");
			#endif
			
			if (bad_token != -2 || i == 1)
				bad_token_start = position;
		}
		fprintf(stderr, "%s", line.tokens[i]);
		
		position += strlen(line.tokens[i]) + 1;
		
		if (i == bad_token || (bad_token == -2 && i > 0))
		{
			fprintf(stderr, reset_colour);
			bad_token_end = position - 1;
		}
		
		if (i != line.n_tokens - 1)
			fputc(' ', stderr);
	}
	
	fputc('\n', stderr);
	
	if (bad_token == -1)
	{
		bad_token_start = position + 1;
		bad_token_end   = position + 1;
	}
	
	if (bad_token_start != 0)
	{
		bad_token_middle = (bad_token_start + bad_token_end) / 2;
		for (int i = 0; i < bad_token_middle; i++)
			fputc(' ', stderr);
		fprintf(stderr, "^\n");
	}
	
	fputc('\n', stderr);
}

void print_error(dasm_error err, wept_config *opt)
{
	if (!opt)
		return;
	
	const char *err_str;
	const char *colour;
	
	switch (opt->levels[err.error_code])
	{
		case WEPT_WARNING:
			colour = warning_colour;
			err_str = warning_str;
			break;
		
		case WEPT_ERROR:
			colour = error_colour;
			err_str = error_str;
			break;
		
		case WEPT_PERMIT:
			return;
		
		case WEPT_TERMINATE:
			colour = error_colour;
			err_str = terminate_str;
			break;
	}
	
	fprintf(stderr, "%s%s (code %d):\e[0m %s\n", colour, err_str, err.error_code & ~CODE_ERROR, err.msg);
	
	if (err.error_code & CODE_ERROR)
	{
		print_err_line(err.file, err.line, err.bad_token, colour);
		
		if (err.error_code == DOUBLE_INCLUDE && err.additional_data)
		{
			fprintf(stderr, "Note: previously included here\n");
			print_err_line(((dasm_inclusion*)err.additional_data)->parent, ((dasm_inclusion*)err.additional_data)->line, ((dasm_inclusion*)err.additional_data)->token, highlight_colour);
		}
	}
}
