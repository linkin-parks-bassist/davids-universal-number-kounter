#ifndef DUNK_FILES_H
#define DUNK_FILES_H

#define MAIN_FILE 		0b01
#define INCLUDED_FILE	0b10

/* Partially assembled file struct.
 * The result of assembling a file, but 
 * not yet inserting the addresses for
 * label references, which is to be done
 * after the sections of the various files 
 * are concatenated in the appropriate order
 * in an output buffer, soas to get correct
 * addresses for all label references */
struct pa_file {
	const char *fname;
	
	dasm_buffer text;
	dasm_buffer strings;
	
	unsigned int n_label_refs;
	label_ref label_refs[MAX_LABEL_REFS];
};

DECLARE_LINKED_PTR_LIST(pa_file);

pa_file *process_file(const char* input_path, dasm_context *cxt, int flags);

pa_file *new_pa_file(const char *fname);
int free_pa_file(pa_file *file);

pa_file *combine_files(pa_file **pa_files);

int valid_pa_file(const pa_file *file);
int init_context_files(dasm_context *cxt);

int add_label_ref_to_file(pa_file *file, const char *label, unsigned int position, unsigned int line_number);

#endif
