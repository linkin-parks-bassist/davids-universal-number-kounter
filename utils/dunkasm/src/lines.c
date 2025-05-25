#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dunkasm.h"

IMPLEMENT_LINKED_LIST(dasm_line);

dasm_line new_dasm_line(const char *src, unsigned int line_number)
{
	if (src == NULL) {
		fprintf(stderr, "Error: NULL pointer passed to function `new_dasm_line'.\n");
		exit(EXIT_FAILURE);
	}
	
	dasm_line result;
	
	result.line_number = line_number;
	result.raw_line = strdup(src);
	result.tokens = tokenize_string(result.raw_line, &result.n_tokens);
	
	return result;
}

void dasm_line_destructor(dasm_line line)
{
	for (int i = 0; i < line.n_tokens; i++)
	{
		free(line.tokens[i]);
		line.tokens[i] = NULL;
	}
	free(line.tokens);
	line.tokens = NULL;
	free(line.raw_line);
	line.raw_line = NULL;
}

void strip_comments(dasm_line *line)
{
	if (line == NULL)
		return;
	
	for (int i = 0; i < line->n_tokens; ++i)
	{
		if (line->tokens[i][0] == '%') {
			for (int j = i + 1; j < line->n_tokens; j++)
			{
				free(line->tokens[j]);
			}
			line->n_tokens = i;
			break;
		}
	}
}

#define ERROR_OUT \
	print_error(err);\
	cxt->n_errors++;\
	return FAILURE

