#ifndef ALIAS_H
#define ALIAS_H

#define N_ARGUMENT_ALIASES 8
#define N_RESULT_ALIASES 8
#define N_DEFAULT_ALIASES 4 + N_ARGUMENT_ALIASES + N_RESULT_ALIASES

typedef struct
{
	char* replacee;
	char* replacer;
} dasm_alias;

DECLARE_LINKED_LIST(dasm_alias);

typedef dasm_alias_linked_list dasm_alias_linked_list;

void init_aliases();

void free_alias(dasm_alias a);

void dasm_alias_destructor(dasm_alias a);
int compare_aliases(dasm_alias a, dasm_alias b);

int init_context_aliases(dasm_context *cxt);
int clear_nondefault_aliases(dasm_context *cxt);

#endif
