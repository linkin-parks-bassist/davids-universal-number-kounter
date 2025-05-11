#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dunkasm.h"


int n_aliases;
alias aliases[MAX_N_ALIASES];


int init_context_aliases(dasm_context *cxt)
{
	if (cxt == NULL)
		return BAD_ARGUMENTS;
	
	cxt->n_aliases = N_DEFAULT_ALIASES;
	
	cxt->aliases[0].replacee = malloc(strlen("pk") + 1);
	strcpy(cxt->aliases[0].replacee, "pk");

	cxt->aliases[0].replacer = malloc(strlen("sr0") + 1);
	strcpy(cxt->aliases[0].replacer, "sr0");

	cxt->aliases[1].replacee = malloc(strlen("sp") + 1);
	strcpy(cxt->aliases[1].replacee, "sp");

	cxt->aliases[1].replacer = malloc(strlen("sr1") + 1);
	strcpy(cxt->aliases[1].replacer, "sr1");

	cxt->aliases[2].replacee = malloc(strlen("argument") + 1);
	strcpy(cxt->aliases[2].replacee, "argument");

	cxt->aliases[2].replacer = malloc(strlen("*(sr1+1)") + 1);
	strcpy(cxt->aliases[2].replacer, "*(sr1+1)");

	cxt->aliases[3].replacee = malloc(strlen("result") + 1);
	strcpy(cxt->aliases[3].replacee, "result");

	cxt->aliases[3].replacer = malloc(strlen("*(sr1+1)") + 1);
	strcpy(cxt->aliases[3].replacer, "*(sr1+1)");

	int arslen = 9;
	char buf[arslen];
	for (int i = 0; i < N_ARGUMENT_ALIASES; i++) {
		sprintf(buf, "argument%d", i);
		cxt->aliases[i+4].replacee = malloc(sizeof(char) * arslen);
		strcpy(cxt->aliases[i+4].replacee, buf);
		
		sprintf(buf, "*(sr1+%d)", i);
		cxt->aliases[i+4].replacer = malloc(sizeof(char) * arslen);
		strcpy(cxt->aliases[i+4].replacer, buf);
	}

	for (int i = cxt->n_aliases; i < MAX_N_ALIASES; i++) {
		cxt->aliases[i].replacee = NULL;
		cxt->aliases[i].replacer = NULL;
	}

	return 0;
}

void free_alias(alias a)
{
	if (a.replacee != NULL)
		free(a.replacee);
	if (a.replacer != NULL)
		free(a.replacee);
}

int clear_nondefault_aliases(dasm_context *cxt)
{
	if (!valid_dasm_context(cxt))
		return BAD_ARGUMENTS;
	
	for (int i = N_DEFAULT_ALIASES; i < cxt->n_aliases; i++) {
		free(cxt->aliases[i].replacee);
		cxt->aliases[i].replacee = NULL;
		free(cxt->aliases[i].replacer);
		cxt->aliases[i].replacer = NULL;
	}
	
	cxt->n_aliases = N_DEFAULT_ALIASES;
	
	return 0;
}
