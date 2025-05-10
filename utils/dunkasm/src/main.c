#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dunkasm.h"

uint8_t* binary_output;
int current_position = 0;

int n_labels = 0;
int n_label_refs = 0;

uint16_t* word_ptr;

void write_word(uint16_t v)
{
	//printf("write_word 0x%04x at position %i=0x%x\n", v, current_position, current_position); 
	word_ptr = (uint16_t*)(&binary_output[2 * current_position]); 
	*word_ptr = v;
	current_position += 1;
	word_ptr = (uint16_t*)(&binary_output[2 * current_position]);
}

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
		
		//printf("Encoding goto of type 0x%x with parameter value %d at 0x%x\n", code, param.value, code + 0x1000 * param.value);
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

		write_word(code);
		write_word(0);
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
		
		//printf("I repeat: line %d, which has %d tokens, which are\n    ``%s\"", line.line_number, line.token_count, line.tokens[0]);

		//for (int i = 1; i < line.token_count; ++i)
			//printf(", ``%s\"", line.tokens[i]);
		
		//putchar('\n');
		
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

			write_word(code);

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
						write_word(params[j].value);
					} else if ((dunk_instrs[index].arg_positions[j] & FOLLOWING_MASK) == OFFSET_FOLLOWING(k)) {
						//printf("its offset does.\n", j, k);
						write_word(params[j].offset);
					}
				}
			}
		}
	}
}

// Function to process file input and generate output
void process_file(const char* input_path)
{
	//printf("Assembling file %s.\n", input_path);

	line_data_node *head = tokenize_file(input_path);
	line_data_node* current = head;
	line_data_str line;

	while (current) {
		line = *current->data;

		//printf("Assembling line %d, which has %d tokens, which are\n    ``%s\"", line.line_number, line.token_count, line.tokens[0]);

		for (int i = 1; i < line.token_count; ++i)
			//printf(", ``%s\"", line.tokens[i]);
		
		//putchar('\n');
		
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
	binary_output = malloc(MAX_BIN_SIZE);
	
	for (int i = 0; i < MAX_BIN_SIZE; i++) {
		binary_output[1] = 0;
	}
	
	for (int i = 0; i < MAX_LABELS; i++) {
		for (int j = 0; j < MAX_LABEL_LENGTH; j++) {
			labels[j][i] = 0;
		}
	}
	
	init_aliases();
	
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
		write_word(label_positions[label_index]);
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
	int n_files = 0;

	char* output_path = NULL;

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
	// the write_word macro, which uses current_position. Not great practice, I admit,
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
