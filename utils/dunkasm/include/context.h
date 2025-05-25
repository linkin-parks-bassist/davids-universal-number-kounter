#ifndef DASM_CONTEXT_H
#define DASM_CONTEXT_H

#define MAX_N_FILES 32

#define VERBOSE 0b1

struct  dasm_context {
	unsigned int n_strings;
	unsigned int n_labels;
	unsigned int n_files;
	
	dasm_alias_linked_list   *aliases;
	dasm_file_ptr_linked_list  *files;
	dasm_label_linked_list	*labels;
	dasm_string_linked_list  *strings;
	
	int flags;
};

int init_context(dasm_context *cxt);

dasm_context *new_dasm_context();

int free_context_data(dasm_context *cxt);

int valid_dasm_context(dasm_context *cxt);

int writeout_and_destroy_context(dasm_context *cxt, dasm_buffer *buf);

int add_alias_to_context(dasm_context *cxt, const char *replacee, const char *replacer);
int add_file_to_context(dasm_context *cxt, dasm_file *file, dasm_file *current_file, int flags);
dasm_label *add_label_to_context(dasm_context *cxt, dasm_file *parent, const char *name, unsigned int pos, unsigned int line, int type);
dasm_label *add_string_to_context(dasm_context *cxt, const char *value, dasm_file *parent, unsigned int pos, unsigned int line);

int is_file_already_in_context(dasm_context *cxt, char *fname);

int generate_strings_section(dasm_context *cxt);

#endif
