#ifndef DUNKASM_LABEL_H
#define DUNKASM_LABEL_H

#define MAX_LABEL_LENGTH 	64
#define MAX_LABELS			1024
#define MAX_LABEL_REFS		1024

#define CODE_LABEL 	  0
#define STRING_LABEL  1

typedef struct {
	struct pa_file *parent;
	char *name;
	unsigned int position;
	int type;
} dasm_label;

typedef struct
{
	struct pa_file *parent;
	char label[MAX_LABEL_LENGTH];
	unsigned int position;
	unsigned int line_number;
} label_ref;

int insert_label_addresses(dasm_context *cxt, dasm_buffer *buf);

void free_label(dasm_label l);

int init_context_labels(dasm_context *context);

#endif
