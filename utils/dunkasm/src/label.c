#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dunkasm.h"

IMPLEMENT_LINKED_LIST(dasm_label);
IMPLEMENT_LINKED_LIST(label_ref);

void dasm_label_destructor(dasm_label label)
{
	free(label.name);
}

int compare_label_names(dasm_label a, dasm_label b)
{
	return strcmp(a.name, b.name);
}

int insert_label_addresses(dasm_context *cxt, dasm_buffer *buf)
{
	if (!valid_dasm_context)
		return BAD_ARGUMENTS;
	
	int matching_label_index;
	
	unsigned int label_offsets[cxt->n_labels];
	
	dasm_label_linked_list *current_label = cxt->labels;
	dasm_file_ptr_linked_list *current_file = NULL;
	
	for (int i = 0; current_label != NULL; i++) {
		label_offsets[i] = 0;
		
		switch (current_label->data.type) {
			case STRING_LABEL:
				current_file = cxt->files;
				
				while (current_file != NULL && current_file->data != NULL) {
					label_offsets[i] += current_file->data->text.length;
					
					current_file = current_file->next;
				}
				
				current_file = cxt->files;
				
				while (current_file != NULL && current_file->data != NULL) {
					if (current_label->data.parent == current_file->data)
						break;
					label_offsets[i] += current_file->data->strings.length;
					
					current_file = current_file->next;
				}
				break;
				
			case CODE_LABEL:
				current_file = cxt->files;
				
				while (current_file != NULL && current_file->data != NULL) {
					if (current_label->data.parent == current_file->data) break;
						label_offsets[i] += current_file->data->text.length;
					
					current_file = current_file->next;
				}
				break;
		}
		
		current_label = current_label->next;
	}

	unsigned int lr_offset = 0;

	current_file = cxt->files;
	
	label_ref_linked_list *current_label_ref;
	dasm_label *matching_label;
				
	while (current_file != NULL && current_file->data != NULL) {
		current_label_ref = current_file->data->label_refs;
		matching_label = NULL;
		
		while (current_label_ref) {
			matching_label = NULL;
			matching_label_index = 0;
			
			current_label = cxt->labels;
			
			while (current_label && !matching_label) {
				if (strcmp(current_label_ref->data.label, current_label->data.name) == 0) {
					matching_label = &current_label->data;
					break;
				}
				
				matching_label_index++;
				current_label = current_label->next;
			}
			
			if (cxt->flags & VERBOSE) {
				printf("Label reference in file \"%s\"; referenced label is ``%s\". Position in local buffer is 0x%x, and in output buffer: 0x%x + 0x%x = 0x%x\n",
						current_file->data->absolute_path, current_label_ref->data.label, current_label_ref->data.position, current_label_ref->data.position,
						lr_offset, current_label_ref->data.position + lr_offset);
			}
			
			if (!matching_label) {
				fprintf(
					stderr,
					"%s:%d: Error: referenced label ``%s\'\' is undefined.\n",
					current_label_ref->data.parent->given_path, current_label_ref->data.line_number, current_label_ref->data.label);
				exit(EXIT_FAILURE);
			}
			
			if (cxt->flags & VERBOSE) {
				printf("label found; label ``%s\" has postion 0x%x in local buffer, and offset position 0x%x + 0x%x = 0x%x\n",
					matching_label->name, matching_label->position, matching_label->position,
					label_offsets[matching_label_index], matching_label->position + label_offsets[matching_label_index]);
			}
			
			write_buffer_at(buf, matching_label->position + label_offsets[matching_label_index],
					current_label_ref->data.position + lr_offset);
				
			current_label_ref = current_label_ref->next;
		}
		
		lr_offset += current_file->data->text.length;
		current_file = current_file->next;
	}
	
	return 0;
}

int init_context_labels(dasm_context *cxt)
{
	if (!cxt)
		return BAD_ARGUMENTS;
	
	cxt->labels = NULL;
	
	return SUCCESS;
}

void free_label(dasm_label l)
{
	if (l.name != NULL)
		free(l.name);
}
