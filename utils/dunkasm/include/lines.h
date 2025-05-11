#ifndef DUNK_LINES_H
#define DUNK_LINES_H

#define MAX_LINE_LENGTH 1024

typedef struct
{
	int line_number;
	char *raw_line;
	char** tokens;
	int token_count;
} line_data_struct;

typedef struct line_data_node
{
	line_data_struct* data;
	struct line_data_node* next;
} line_data_node;

//DECLARE_LINKED_LIST(line_data_struct);

void free_line_data(line_data_struct* line_data);
void free_list(line_data_node* head);

int process_line(line_data_struct line, struct pa_file *file, struct dasm_context *cxt);

#endif
