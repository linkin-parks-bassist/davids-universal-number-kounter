#include <string.h>
#include "dunkasm.h"

#define INPUT_FILE_NAME 	1
#define OUTPUT_FILE_NAME 	2
#define DASH_O 				3
#define FLAG_LIST 			4
#define OPTION				5

void init_dasm_options(dasm_options *opt)
{
	if (!opt)
		return;
	
	opt->flags = 0;
	opt->error_tolerance = 5;
	opt->input_paths = NULL;
	opt->output_path = NULL;
	opt->n_input_files = 0;
	opt->output_path_generated = 0;
	opt->label_offset = 0;
	opt->output_format = OUTPUT_FORMAT_V3_HEX;
	
	init_wept_config(&opt->err_opt);
}

void free_options(dasm_options *opt)
{
	if (!opt)
		return;
	
	free(opt->input_paths);
	
	if (opt->output_path_generated)
		free(opt->output_path);
}

int wept_option(char *option)
{
	if (!option)
		return WEPT_NO_ERROR;
	
	if (strcmp(option, "warn") == 0)
		return WEPT_WARNING;
	if (strcmp(option, "permit") == 0)
		return WEPT_PERMIT;
	if (strcmp(option, "error") == 0)
		return WEPT_ERROR;
	if (strcmp(option, "term") == 0)
		return WEPT_TERMINATE;
	
	return WEPT_NO_ERROR;
}

