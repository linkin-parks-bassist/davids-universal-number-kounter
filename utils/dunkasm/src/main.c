#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>

#include "dunkasm.h"

uint16_t encode_goto(dasm_context *cxt, line_data_struct line)
{
	uint16_t code = INVALID;

	if (strcmp(line.tokens[0], "goto") == 0) {
		code = PLAIN_GOTO;
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

		parameter param = parse_parameter(cxt, line.tokens[1]);

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

// Main function to handle input arguments
int main(int argc, char* argv[])
{
	srand(time(0));
	
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
	
	dasm_context cxt;
	init_context(&cxt);
	
	dasm_buffer output_buffer;
	init_buffer(&output_buffer);

	for (int i = 0; i < n_files; i++) {
		process_file(input_paths[i], &cxt, (i == 0) ? MAIN_FILE : 0);
	}
	
	writeout_context(&cxt, &output_buffer);
	
	// Save the writing position at the end of writing code to the output_buffer array; 
	// The function insert_label_addresses() messes with this value, since it has to go
	// back to insert label addresses, and it writes to the output_buffer array using
	// the append_buffer macro, which uses current_position. Not great practice, I admit,
	// but it works fine for a project on this scale.

	FILE* output_file = fopen(output_path, "wb");
	if (!output_file) {
		perror("Error opening output file");
		exit(EXIT_FAILURE);
	}
	
	display_buffer(&output_buffer, "Final output buffer");
	
	writeout_and_destroy_buffer(&output_buffer, output_file);
	fclose(output_file);
	
	free(input_paths);
	
	if (output_path_generated) {
		free(output_path);
	}

	return EXIT_SUCCESS;
}
