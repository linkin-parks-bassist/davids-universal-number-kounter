#ifndef DASM_INCLUSION_H
#define DASM_INCLUSION_H

struct dasm_inclusion
{
	dasm_file *parent;
	dasm_line line;
	int token;
	char *given_path;
	char *abspath;
};

DECLARE_LINKED_LIST(dasm_inclusion);

dasm_inclusion new_inclusion(dasm_file *parent, char *fname, dasm_line line, int token);
int cmp_inclusions(dasm_inclusion a, dasm_inclusion b);

void dasm_inclusion_destructor(dasm_inclusion a);

#endif
