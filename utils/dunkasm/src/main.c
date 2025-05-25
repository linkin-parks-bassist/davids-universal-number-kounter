#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>

#include "dunkasm.h"

#define INPUT_FILE_NAME 	1
#define OUTPUT_FILE_NAME 	2
#define DASH_O 				3
#define FLAG_LIST 			4
#define OPTION				5

// Main function to handle input arguments
int main(int argc, char* argv[])
{
	srand(time(0));
	
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <input file(s)> [-o <output file>]\n", argv[0]);
		return EXIT_FAILURE;
	}
	
	int n_input_files = 0;
	char* output_path = NULL;

	// Parse arguments
	
	int argument_types[argc-1];
	
	for (int i = 0; i < argc - 1; i++) {
		if (strcmp(argv[i + 1], "-o") == 0) {
			argument_types[i] = DASH_O;
			argument_types[i+1] = OUTPUT_FILE_NAME;
			i += 2;
		} else {
			if (argv[i + 1][0] == '-') {
				if (argv[i + 1][1] == '-') {
					argument_types[i] = OPTION;
				} else {
					argument_types[i] = FLAG_LIST;
				}
			} else {
				argument_types[i] = INPUT_FILE_NAME;
				n_input_files++;
			}
		}
	}
	
	if (n_input_files == 0) {
		fprintf(stderr, "Error: No input files specified.\n");
		return EXIT_FAILURE;
	}
	
	char *input_paths[n_input_files];
	
	int input_files_stored = 0;
	int len;
	
	dasm_context cxt;
	init_context(&cxt);
	
	for (int i = 0; i < argc - 1; i++) {
		switch (argument_types[i]) {
			case DASH_O:
				output_path = argv[i + 2];
				i++;
				break;
			
			case INPUT_FILE_NAME:
				input_paths[input_files_stored] = argv[i + 1];
				input_files_stored++;
				break;
			
			case FLAG_LIST:
				len = strlen(argv[i + 1]);
				for (int j = 1; j < len; j++) {
					switch (argv[i+1][j]) {
						case 'v':
							cxt.flags = cxt.flags | VERBOSE;
							break;
						
						default:
							fprintf(stderr, "Error: unknown flag \"-%c\"\n", argv[i+1][j]);
							exit(EXIT_FAILURE);
							break;
					}
				}
				
			case OPTION:
				if (strcmp(&argv[i + 1][2], "verbose") == 0) {
					cxt.flags = cxt.flags | VERBOSE;
				} else {
					fprintf(stderr, "Error: unknown option \"%s\"\n", &argv[i+1][2]);
					exit(EXIT_FAILURE);
				}
				break;
			
			default:
				fprintf(stderr, "Error: unrecognised argument \"%s\"\n", &argv[i+1][2]);
				exit(EXIT_FAILURE);
				break;
		}
	}

	// Generate default output path if not provided
	int output_path_generated = 0;
	if (!output_path)
	{
		size_t len = strlen(input_paths[0]) + 10;
		output_path = malloc(len);
		if (!output_path) {
			perror("Memory allocation failed");
			return EXIT_FAILURE;
		}
		snprintf(output_path, len, "%s-assembled", input_paths[0]);
		output_path_generated = 1;
	}
	
	dasm_buffer output_buffer;
	init_buffer(&output_buffer);
	
	int failed = 0;

	for (int i = 0; i < n_input_files; i++)
	{
		if (process_file(input_paths[i], &cxt, (i == 0) ? MAIN_FILE : 0) == NULL)
		{
			failed = 1;
			break;
		}
	}
	
	if (failed)
	{
		free_context_data(&cxt);
		destroy_buffer(&output_buffer);
		
		if (output_path_generated)
			free(output_path);
		
		return EXIT_FAILURE;
	}
	
	writeout_and_destroy_context(&cxt, &output_buffer);

	FILE* output_file = fopen(output_path, "wb");
	if (!output_file)
	{
		perror("Error opening output file");
		exit(EXIT_FAILURE);
	}
	
	if (cxt.flags & VERBOSE)
		display_buffer(&output_buffer, "final output buffer:");
	
	writeout_buffer(&output_buffer, output_file);
	
	destroy_buffer(&output_buffer);
	
	fclose(output_file);
	if (output_path_generated) {
		free(output_path);
	}

	return EXIT_SUCCESS;
}
