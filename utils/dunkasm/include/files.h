#include <limits.h>

#ifndef DUNK_FILES_H
#define DUNK_FILES_H

#define MAIN_FILE 		0b01
#define INCLUDED_FILE	0b10

struct dasm_file {
	char *absolute_path;
	char *given_path;
	
	dasm_buffer text;
	dasm_buffer strings;
	
	unsigned int n_label_refs;
	
	label_ref_linked_list *label_refs;
};

DECLARE_LINKED_PTR_LIST(dasm_file);

typedef dasm_file_ptr_linked_list dasm_file_ptr_linked_list;

dasm_file *process_file(const char* input_path, dasm_context *cxt, int flags);

dasm_file *new_dasm_file(const char *fname);

void dasm_file_destructor(dasm_file *file);
void free_dasm_file(dasm_file *file);

dasm_file *combine_files(dasm_file **dasm_files);

int valid_dasm_file(const dasm_file *file);
int init_context_files(dasm_context *cxt);

int add_label_ref_to_file(dasm_file *file, const char *label, unsigned int position, unsigned int line_number);

int compare_filenames(dasm_file *f1, dasm_file *f2);

int get_file_directory(const char* input_path, char *dest);

#endif
