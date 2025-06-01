#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>

#include "dunkasm.h"

// Main function to handle input arguments
int main(int argc, char* argv[])
{
	srand(time(0));
	
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <input file(s)> [-o <output file>]\n", argv[0]);
		return EXIT_FAILURE;
	}
	
	dasm_context cxt;
	dasm_buffer output_buffer;
	dasm_error err;
	
	init_context(&cxt);
	init_buffer(&output_buffer);
	
	int failed = 0;
	
	switch (process_options(&cxt, argc - 1, &argv[1]))
	{
		case WEPT_ERROR:
			failed = 1;
			break;
			
		case WEPT_TERMINATE:
			failed = 1;
			break;
			
		default:
			break;
	}
	
	for (int i = 0; i < cxt.opt.n_input_files & !failed; i++)
	{		
		switch (process_file(cxt.opt.input_paths[i], &cxt, (i == 0) ? MAIN_FILE : 0))
		{
			case WEPT_ERROR:
				failed = 1;
				break;
				
			case WEPT_TERMINATE:
				failed = 1;
				break;
			
			default:
				break;
		}
	}
	
	if (failed)
	{
		free_context_data(&cxt);
		destroy_buffer(&output_buffer);
		
		return EXIT_FAILURE;
	}
	
	writeout_and_destroy_context(&cxt, &output_buffer);

	FILE* output_file = fopen(cxt.opt.output_path, "wb");
	if (!output_file)
	{
		sprintf(err.msg, "unable to open output file \"%s\"", cxt.opt.output_path);
		err.error_code = IO_ERROR;
		print_error(err, &cxt.opt.err_opt);
		failed = 1;
	}
	
	if (!failed && (cxt.opt.flags & VERBOSE))
		display_buffer(&output_buffer, "final output buffer:");
	
	if (!failed)
		writeout_buffer(&output_buffer, output_file, cxt.opt.output_format);
	
	destroy_buffer(&output_buffer);
	
	if (!failed)
		fclose(output_file);

	if (failed)
		return EXIT_FAILURE;
	
	return EXIT_SUCCESS;
}
