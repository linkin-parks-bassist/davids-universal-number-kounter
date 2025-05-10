#ifndef DUNK_LINES_H
#define DUNK_LINES_H

#define MAX_LINE_LENGTH 	1024

typedef struct
{
	int line_number;
	char *raw_line;
	char** tokens;
	int token_count;
} line_data_str;

typedef struct line_data_node
{
	line_data_str* data;
	struct line_data_node* next;
} line_data_node;

void free_line_data(line_data_str* line_data);
void free_list(line_data_node* head);

void process_line(line_data_str line, const char* fname);

#endif
