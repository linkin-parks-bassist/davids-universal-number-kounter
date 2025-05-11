#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>

#include "dunkasm.h"

int init_context(dasm_context *cxt)
{
	if (cxt == NULL)
		return BAD_ARGUMENTS;
	
	init_context_aliases(cxt);
	init_context_labels(cxt);
	init_context_files(cxt);
	
	cxt->n_strings = 0;
	
	return SUCCESS;
}

dasm_context *new_dasm_context()
{
	dasm_context *res = malloc(sizeof(dasm_context));
	
	if (res == NULL)
		return NULL;
	
	init_context(res);
	return res;
}

int free_context_data(dasm_context *cxt)
{
	if (cxt == NULL)
		return BAD_ARGUMENTS;
	
	
	for (int i = 0; i < cxt->n_aliases; i++) {
		free_alias(cxt->aliases[i]);
	}
	
	for (int i = 0; i < cxt->n_pa_files; i++) {
		free_pa_file(cxt->pa_files[i]);
	}
	
	for (int i = 0; i < cxt->n_labels; i++) {
		free_label(cxt->labels[i]);
	}
}

int valid_dasm_context(dasm_context *cxt)
{
	if (cxt == NULL)
		return 0;
	
	if (cxt->n_aliases > MAX_N_ALIASES)
		return 0;
	
	for (int i = 0; i < cxt->n_aliases; i++) {
		if (cxt->aliases[i].replacee == NULL || cxt->aliases[i].replacer == NULL) {
			return 0;
		}
	}
	
	
	if (cxt->n_pa_files > MAX_N_ALIASES)
		return 0;
	
	for (int i = 0; i < cxt->n_pa_files; i++) {
		if (!valid_pa_file(cxt->pa_files[i])) {
			return 0;
		}
	}
	
	return 1;
}


int add_pa_file_to_context(dasm_context *cxt, pa_file *file)
{
	if (!valid_dasm_context(cxt) || !valid_pa_file(file))
		return BAD_ARGUMENTS;
	
	if (cxt->n_pa_files >= MAX_N_FILES)
		return 3;
	
	cxt->pa_files[cxt->n_pa_files] = file;
	cxt->n_pa_files++;
	
	return 0;
}

int writeout_context(dasm_context *cxt, dasm_buffer *buf)
{
	if (!valid_dasm_context(cxt) || !valid_dasm_buffer(buf))
		return BAD_ARGUMENTS;
	
	generate_strings(cxt);
	
	/* Paste all the generated machine code into the output buffer */
	for (int i = 0; i < cxt->n_pa_files; i++) {
		concat_buffers(buf, &cxt->pa_files[i]->text);
	}
	
	/* Paste the generated strings sections into the output buffer */
	for (int i = 0; i < cxt->n_pa_files; i++) {
		concat_buffers(buf, &cxt->pa_files[i]->strings);
	}
	
	/* Calculate and insert the final resting places
	 * of all labels referred to inside the code */
	insert_label_addresses(cxt, buf);
	
	/* De-allocating */
	for (int i = 0; i < cxt->n_pa_files; i++) {
		destroy_buffer(&cxt->pa_files[i]->text);
		destroy_buffer(&cxt->pa_files[i]->strings);
	}
	
	return SUCCESS;
}

int add_label_to_context(dasm_context *cxt, pa_file *parent, const char *name, unsigned int pos, int type)
{
	if (!valid_dasm_context(cxt) || name == NULL)
		return -BAD_ARGUMENTS;
	
	if (cxt->n_labels >= MAX_LABELS)
		return -MEMORY_FAILURE;
	
	cxt->labels[cxt->n_labels].parent = parent;
	cxt->labels[cxt->n_labels].name = strdup(name);
	
	cxt->labels[cxt->n_labels].position = pos;
	cxt->labels[cxt->n_labels].type = type;
	
	cxt->n_labels++;
	
	if (cxt->labels[cxt->n_labels - 1].name == NULL)
		return -MEMORY_FAILURE;

	return cxt->n_labels - 1;
}

int add_string_to_context(dasm_context *cxt, const char *value, pa_file *parent, unsigned int pos)
{
	if (!valid_dasm_context(cxt) || value == NULL || parent == NULL)
		return BAD_ARGUMENTS;
	
	cxt->strings[cxt->n_strings] = new_dasm_string(value, cxt->n_labels);
	cxt->n_strings++;
	
	char namebuf[48];
	
	sprintf(namebuf, "%s_string_0x%x__%d", parent->fname, cxt->n_strings, rand());
	
	int retval = add_label_to_context(cxt, parent, namebuf, pos, STRING_LABEL);
	
	return retval;
}

int generate_strings(dasm_context *cxt)
{
	if (!valid_dasm_context(cxt))
		return BAD_ARGUMENTS;
	
	int j;
	for (int i = 0; i < cxt->n_strings; i++) {
		j = cxt->strings[i].label_n;
		
		if (cxt->labels[j].parent == NULL)
			return BAD_ARGUMENTS;
		
		cxt->labels[j].position = cxt->labels[j].parent->strings.position;
		
		for (int n = 0; cxt->strings[i].value[n] != 0; n++) {
			append_buffer(&cxt->labels[j].parent->strings, cxt->strings[i].value[n]);
		}
		
		append_buffer(&cxt->labels[j].parent->strings, 0);
	}
	
	for (int n = 0; n < cxt->n_pa_files; n++) {
		char message_buffer[48];
		sprintf(message_buffer, "File %d (%s) text buffer", n, cxt->pa_files[n]->fname);
		display_buffer(&cxt->pa_files[n]->text, message_buffer);
		
		sprintf(message_buffer, "File %d (%s) strings buffer", n, cxt->pa_files[n]->fname);
		display_buffer(&cxt->pa_files[n]->strings, message_buffer);
	}
	
	return 0;
}
