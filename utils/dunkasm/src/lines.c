#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dunkasm.h"

void free_line_data(line_data_str* line_data)
{
	for (int i = 0; i < line_data->token_count; i++) {
		free(line_data->tokens[i]);
	}
	free(line_data->tokens);
	free(line_data->raw_line);
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

void process_line(line_data_str line, const char* fname) {
	if (line.token_count == 0)
		return;
	
	printf("Processing line %d, \"%s\", which has %d tokens, which are\n    ``%s\"", line.line_number, line.raw_line, line.token_count, line.tokens[0]);

	for (int i = 1; i < line.token_count; ++i)
		printf(", ``%s\"", line.tokens[i]);
	
	putchar('\n');
	
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

		buffer_word(code);
		buffer_word(0);
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
		
		//printf("Number of arguments: %d\n", argc);
		
		if (argc >= MAX_PARAMS) {
			fprintf(
					stderr,
					"Error: too many arguments on ``%s\", line %i.\n", fname, line.line_number);
				exit(EXIT_FAILURE);
		}
		
		for (int i = 0; i < argc; ++i) {
			params[i] = parse_parameter(line.tokens[i + 1]);
			//printf("argument ``%s\", type %i\n", line.tokens[i + 1], params[i].type);
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

			buffer_word(code);

			//printf("Command %s, %i arguments:\n", dunk_instrs[index].name, argc);
			
			//for (int j = 0; j < argc; j++) {
				//printf("    type %i in position 0b%16b\n",
				//dunk_instrs[index].arg_types[j],
				//dunk_instrs[index].arg_positions[j]);
			//}

			for (int k = 1; k < argc + 1; k++) {
				for (int j = 0; j < argc; j++) {
					//printf("Check if argument %i follows in position %i, by comparing 0b%16b with 0b%16b and 0b%16b\n",
					//	j, k, dunk_instrs[index].arg_positions[j], FOLLOWING(k), OFFSET_FOLLOWING(k));
						
					if ((dunk_instrs[index].arg_positions[j] & FOLLOWING_MASK) == FOLLOWING(k)) {
						//printf("it does.\n", j, k);
						buffer_word(params[j].value);
					} else if ((dunk_instrs[index].arg_positions[j] & FOLLOWING_MASK) == OFFSET_FOLLOWING(k)) {
						//printf("its offset does.\n", j, k);
						buffer_word(params[j].offset);
					}
				}
			}
		}
	}
}
