#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "dunkasm.h"

IMPLEMENT_LINKED_LIST(dasm_inclusion);

int cmp_inclusions(dasm_inclusion a, dasm_inclusion b)
{
	return strcmp(a.abspath, b.abspath);
}

dasm_inclusion new_inclusion(dasm_file *parent, char *fname, dasm_line line, int token)
{
	dasm_inclusion result;
	char abs_path_buf[PATH_MAX];
	
	result.parent = parent;
	result.given_path = fname;
	result.line = line;
	result.token = token;
	
	if (realpath(fname, abs_path_buf) == NULL)
	{
		perror(fname);
		exit(EXIT_FAILURE);
	}
	
	result.abspath = strdup(abs_path_buf);
	
	return result;
}

void dasm_inclusion_destructor(dasm_inclusion a)
{
	if (a.abspath)
		free(a.abspath);
}
