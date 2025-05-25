#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>

#include <unistd.h>

#include "dunkasm.h"

IMPLEMENT_LINKED_PTR_LIST(dasm_file);

dasm_file *new_dasm_file(const char *fname)
{
	if (fname == NULL) {
		fprintf(stderr, "Error: NULL pointer passed to function `new_dasm_file'\n");
		exit(EXIT_FAILURE);
	}
	dasm_file *res = malloc(sizeof(dasm_file));
	
	if (res == NULL)
		return NULL;
	
	init_buffer(&res->text);
	init_buffer(&res->strings);
	
	char abs_path_buf[PATH_MAX];
	
	if (realpath(fname, abs_path_buf) == NULL) {
		perror(fname);
		exit(EXIT_FAILURE);
	}
	
	res->absolute_path = strdup(abs_path_buf);
	res->given_path = strdup(fname);
	
	res->n_label_refs = 0;
	res->label_refs = NULL;
	
	return res;
}

void dasm_file_destructor(dasm_file *file)
{
	if (file == NULL)
		return;
	
	free(file->absolute_path);
	free(file->given_path);
	free_label_ref_linked_list(file->label_refs);
	destroy_buffer(&file->text);
	destroy_buffer(&file->strings);
}

void free_dasm_file(dasm_file *file)
{
	if (file == NULL)
		return;
	
	dasm_file_destructor(file);
	
	free(file);
}

int file_accessible(dasm_context *cxt, const char *fname)
{
	return (access(fname, F_OK) == 0);
}

int get_file_directory(const char* input_path, char *dest)
{
	if (input_path == NULL || dest == NULL)
		return BAD_ARGUMENTS;
	
	char *slash = strrchr(input_path, '/');
	if (slash != NULL) {
		strncpy(dest, input_path, strrchr(input_path, '/') + 1 - input_path);
		dest[strrchr(input_path, '/') + 1 - input_path] = 0;
	} else {
		dest[0] = '.';
		dest[1] = 0;
	}
	
	return 0;
}

int compare_filenames(dasm_file *f1, dasm_file *f2)
{
	if (f1 == NULL || f2 == NULL)
		return 1;
	
	return strcmp(f1->absolute_path, f2->absolute_path);
}

dasm_file *process_file(const char* input_path, dasm_context *cxt, int flags)
{
	if (input_path == NULL || !valid_dasm_context(cxt))
		return NULL;
	
	dasm_file *file = new_dasm_file(input_path);
	
	if (cxt->flags & VERBOSE) {
		printf("Assembling file \"%s\"\n", input_path);
		
		if (flags & MAIN_FILE)
			printf("This is the main file.\n");
		if (flags & INCLUDED_FILE)
			printf("This is an included file\n");
		if (flags & INCLUDE_FIRST)
			printf("This is an \"include_first\" file\n");
	}
	
	if (file == NULL)
		return NULL;
	
	add_file_to_context(cxt, file, flags & INCLUDE_FIRST);
	
	dasm_line_linked_list *lines = tokenize_file(input_path);
	dasm_line_linked_list *current = lines;
		
	if (lines == NULL)
		return NULL;
		
	char cwd[PATH_MAX];
	getcwd(cwd, PATH_MAX);
	
	char filedir[PATH_MAX];
	get_file_directory(input_path, filedir);
	
	chdir(filedir);

	while (current) {
		strip_comments(&current->data);
		
		process_line(current->data, file, cxt, flags);
		
		if (cxt->n_errors > cxt->error_tolerance)
		{
			destructor_free_dasm_line_linked_list(lines, &dasm_line_destructor);
			return NULL;
		}

		current = current->next;
	}
	
	if (cxt->n_errors == 0)
	{
		/* Add a "halt and catch fire" at the end of the first input file
		 * so that execution doesn't run on after finishing the main file */
		if (flags & MAIN_FILE)
			append_buffer(&file->text, 0xff);
		
		/* Don't allow declared aliases to carry over between files 
		 * except in the case of file inclusion */
		if (!(flags & INCLUDED_FILE))
			clear_nondefault_aliases(cxt);
		
		chdir(cwd);
	}
	
	destructor_free_dasm_line_linked_list(lines, &dasm_line_destructor);
	
	return (cxt->n_errors == 0) ? file : NULL;
}

int valid_dasm_file(const dasm_file *file)
{
	if (file == NULL)
		return 0;
	
	if (!valid_dasm_buffer(&file->text))
		return 0;
	
	if (!valid_dasm_buffer(&file->strings))
		return 0;
	
	if (file->absolute_path == NULL)
		return 0;
	
	return 1;
}

int init_context_files(dasm_context *cxt)
{
	if (cxt == NULL)
		return BAD_ARGUMENTS;
	
	cxt->files = NULL;
	
	return SUCCESS;
}


int add_label_ref_to_file(dasm_file *file, const char *label, unsigned int position, unsigned int line_number)
{
	if (!valid_dasm_file(file) || label == NULL)
		return BAD_ARGUMENTS;
	
	if (file->n_label_refs >= MAX_LABEL_REFS)
		return MEMORY_FAILURE;
	
	label_ref ref;
	
	ref.parent = file;
	strncpy(ref.label, label, MAX_LABEL_LENGTH);
	ref.position = position;
	ref.line_number = line_number;
	
	file->label_refs = label_ref_linked_list_append(file->label_refs, ref);
	
	file->n_label_refs++;
	
	return 0;
}
