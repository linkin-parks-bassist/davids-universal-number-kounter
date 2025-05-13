#ifndef ALIAS_H
#define ALIAS_H

#define N_ARGUMENT_ALIASES 8
#define N_RESULT_ALIASES 8
#define N_DEFAULT_ALIASES 4 + N_ARGUMENT_ALIASES + N_RESULT_ALIASES

typedef struct
{
	char* replacee;
	char* replacer;
} alias;

void init_aliases();

void free_alias(alias a);

int init_context_aliases(dasm_context *cxt);
int clear_nondefault_aliases(dasm_context *cxt);

#endif