int process_line(dasm_line line, dasm_file *file, dasm_context *cxt, int flags)
{
	if (!valid_dasm_file(file) || !valid_dasm_context(cxt))
		return BAD_ARGUMENTS;
	if (line.n_tokens == 0)
		return SUCCESS;
	
	if (cxt->flags & VERBOSE) {
		printf("Assembling %s:%d, whose tokens are:\n\t", file->given_path, line.line_number);
		
		for (int i = 0; i < line.n_tokens; i++) {
			printf("%s", line.tokens[i]);
			if (i == line.n_tokens - 1)
				printf("\n");
			else
				printf(", ");
		}
	}
	
	char temp_string[strlen(line.raw_line)];
	dasm_alias temp_alias;
	
	dasm_error err;
	err.line = line;
	err.file = file;
	
	int first_token_length = strlen(line.tokens[0]);
	int buffer_pos = file->text.position;
	int n_errors = 0;
	
	if (strcmp(line.tokens[0], "alias") == 0)
	{
		if (line.n_tokens > 3) {
			err.error_code = SYNTAX_ERROR;
			err.bad_token = 3;
			
			strcpy(err.msg, "Syntax error: too many arguments for \"alias\"; usage is alias [replacee] [replacer].");
			
			ERROR_OUT;
		}

		add_alias_to_context(cxt, line.tokens[1], line.tokens[2], file);
	} 
	else if (strcmp(line.tokens[0], "dealias") == 0)
	{
		if (line.n_tokens < 2)
		{
			err.error_code = SYNTAX_ERROR;
			err.bad_token = -1;
			strcpy(err.msg, "Syntax error: no aliases provided to dealias");
			
			ERROR_OUT;
		}
		if (strcmp(line.tokens[1], "all") == 0)
		{
			clear_nondefault_aliases_with_parent(cxt, file);
		}
		else 
		{
			for (int k = 1; k < line.n_tokens; k++)
			{
				switch (remove_alias_from_context(cxt, line.tokens[k]))
				{
					case BAD_ARGUMENTS:
						err.error_code = MEMORY_ERROR;
						sprintf(err.msg, "A stray NULL pointer was encountered.");
						
						ERROR_OUT;
				}
			}
		}
	}
	else if (strcmp(line.tokens[0], "include") == 0 || strcmp(line.tokens[0], "include_first") == 0)
	{
		for (int i = 1; i < line.n_tokens; i++)
		{
			if (!is_string(line.tokens[i]))
			{
				err.error_code = SYNTAX_ERROR;
				err.bad_token = i;
				strcpy(err.msg, "Syntax error: non-string literal encountered in \"include\" directive.");
				
				ERROR_OUT;
			}
			strcpy(temp_string, &line.tokens[i][1]);
			
			temp_string[strlen(temp_string) - 1] = 0;
			
			if (cxt->flags & VERBOSE)
				printf("Including file \"%s\"\n", temp_string);
			
			if (!file_accessible(cxt, temp_string))
			{
				err.error_code = INCLUDE_ERROR;
				err.bad_token = i;
				sprintf(err.msg, "Included file \"%s\" not found.", temp_string);
				
				ERROR_OUT;
			}
			else
			{
				switch (is_file_already_in_context(cxt, temp_string)) {
				case 1:
					// Attempted double file inclusion; simply skip this silently
					if (cxt->flags & VERBOSE)
						printf("File is already in list; skipping...\n");
					break;
				
				case BAD_ARGUMENTS:
					err.error_code = MEMORY_ERROR;
					sprintf(err.msg, "A stray NULL pointer was encountered.");
					
					ERROR_OUT;
					break;
				
				default:
					int inc_flags = INCLUDED_FILE | (((flags & MAIN_FILE) && strcmp(line.tokens[0], "include_first") == 0) ? INCLUDE_FIRST : 0);
					process_file(temp_string, cxt, inc_flags);
					break;
				}
			}
		}
	}
	else if (line.n_tokens == 1 && line.tokens[0][first_token_length - 1] == ':')
	{
		strcpy(temp_string, line.tokens[0]);
		
		temp_string[first_token_length - 1] = 0;
		add_label_to_context(cxt, file, temp_string, file->text.position, line.line_number, CODE_LABEL);
	}
	else if (is_an_instruction(line.tokens[0]))
	{
		int argc = line.n_tokens - 1;
		
		
		if (argc > MAX_PARAMS)
		{
			err.error_code = SYNTAX_ERROR;
			err.bad_token = MAX_PARAMS + 1;
			sprintf(err.msg, "Syntax error: too many parameters; expected at most %d, but got %d.");
			
			ERROR_OUT;
		}
		
		dasm_alias_linked_list *current;
		
		/* Copy the tokens over to a new array */
		char *tokens[line.n_tokens];
		
		for (int i = 0; i < line.n_tokens; i++)
			tokens[i] = line.tokens[i];
		
		/* Swap out any alias instances for their replacers */
		for (int j = 0; j < line.n_tokens; j++)
		{
			current = cxt->aliases;
			
			while (current != NULL)
			{
				if (current->data.replacee == NULL || current->data.replacer == NULL)
				{
					err.error_code = MEMORY_ERROR;
					sprintf(err.msg, "A stray NULL pointer was encountered.");
					
					ERROR_OUT;
				}
				if (strcmp(current->data.replacee, tokens[j]) == 0) {
					tokens[j] = current->data.replacee;
				}
				
				current = current->next;
			}
		}
		
		
		parameter params[argc];
		
		for (int i = 0; i < argc; ++i)
		{
			params[i] = parse_parameter(cxt, tokens[i + 1]);
			
			if (params[i].type == LABEL_P)
			{
				handle_label_parameter(cxt, file, &params[i], tokens[i + 1], line.line_number);
			}
			else if (params[i].type == STRING_P)
			{
				handle_string_parameter(cxt, file, &params[i], tokens[i + 1], line.line_number);
			}
			if (params[i].type == INVALID)
			{
				err.error_code = MALFORMED_PARAM_ERROR;
				err.bad_token = i + 1;
				sprintf(err.msg, "Syntax error: malformed parameter.");
				
				ERROR_OUT;
			}
		}
		
		int found = 0;
		int index;
			
		for (index = 0; !found && index < N_INSTR; index++)
		{
			if (strcmp(tokens[0], dunk_instrs[index].name) == 0 &&
					line.n_tokens - 1 == dunk_instrs[index].n_parameters)
			{
				found = 1;
				
				for (int j = 0; j < argc; j++)
				{
					if (params[j].type != dunk_instrs[index].parameter_types[j])
					{
						found = 0;
						break;
					}
				}
			}
		}

		if (!found)
		{
			err.error_code = PARAM_TYPE_ERROR;
			
			if (strcmp(tokens[0], "set") == 0 && params[0].type == CONSTANT)
			{
				err.bad_token = 1;
				sprintf(err.msg, "Immediate values are not valid destinations for \"set\".");
				
				ERROR_OUT;
			}
			
			char *valid_params = instruction_valid_parameters_errf(tokens[0]);
			
			if (valid_params)
			{
				char buf[512];
				for (int i = 0; i < 512; i++)
					buf[i] = 0;
				
				int position = 0;
				buf[position++] = '(';
				for(int i = 0; i < argc; i++)
				{
					switch (params[i].type)
					{
						case CONSTANT:
							sprintf(&buf[position], "constant");
							break;
						
						case CONSTANT | POINTER:
							sprintf(&buf[position], "constant*");
							break;
						
						case REGISTER:
							sprintf(&buf[position], "register");
							break;
						
						case REGISTER | POINTER:
							sprintf(&buf[position], "register*");
							break;
						
						case S_REGISTER:
							sprintf(&buf[position], "sregister");
							break;
						
						case S_REGISTER | POINTER:
							sprintf(&buf[position], "sregister*");
							break;
					}
					
					position = strlen(buf);
					if (i != argc-1)
					{
						buf[position++] = ',';
						buf[position++] = ' ';
					}
				}
				buf[position++] = ')';
				buf[position++] = 0;
				
				sprintf(err.msg, "Instruction \"%s\" expects parameters of type \n%s,\n\nbut was given parameters of type %s.\n", tokens[0], valid_params, buf);
				free(valid_params);
				err.bad_token = -2;
			}
			else
			{
				sprintf(err.msg, "Invalid parameters for instruction \"%s\".\n", tokens[0]);
				free(valid_params);
				err.bad_token = -2;
			}
			
			ERROR_OUT;
		}
		else
		{
			index--;

			encode_instruction(&file->text, &dunk_instrs[index], params, argc);
			
			if (cxt->flags & VERBOSE)
			{
				printf("Result: ");
				for (int i = buffer_pos; i < file->text.position; i++)
					printf("0x%04x ", file->text.data[i]);
				printf("\n");
			}
		}
	}
	else
	{
		err.error_code = BAD_INSTRUCTION_ERROR;
		strcpy(err.msg, "Unknown instruction");
		err.bad_token = 0;
		
		ERROR_OUT;
	}
	
	if (n_errors != 0)
		return FAILURE;
	return SUCCESS;
}
