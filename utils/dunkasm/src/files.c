#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>

#include <unistd.h>

#include "dunkasm.h"

//IMPLEMENT_LINKED_PTR_LIST(pa_file);

pa_file *new_pa_file(const char *fname)
{
	pa_file *res = malloc(sizeof(pa_file));
	
	if (res == NULL)
		return NULL;
	
	init_buffer(&res->text);
	init_buffer(&res->strings);
	
	res->fname = strdup(fname);
	
	res->n_label_refs = 0;
}


int free_pa_file(pa_file *file)
{
	if (file == NULL)
		return BAD_ARGUMENTS;
	
	destroy_buffer(&file->text);
	destroy_buffer(&file->strings);
	free(file);
	
	return SUCCESS;
}

pa_file *process_file(const char* input_path, dasm_context *cxt, int flags)
{
	if (input_path == NULL || !valid_dasm_context(cxt))
		return NULL;
		
	char cwd[128];
	getcwd(cwd, 128);
	
	char filedir[128];
	
	char *slash = strrchr(input_path, '/');
	if (slash != NULL) {
		strncpy(filedir, input_path, strrchr(input_path, '/') + 1 - input_path);
		filedir[strrchr(input_path, '/') + 1 - input_path] = 0;
	} else {
		filedir[0] = '.';
		filedir[1] = 0;
	}
	
	pa_file *file = new_pa_file(input_path);
	
	if (file == NULL)
		return NULL;
	
	add_pa_file_to_context(cxt, file);
	
	line_data_node *head = tokenize_file(input_path);
	
	chdir(filedir);
	
	if (head == NULL)
		return NULL;
	
	line_data_node *current = head;
	line_data_struct line;

	while (current) {
		line = *current->data;
		
		for (int i = 0; i < line.token_count; ++i) {
			if (line.tokens[i][0] == '%') {
				line.token_count = i;
				break;
			}
		}
		
		process_line(line, file, cxt);

		current = current->next;
	}
	
	/* Add a "halt and catch fire" at the end of the first input file
	 * so that execution doesn't run on after finishing the main file */
	if (flags & MAIN_FILE)
		append_buffer(&file->text, 0xff);
	
	/* Don't allow declared aliases to carry over between files; free the malloc'd strings */
	clear_nondefault_aliases(cxt);
	
	free_list(head);
	
	chdir(cwd);
	
	return file;
}

int valid_pa_file(const pa_file *file)
{
	if (file == NULL)
		return 0;
	
	if (!valid_dasm_buffer(&file->text))
		return 0;
	
	if (!valid_dasm_buffer(&file->strings))
		return 0;
	
	if (file->fname == NULL)
		return 0;
	
	return 1;
}

int init_context_files(dasm_context *cxt)
{
	if (cxt == NULL)
		return BAD_ARGUMENTS;
	
	cxt->n_pa_files = 0;
	
	for (int i = 0; i < MAX_N_FILES; i++)
		cxt->pa_files[i] = NULL;
	
	return SUCCESS;
}


int add_label_ref_to_file(pa_file *file, const char *label, unsigned int position, unsigned int line_number)
{
	if (!valid_pa_file(file) || label == NULL)
		return BAD_ARGUMENTS;
	
	if (file->n_label_refs >= MAX_LABEL_REFS)
		return MEMORY_FAILURE;
	
	file->label_refs[file->n_label_refs].parent = file;
	strncpy(file->label_refs[file->n_label_refs].label, label, MAX_LABEL_LENGTH);
	file->label_refs[file->n_label_refs].position = position;
	file->label_refs[file->n_label_refs].line_number = line_number;
	
	file->n_label_refs++;
	
	return 0;
}
