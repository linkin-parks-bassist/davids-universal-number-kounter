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
	for (int i = 0; i < line.n_tokens; i++) {
		free(line.tokens[i]);
		line.tokens[i] = NULL;
	}
	free(line.tokens);
	line.tokens = NULL;
	free(line.raw_line);
	line.raw_line = NULL;
}

void strip_comments(dasm_line *line) {
	if (line == NULL)
		return;
	
	for (int i = 0; i < line->n_tokens; ++i) {
		if (line->tokens[i][0] == '%') {
			for (int j = i + 1; j < line->n_tokens; j++) {
				free(line->tokens[j]);
			}
			line->n_tokens = i;
			break;
		}
	}
}

int process_line(dasm_line line, dasm_file *file, dasm_context *cxt)
{
	if (!valid_dasm_file(file) || !valid_dasm_context(cxt))
		return BAD_ARGUMENTS;
	if (line.n_tokens == 0)
		return SUCCESS;
	
	if (line.line_number == 9) {
		printf("We doin the line\n");
	}
	
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
	
	char *temp_string = NULL;
	dasm_alias temp_alias;
	
	int first_token_length = strlen(line.tokens[0]);
	int buffer_pos = file->text.position;
	
	if (strcmp(line.tokens[0], "alias") == 0) {
		if (line.n_tokens != 3) {
			fprintf(stderr, "Error: syntax error defining alias on line %i; usage is alias [replacee] [replacer].\n", line.line_number);
			exit(EXIT_FAILURE);
		}

		add_alias_to_context(cxt, line.tokens[1], line.tokens[2]);
	} else if (strcmp(line.tokens[0], "include") == 0) {
		for (int i = 1; i < line.n_tokens; i++) {
			
			char *temp_string = strdup(&line.tokens[i][1]);
			
			if (temp_string == NULL) {
				perror("Memory allocation failure (lines.c:89)");
				exit(EXIT_FAILURE);
			}
			
			temp_string[strlen(temp_string) - 1] = 0;
			
			if (cxt->flags & VERBOSE)
				printf("Included file \"%s\"\n", temp_string);
			
			switch (is_file_already_in_context(cxt, temp_string)) {
				case 1:
					// Attempted double file inclusion; simply skip this silently
					if (cxt->flags & VERBOSE)
						printf("File is already in list; skipping...\n");
					break;
				
				case 2:
					fprintf(stderr, "%s:%d: error opening included file \"%s\"\n", file->given_path, line.line_number, temp_string);
					exit(EXIT_FAILURE);
					break;
				
				default:
					process_file(temp_string, cxt, INCLUDED_FILE);
					break;
			}
			
			free(temp_string);
		}
	} else if (strcmp(line.tokens[0], "dealias") == 0) {
		for (int k = 1; k < line.n_tokens; k++) {
			temp_alias.replacee = line.tokens[k];
			
			cxt->aliases = dasm_alias_linked_list_destructor_free_and_remove_matching(cxt->aliases, &compare_aliases, temp_alias, &dasm_alias_destructor);
		}
	} else if (line.n_tokens == 1 && line.tokens[0][first_token_length - 1] == ':') {
		temp_string = strdup(line.tokens[0]);
		
		if (temp_string == NULL) {
			perror("Memory allocation failure (lines.c:127)");
			exit(EXIT_FAILURE);
		}
		
		temp_string[first_token_length - 1] = 0;
		add_label_to_context(cxt, file, temp_string, file->text.position, line.line_number, CODE_LABEL);
		
		free(temp_string);
	} else {
		dasm_alias_linked_list *current;
		for (int j = 0; j < line.n_tokens; j++) {
			current = cxt->aliases;
			
			while (current != NULL) {
				if (current->data.replacee == NULL || current->data.replacer == NULL) {
					fprintf(stderr, "Error: NULL alias\n");
					exit(EXIT_FAILURE);
				}
				if (strcmp(current->data.replacee, line.tokens[j]) == 0) {
					free(line.tokens[j]);
					line.tokens[j] = strdup(current->data.replacee);
				}
				
				current = current->next;
			}
		}
		
		parameter params[MAX_PARAMS];
		int argc = line.n_tokens - 1;
		
		
		if (argc >= MAX_PARAMS) {
			fprintf(
					stderr,
					"Error: too many arguments on ``%s\", line %i.\n", file->absolute_path, line.line_number);
				exit(EXIT_FAILURE);
		}
		
		for (int i = 0; i < argc; ++i) {
			params[i] = parse_parameter(cxt, line.tokens[i + 1]);
			
			if (params[i].type == LABEL_P) {
				handle_label_parameter(cxt, file, &params[i], line.tokens[i + 1], line.line_number);
			}
			else if (params[i].type == STRING_P) {
				handle_string_parameter(cxt, file, &params[i], line.tokens[i + 1], line.line_number);
			}
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
					line.n_tokens - 1 == dunk_instrs[index].n_parameters) {
				found = 1;
				
				for (int j = 0; j < argc; j++) {
					if (params[j].type != dunk_instrs[index].parameter_types[j]) {
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

			encode_instruction(&file->text, &dunk_instrs[index], params, argc);
			
			if (cxt->flags & VERBOSE) {
				printf("Result: ");
				for (int i = buffer_pos; i < file->text.position; i++)
					printf("0x%04x ", file->text.data[i]);
				printf("\n");
			}
		}
	}
	
	return SUCCESS;
}
