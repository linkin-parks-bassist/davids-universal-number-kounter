#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dunkasm.h"

int insert_label_addresses(dasm_context *cxt, dasm_buffer *buf)
{
	if (!valid_dasm_context)
		return BAD_ARGUMENTS;
	
	int label_index;
	int found;
	
	unsigned int label_offsets[cxt->n_labels];
	
	for (int i = 0; i < cxt->n_labels; i++) {
		label_offsets[i] = 0;
		
		switch (cxt->labels[i].type) {
			case STRING_LABEL:
				for (int n = 0; n < cxt->n_pa_files; n++) {
					label_offsets[i] += cxt->pa_files[n]->text.length;
				}
				
				for (int n = 0; n < cxt->n_pa_files; n++) {
					if (cxt->labels[i].parent == cxt->pa_files[n])
						break;
					label_offsets[i] += cxt->pa_files[n]->strings.length;
				}
				break;
				
			case CODE_LABEL:
				for (int n = 0; n < cxt->n_pa_files; n++) {
					if (cxt->labels[i].parent == cxt->pa_files[n]) break;
					label_offsets[i] += cxt->pa_files[n]->text.length;
				}
				break;
		}
	}

	unsigned int lr_offset = 0;

	for (int n = 0; n < cxt->n_pa_files; n++) {
		for (int i = 0; i < cxt->pa_files[n]->n_label_refs; i++) {
			found = 0;
			for (int j = 0; j < cxt->n_labels && !found; j++) {
				if (strcmp(cxt->pa_files[n]->label_refs[i].label, cxt->labels[j].name) == 0) {
					label_index = j;
					found = 1;
				}
			}
			
			printf("Label reference in file %d (%s); referenced label is ``%s\". Position in local buffer is 0x%x, and in output buffer: 0x%x + 0x%x = 0x%x\n",
				n, cxt->pa_files[n]->fname, cxt->pa_files[n]->label_refs[i].label, cxt->pa_files[n]->label_refs[i].position, cxt->pa_files[n]->label_refs[i].position,
				 lr_offset, cxt->pa_files[n]->label_refs[i].position + lr_offset);
			
			if (!found) {
				fprintf(
					stderr,
					"Error: label ``%s\'\', referenced on line %i, was never defined.\n",
					cxt->pa_files[n]->label_refs[i].label,
					cxt->pa_files[n]->label_refs[i].line_number);
				exit(EXIT_FAILURE);
			}
			
			printf("label found; label ``%s\" has postion 0x%x in local buffer, and offset position 0x%x + 0x%x = 0x%x\n",
				cxt->labels[label_index].name, cxt->labels[label_index].position, cxt->labels[label_index].position,
				label_offsets[label_index], cxt->labels[label_index].position + label_offsets[label_index]);
			
			write_buffer_at(buf,
				cxt->labels[label_index].position + label_offsets[label_index],
				cxt->pa_files[n]->label_refs[i].position + lr_offset);
		}
		
		lr_offset += cxt->pa_files[n]->text.length;
	}
	
	return 0;
}

int init_context_labels(dasm_context *cxt)
{
	if (!cxt)
		return BAD_ARGUMENTS;
	
	cxt->n_labels = 0;
	
	for (int i = 0; i < MAX_LABELS; i++) {
		cxt->labels[i].name = NULL;
		cxt->labels[i].parent = NULL;
	}
}

void free_label(dasm_label l)
{
	if (l.name != NULL)
		free(l.name);
}
