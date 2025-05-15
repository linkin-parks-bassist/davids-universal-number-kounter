#ifndef DUNKASM_LABEL_H
#define DUNKASM_LABEL_H

#define MAX_LABEL_LENGTH 	512
#define MAX_LABELS			1024
#define MAX_LABEL_REFS		256

#define CODE_LABEL 	  0
#define STRING_LABEL  1

typedef struct {
	struct dasm_file *parent;
	char *name;
	unsigned int position;
	unsigned int line;
	int type;
} dasm_label;

typedef struct
{
	struct dasm_file *parent;
	char label[MAX_LABEL_LENGTH];
	unsigned int position;
	unsigned int line_number;
} label_ref;

DECLARE_LINKED_LIST(dasm_label);
DECLARE_LINKED_LIST(label_ref);

// Just so my IDE knows to highlight it lol
typedef struct dasm_label_linked_list dasm_label_linked_list;
typedef struct  label_ref_linked_list  label_ref_linked_list;

int insert_label_addresses(dasm_context *cxt, dasm_buffer *buf);

int compare_label_names(dasm_label a, dasm_label b);

void dasm_label_destructor(dasm_label label);
void free_label(dasm_label l);

int init_context_labels(dasm_context *context);

#endif
