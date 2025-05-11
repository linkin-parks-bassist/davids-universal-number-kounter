#ifndef DASM_CONTEXT_H
#define DASM_CONTEXT_H

#define MAX_N_FILES 32

struct  dasm_context {
	int n_aliases;
	alias aliases[MAX_N_ALIASES];
	
	int n_pa_files;
	pa_file *pa_files[MAX_N_FILES];
	
	unsigned int n_labels;
	dasm_label labels[MAX_LABELS];
	
	unsigned int n_strings;
	dasm_string strings[MAX_N_STRINGS];
};

int init_context(dasm_context *cxt);
dasm_context *new_dasm_context();
int free_context_data(dasm_context *cxt);

int valid_dasm_context(dasm_context *cxt);

int writeout_context(dasm_context *cxt, dasm_buffer *buf);

int add_pa_file_to_context(dasm_context *cxt, pa_file *file);
int add_label_to_context(dasm_context *cxt, pa_file *parent, const char *name, unsigned int pos, int type);
int add_string_to_context(dasm_context *cxt, const char *value, pa_file *parent, unsigned int pos);

int generate_strings(dasm_context *cxt);

#endif
