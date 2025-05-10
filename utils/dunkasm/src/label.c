#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dunkasm.h"

char labels[MAX_LABELS][MAX_LABEL_LENGTH];
uint16_t label_positions[MAX_LABELS];
label_ref label_refs[MAX_LABEL_REFS];

void insert_label_addresses()
{
	int label_index;
	int found;

	for (int i = 0; i < n_label_refs; i++) {
		found = 0;
		for (int j = 0; j < n_labels && !found; j++) {
			if (strcmp(label_refs[i].label, labels[j]) == 0) {
				label_index = j;
				found = 1;
			}
		}

		if (!found) {
			fprintf(
				stderr,
				"Error: label ``%s\'\', referenced on line %i, was never defined.\n",
				label_refs[i].label,
				label_refs[i].line_number);
			exit(EXIT_FAILURE);
		}
		
		current_position = label_refs[i].position + 1;
		buffer_word(label_positions[label_index]);
	}
}
