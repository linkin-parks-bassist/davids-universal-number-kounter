#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>

#include "dunkasm.h"

int init_context(dasm_context *cxt)
{
	if (cxt == NULL)
		return BAD_ARGUMENTS;
	
	cxt->n_strings = 0;
	cxt->n_labels = 0;
	cxt->n_files = 0;
	
	cxt->aliases = NULL;
	cxt->labels  = NULL;
	cxt->files   = NULL;
	cxt->strings = NULL;
	cxt->inclusions = NULL;
	
	init_context_aliases(cxt);
	init_context_labels(cxt);
	init_context_files(cxt);
	init_dasm_options(&cxt->opt);
	
	cxt->n_errors = 0;
	
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
	
	
	destructor_free_dasm_alias_linked_list 	  (cxt->aliases, &dasm_alias_destructor);
	destructor_free_dasm_file_ptr_linked_list (cxt->files,   &free_dasm_file);
	destructor_free_dasm_label_linked_list 	  (cxt->labels,  &dasm_label_destructor);
	destructor_free_dasm_string_linked_list	  (cxt->strings, &dasm_string_destructor);
	destructor_free_dasm_inclusion_linked_list(cxt->inclusions, &dasm_inclusion_destructor);
	
	free_options(&cxt->opt);
}

int valid_dasm_context(dasm_context *cxt)
{
	if (cxt == NULL)
		return 0;
	
	return 1;
}

int is_file_already_in_context(dasm_context *cxt, char *fname)
{
	if (!valid_dasm_context(cxt) || fname == NULL)
		return BAD_ARGUMENTS;
	
	char abspath[PATH_MAX];
	realpath(fname, abspath);
	
	dasm_file_ptr_linked_list *current = cxt->files;
	
	while (current)
	{
		if (strcmp(current->data->absolute_path, abspath) == 0)
			return 1;
		current = current->next;
	}
	
	return 0;
}

int add_file_to_context(dasm_context *cxt, dasm_file *file, int include_first)
{
	if (!valid_dasm_context(cxt) || !valid_dasm_file(file))
		return BAD_ARGUMENTS;
	
	if (include_first)
	{
		dasm_file_ptr_linked_list *new_ll = dasm_file_ptr_linked_list_new(file);
		new_ll->next = cxt->files;
		cxt->files = new_ll;
	}
	else
	{
		cxt->files = dasm_file_ptr_linked_list_append(cxt->files, file);
	}
	
	cxt->n_files++;
	
	return 0;
}

int add_alias_to_context(dasm_context *cxt, const char *replacee, const char *replacer, dasm_file *parent)
{
	if (!valid_dasm_context(cxt) || replacee == NULL || replacer == NULL)
		return BAD_ARGUMENTS;
	
	dasm_alias alias;
	
	alias.replacee = strdup(replacee);
	alias.replacer = strdup(replacer);
	alias.parent = parent;
	
	cxt->aliases = dasm_alias_linked_list_append(cxt->aliases, alias);
	
	return SUCCESS;
}

dasm_label *add_label_to_context(dasm_context *cxt, dasm_file *parent, const char *name, unsigned int pos, unsigned int line, int type)
{
	if (!valid_dasm_context(cxt) || name == NULL)
		return NULL;
	
	dasm_label label;
	
	label.parent = parent;
	label.name 	 = strdup(name);
	
	label.line 		= line;
	label.position  = pos;
	label.type 		= type;
	
	dasm_label_linked_list *duplicate = dasm_label_linked_list_cmp_search(cxt->labels, &compare_label_names, label);
	
	if (duplicate != NULL) {
		fprintf(stderr, "%s:%d: Error: label \"%s\" already defined (%s:%d)\n", parent->given_path, line, name, duplicate->data.parent->given_path, duplicate->data.line);
		exit(EXIT_FAILURE);
	}
	
	dasm_label_linked_list *tail = dasm_label_linked_list_append_return_tail(&cxt->labels, label);

	cxt->n_labels++;
	
	return &tail->data;
}

dasm_label *add_string_to_context(dasm_context *cxt, const char *value, dasm_file *parent, unsigned int pos, unsigned int line)
{
	if (!valid_dasm_context(cxt) || value == NULL || parent == NULL)
		return NULL;
	
	char namebuf[PATH_MAX];
	sprintf(namebuf, "%s_string_0x%x__%d", parent->absolute_path, cxt->n_strings, rand());
	
	dasm_label *label = add_label_to_context(cxt, parent, namebuf, pos, line, STRING_LABEL);
	
	if (label == NULL)
		return NULL;
	
	dasm_string str = new_dasm_string(value, label);
	
	cxt->strings = dasm_string_linked_list_append(cxt->strings, str);
	//cxt->n_strings++;
	
	return label;
}

int writeout_and_destroy_context(dasm_context *cxt, dasm_buffer *buf)
{
	if (!valid_dasm_context(cxt) || !valid_dasm_buffer(buf))
		return BAD_ARGUMENTS;
	
	generate_strings_section(cxt);
	
	/* Paste all the generated machine code into the output buffer */
	dasm_file_ptr_linked_list *current_file = cxt->files;
	
	while (current_file)
	{
		concat_buffers(buf, &current_file->data->text);
		
		current_file = current_file->next;
	}
	
	/* Paste the generated strings sections into the output buffer */
	current_file = cxt->files;
	while (current_file)
	{
		concat_buffers(buf, &current_file->data->strings);
		
		current_file = current_file->next;
	}
	
	/* Calculate and insert the final resting places
	 * of all labels referred to inside the code */
	insert_label_addresses(cxt, buf);
	
	free_context_data(cxt);
	
	return SUCCESS;
}

int generate_strings_section(dasm_context *cxt)
{
	if (!valid_dasm_context(cxt))
		return BAD_ARGUMENTS;
	
	dasm_string_linked_list *current_string = cxt->strings;
	
	while (current_string) {
		if (current_string->data.label == NULL || current_string->data.value == NULL)
			return BAD_ARGUMENTS;
		
		current_string->data.label->position = current_string->data.label->parent->strings.position;
		
		for (int n = 0; current_string->data.value[n] != 0; n++) {
			append_buffer(&current_string->data.label->parent->strings, current_string->data.value[n]);
		}
		
		append_buffer(&current_string->data.label->parent->strings, 0);
		current_string = current_string->next;
	}
	
	dasm_file_ptr_linked_list *current_file = cxt->files;
	
	if (cxt->opt.flags & VERBOSE) {
		while (current_file) {
			char message_buffer[PATH_MAX + 64];
			sprintf(message_buffer, "File \"%s\" text buffer", current_file->data->absolute_path);
			display_buffer(&current_file->data->text, message_buffer);
			
			sprintf(message_buffer, "File \"%s\" strings buffer", current_file->data->absolute_path);
			display_buffer(&current_file->data->strings, message_buffer);
			
			current_file = current_file->next;
		}
	}
	
	return 0;
}