int process_options(dasm_context *cxt, int argc, char **arguments)
{
	int n_input_files = 0;
	dasm_error err;

	int argument_types[argc];
	
	for (int i = 0; i < argc; i++)
	{
		if (strcmp(arguments[i], "-o") == 0)
		{
			argument_types[i++] = DASH_O;
			argument_types[i] = OUTPUT_FILE_NAME;
		}
		else
		{
			if (arguments[i][0] == '-')
			{
				if (arguments[i][1] == '-')
				{
					argument_types[i] = OPTION;
				}
				else
				{
					argument_types[i] = FLAG_LIST;
				}
			}
			else
			{
				argument_types[i] = INPUT_FILE_NAME;
				n_input_files++;
			}
		}
	}
	
	if (n_input_files == 0)
	{
		sprintf(err.msg, "no input files specified.");
		err.error_code = BAD_COMMAND_ARGS;
		print_error(err, &cxt->opt.err_opt);
		return WEPT_TERMINATE;
	}
	
	cxt->opt.input_paths = malloc(sizeof(char*) * n_input_files);
	
	for (int j = 0; j < n_input_files; j++)
		cxt->opt.input_paths[j] = NULL;
	
	int len;
	int good;
	int paths_stored = 0;
	int ignore_file;
	int wept_opt_val;
	
	for (int i = 0; i < argc; i++)
	{
		if (argument_types[i] == DASH_O)
		{
			cxt->opt.output_path = arguments[i + 1];
			i++;
		}
		else if (argument_types[i] == INPUT_FILE_NAME)
		{
			if (!file_accessible(cxt, arguments[i]))
			{
				sprintf(err.msg, "File \"%s\" not found", arguments[i]);
				err.error_code = IO_ERROR;
				print_error(err, &cxt->opt.err_opt);
				return WEPT_TERMINATE;
			}
			
			ignore_file = 0;
			for (int j = 0; j < n_input_files; j++)
			{
				if (cxt->opt.input_paths[j] && strcmp(arguments[i], cxt->opt.input_paths[j]) == 0)
				{
					sprintf(err.msg, "File \"%s\" listed twice", arguments[i]);
					err.error_code = BAD_COMMAND_ARGS;
					print_error(err, &cxt->opt.err_opt);
					ignore_file = 1;
					break;
				}
			}
				
			if (!ignore_file)
				cxt->opt.input_paths[paths_stored++] = arguments[i];
		}
		else if (argument_types[i] == FLAG_LIST)
		{
			len = strlen(arguments[i]);
			for (int j = 1; j < len; j++)
			{
				switch (arguments[i][j])
				{
					case 'v':
						cxt->opt.flags = cxt->opt.flags | VERBOSE;
						break;
					
					case 't':
						cxt->opt.output_format = OUTPUT_FORMAT_V3_HEX;
						break;
					
					case 'b':
						cxt->opt.output_format = OUTPUT_FORMAT_BINARY;
						break;
					
					default:
						sprintf(err.msg, "unknown flag \"-%c\"", arguments[i][j]);
						err.error_code = BAD_COMMAND_ARGS;
						print_error(err, &cxt->opt.err_opt);
						return WEPT_TERMINATE;
				}
			}
		}	
		else if (argument_types[i] == OPTION)
		{
			if (strcmp(&arguments[i][2], "verbose") == 0)
			{
				cxt->opt.flags = cxt->opt.flags | VERBOSE;
			}
			else if (strncmp(&arguments[i][2], "offset", 6) == 0)
			{
				if (arguments[i][8] != '=')
				{
					sprintf(err.msg, "Invalid syntax for option \"offset\"; usage is \"--offset=____\"");
					err.error_code = BAD_COMMAND_ARGS;
					print_error(err, &cxt->opt.err_opt);
					return WEPT_TERMINATE;
				}
				
				if (!is_number(&arguments[i][9]))
				{
					sprintf(err.msg, "Invalid argument for option \"offset\": \"%s\"", &arguments[i][9]);
					err.error_code = BAD_COMMAND_ARGS;
					print_error(err, &cxt->opt.err_opt);
					return WEPT_TERMINATE;
				}
				
				cxt->opt.label_offset = parse_number(&arguments[i][9]);
			}
			else if (wept_opt_val = wept_option(&arguments[i][2]))
			{
				if (argc <= i + 1)
				{
					sprintf(err.msg, "No argument specified for \"%s\"", arguments[i]);
					err.error_code = BAD_COMMAND_ARGS;
					print_error(err, &cxt->opt.err_opt);
					return WEPT_TERMINATE;
				}
				
				good = 0;
				for (int j = 0; j < N_WEPT_OPTS; j++)
				{
					if (strcmp(wept_options[j].name, arguments[i+1]) == 0)
					{
						cxt->opt.err_opt.levels[wept_options[j].index] = wept_opt_val;
						good = 1;
						break;
					}
				}
				
				if (!good)
				{
					sprintf(err.msg, "Invalid argument \"%s\" given for \"%s\"", arguments[i + 1], arguments[i]);
					err.error_code = BAD_COMMAND_ARGS;
					print_error(err, &cxt->opt.err_opt);
					return WEPT_TERMINATE;
				}
				
				i++;
			}
			else
			{
				sprintf(err.msg, "unknown option \"%s\"", &arguments[i][2]);
				err.error_code = BAD_COMMAND_ARGS;
				print_error(err, &cxt->opt.err_opt);
				return WEPT_TERMINATE;
			}
		}	
		else
		{
			sprintf(err.msg, "unrecognised argument \"%s\"\n", &arguments[i][2]);
			err.error_code = BAD_COMMAND_ARGS;
			print_error(err, &cxt->opt.err_opt);
			return WEPT_TERMINATE;
		}
	}
	
	cxt->opt.n_input_files = paths_stored;

	// Generate default output path if not provided
	cxt->opt.output_path_generated = 0;
	if (!cxt->opt.output_path)
	{
		size_t len = strlen(cxt->opt.input_paths[0]) + 10;
		cxt->opt.output_path = malloc(len);
		if (!cxt->opt.output_path)
		{
			perror("Memory allocation failed");
			sprintf(err.msg, "Memory allocation failed");
			print_error(err, &cxt->opt.err_opt);
			return WEPT_TERMINATE;
		}
		snprintf(cxt->opt.output_path, len, "%s-assembled", cxt->opt.input_paths[0]);
		cxt->opt.output_path_generated = 1;
	}
}
