#ifndef DUNKASM_OPTIONS_H
#define DUNKASM_OPTIONS_H

#define OUTPUT_FORMAT_BINARY 0
#define OUTPUT_FORMAT_V3_HEX 1

struct dasm_options
{
	int flags;
	int error_tolerance;
	
	char **input_paths;
	char *output_path;
	int n_input_files;
	
	uint16_t label_offset;
	
	int output_path_generated;
	
	int output_format;
	
	wept_config err_opt;
};

int process_options(dasm_context *cxt, int argc, char **arguments);
void free_options(dasm_options *opt);

void init_dasm_options(dasm_options *opt);

#endif
