#ifndef ALIAS_H
#define ALIAS_H

#define N_ARGUMENT_ALIASES 8
#define N_RESULT_ALIASES 8
#define N_DEFAULT_ALIASES 4 + N_ARGUMENT_ALIASES + N_RESULT_ALIASES

#define NOT_FOUND 8

#define ALIAS_FREE		STD_RETVAL_MAX + 1
#define ALIAS_RESERVED  STD_RETVAL_MAX + 2
#define ALIAS_BUILT_IN	STD_RETVAL_MAX + 3
#define ALIAS_TAKEN		STD_RETVAL_MAX + 4

typedef struct
{
	char* replacee;
	char* replacer;
	dasm_file *parent;
	dasm_line *line;
} dasm_alias;

DECLARE_LINKED_LIST(dasm_alias);

typedef dasm_alias_linked_list dasm_alias_linked_list;

void init_aliases();

void free_alias(dasm_alias a);

void dasm_alias_destructor(dasm_alias a);
int compare_aliases(dasm_alias a, dasm_alias b);

int init_context_aliases(dasm_context *cxt);
int remove_alias_from_context(dasm_context *cxt, const char *alias);
int clear_nondefault_aliases(dasm_context *cxt);
int clear_nondefault_aliases_with_parent(dasm_context *cxt, dasm_file *parent);

int is_alias_allowed(dasm_context *cxt, char *name);

#endif
