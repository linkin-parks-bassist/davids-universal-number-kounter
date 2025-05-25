#ifndef DUNK_LINES_H
#define DUNK_LINES_H

#define MAX_LINE_LENGTH 1024

typedef struct
{
	int line_number;
	char *raw_line;
	char** tokens;
	int n_tokens;
} dasm_line;

DECLARE_LINKED_LIST(dasm_line);

typedef dasm_line_linked_list dasm_line_linked_list;


dasm_line new_dasm_line(const char *src, unsigned int line_number);
void dasm_line_destructor(dasm_line line);

void strip_comments(dasm_line *line);

int process_line(dasm_line line, dasm_file *file, struct dasm_context *cxt, int flags);

#endif
