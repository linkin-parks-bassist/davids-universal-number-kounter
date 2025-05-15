#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dunkasm.h"

IMPLEMENT_LINKED_LIST(dasm_alias);

void dasm_alias_destructor(dasm_alias a)
{
	free(a.replacee);
	free(a.replacer);
}

int compare_aliases(dasm_alias a, dasm_alias b)
{
	return strcmp(a.replacee, b.replacee);
}

int init_context_aliases(dasm_context *cxt)
{
	if (cxt == NULL)
		return BAD_ARGUMENTS;
	
	add_alias_to_context(cxt, "pk", "sr0");
	add_alias_to_context(cxt, "sp", "sr1");
	
	add_alias_to_context(cxt, "argument", "r8");
	
	add_alias_to_context(cxt, "argument1", "r1");
	add_alias_to_context(cxt, "argument2", "r2");
	add_alias_to_context(cxt, "argument3", "r3");
	add_alias_to_context(cxt, "argument4", "r4");
	add_alias_to_context(cxt, "argument5", "r5");
	add_alias_to_context(cxt, "argument6", "r6");
	add_alias_to_context(cxt, "argument7", "r7");
	add_alias_to_context(cxt, "argument8", "r8");
	
	add_alias_to_context(cxt, "result", "r8");
	
	add_alias_to_context(cxt, "result1", "r1");
	add_alias_to_context(cxt, "result2", "r2");
	add_alias_to_context(cxt, "result3", "r3");
	add_alias_to_context(cxt, "result4", "r4");
	add_alias_to_context(cxt, "result5", "r5");
	add_alias_to_context(cxt, "result6", "r6");
	add_alias_to_context(cxt, "result7", "r7");
	add_alias_to_context(cxt, "result8", "r8");
	
	
	add_alias_to_context(cxt, "ih1", "sr8");
	add_alias_to_context(cxt, "ih2", "sr9");
	add_alias_to_context(cxt, "ih3", "sra");
	add_alias_to_context(cxt, "ih4", "srb");
	add_alias_to_context(cxt, "ih5", "src");
	add_alias_to_context(cxt, "ih6", "srd");
	add_alias_to_context(cxt, "ih7", "sre");
	add_alias_to_context(cxt, "ih8", "srf");

	return 0;
}

void free_alias(dasm_alias a)
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
	
	dasm_alias_linked_list *current = cxt->aliases;
	
	if (current == NULL)
		return BAD_ARGUMENTS;
	
	while (current && strcmp(current->data.replacee, "ih8"))
		current = current->next;
	
	destructor_free_dasm_alias_linked_list(current->next, &dasm_alias_destructor);
	
	current->next = NULL;
	
	/*for (int i = N_DEFAULT_ALIASES; i < cxt->n_aliases; i++) {
		free(cxt->aliases[i].replacee);
		cxt->aliases[i].replacee = NULL;
		free(cxt->aliases[i].replacer);
		cxt->aliases[i].replacer = NULL;
	}*/
	
	return 0;
}
