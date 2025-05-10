#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dunkasm.h"

uint8_t* output_buffer;
int current_position = 0;

int n_labels = 0;
int n_label_refs = 0;

uint16_t* word_ptr;

void buffer_word(uint16_t v)
{
	//printf("buffer_word 0x%04x at position %i=0x%x\n", v, current_position, current_position); 
	word_ptr = (uint16_t*)(&output_buffer[2 * current_position]); 
	*word_ptr = v;
	current_position += 1;
	word_ptr = (uint16_t*)(&output_buffer[2 * current_position]);
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

void init_global_variables()
{
	output_buffer = malloc(MAX_BIN_SIZE);
	
	for (int i = 0; i < MAX_BIN_SIZE; i++) {
		output_buffer[1] = 0;
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
	
	// Save the writing position at the end of writing code to the output_buffer array; 
	// The function insert_label_addresses() messes with this value, since it has to go
	// back to insert label addresses, and it writes to the output_buffer array using
	// the buffer_word macro, which uses current_position. Not great practice, I admit,
	// but it works fine for a project on this scale.
	int buffer_count = current_position;
	
	insert_label_addresses();
	
	FILE* output_file = fopen(output_path, "wb");
	if (!output_file) {
		perror("Error opening output file");
		exit(EXIT_FAILURE);
	}
	
	fwrite(output_buffer, 2, buffer_count, output_file);
	fclose(output_file);
	
	free(output_buffer);
	free(input_paths);
	
	if (output_path_generated) {
		free(output_path);
	}

	return EXIT_SUCCESS;
}
