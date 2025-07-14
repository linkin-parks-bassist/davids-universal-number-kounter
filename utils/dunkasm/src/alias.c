#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dunkasm.h"

IMPLEMENT_LINKED_LIST(dasm_alias);

#define N_RESERVED_KEYWORDS 6

const char *reserved_keywords[N_RESERVED_KEYWORDS] = {"alias", "dealias", "all", "include", "include_first", "word"};

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
	
	add_alias_to_context(cxt, "pk", "sr0", NULL);
	add_alias_to_context(cxt, "sp", "sr1", NULL);
	
	add_alias_to_context(cxt, "argument", "r8", NULL);
	
	add_alias_to_context(cxt, "argument1", "r8", NULL);
	add_alias_to_context(cxt, "argument2", "r9", NULL);
	add_alias_to_context(cxt, "argument3", "ra", NULL);
	add_alias_to_context(cxt, "argument4", "rb", NULL);
	add_alias_to_context(cxt, "argument5", "rc", NULL);
	add_alias_to_context(cxt, "argument6", "rd", NULL);
	add_alias_to_context(cxt, "argument7", "re", NULL);
	add_alias_to_context(cxt, "argument8", "rf", NULL);
	
	add_alias_to_context(cxt, "result", "r8", NULL);
	
	add_alias_to_context(cxt, "result1", "r8", NULL);
	add_alias_to_context(cxt, "result2", "r9", NULL);
	add_alias_to_context(cxt, "result3", "ra", NULL);
	add_alias_to_context(cxt, "result4", "rb", NULL);
	add_alias_to_context(cxt, "result5", "rc", NULL);
	add_alias_to_context(cxt, "result6", "rd", NULL);
	add_alias_to_context(cxt, "result7", "re", NULL);
	add_alias_to_context(cxt, "result8", "rf", NULL);
	
	
	add_alias_to_context(cxt, "interrupt_handler1", "sr8", NULL);
	add_alias_to_context(cxt, "interrupt_handler2", "sr9", NULL);
	add_alias_to_context(cxt, "interrupt_handler3", "sra", NULL);
	add_alias_to_context(cxt, "interrupt_handler4", "srb", NULL);
	add_alias_to_context(cxt, "interrupt_handler5", "src", NULL);
	add_alias_to_context(cxt, "interrupt_handler6", "srd", NULL);
	add_alias_to_context(cxt, "interrupt_handler7", "sre", NULL);
	add_alias_to_context(cxt, "interrupt_handler8", "srf", NULL);
	
	add_alias_to_context(cxt, "PRINT_DIGIT",				"0", NULL);
	add_alias_to_context(cxt, "PRINT_INTEGER",				"1", NULL);
	add_alias_to_context(cxt, "PRINT_INTEGER_DECIMAL",		"2", NULL);
	add_alias_to_context(cxt, "PRINT_INTEGER_HEX",			"3", NULL);
	add_alias_to_context(cxt, "PRINT_INTEGER_BINARY",		"4", NULL);
	
	add_alias_to_context(cxt, "PRINT_ADDR",					"0xfffc", NULL);

	cxt->last_default_alias = dasm_alias_linked_list_tail(cxt->aliases);

	return 0;
}

void free_alias(dasm_alias a)
{
	if (a.replacee != NULL)
		free(a.replacee);
	if (a.replacer != NULL)
		free(a.replacee);
}

int remove_alias_from_context(dasm_context *cxt, const char *alias)
{
	if (!valid_dasm_context(cxt) || alias == NULL)
		return BAD_ARGUMENTS;
	
	dasm_alias_linked_list *current = cxt->last_default_alias;
	dasm_alias_linked_list *prev;
	
	if (current == NULL)
		return BAD_ARGUMENTS;
	
	prev = current;
	current = current->next;
	int found = 0;
	
	while (current)
	{
		if (current->data.replacee == NULL)
		{
			return BAD_ARGUMENTS;
		}
		if (strcmp(current->data.replacee, alias) == 0)
		{
			prev->next = current->next;
			dasm_alias_destructor(current->data);
			free(current);
			current = prev->next;
			found = 1;
		}
		else
		{
			prev = current;
			current = current->next;
		}
	}
	
	if (found == 1)
		return SUCCESS;
	else
		return NOT_FOUND;
}

int clear_nondefault_aliases(dasm_context *cxt)
{
	if (!valid_dasm_context(cxt))
		return BAD_ARGUMENTS;
	
	if (cxt->last_default_alias == NULL)
		return BAD_ARGUMENTS;
	
	destructor_free_dasm_alias_linked_list(cxt->last_default_alias->next, &dasm_alias_destructor);
	
	cxt->last_default_alias->next = NULL;
	
	return 0;
}

int clear_nondefault_aliases_with_parent(dasm_context *cxt, dasm_file *parent)
{
	if (!valid_dasm_context(cxt) || parent == NULL)
		return BAD_ARGUMENTS;
	
	dasm_alias_linked_list *current = cxt->last_default_alias;
	dasm_alias_linked_list *prev;
	
	if (current == NULL)
		return BAD_ARGUMENTS;
	
	prev = current;
	current = current->next;
	
	while (current)
	{
		if (current->data.parent == parent)
		{
			prev->next = current->next;
			dasm_alias_destructor(current->data);
			free(current);
		}
		else
		{
			prev = current;
		}
		
		current = current->next;
	}
	
	return SUCCESS;
}

int is_alias_allowed(dasm_context *cxt, char *name)
{
	if (!cxt || !name)
		return BAD_ARGUMENTS;
	
	for (int i = 0; i < N_RESERVED_KEYWORDS; i++)
	{
		if (strcmp(name, reserved_keywords[i]) == 0)
			return ALIAS_RESERVED;
	}
	
	for (int i = 0; i < N_INSTR; i++)
	{
		if (strcmp(dunk_instrs[i].name, name) == 0)
			return ALIAS_RESERVED;
	}
	
	dasm_alias_linked_list *alias = cxt->aliases;
	int built_in = 1;
	
	while (alias)
	{
		if (strcmp(alias->data.replacee, name) == 0)
		{
			if (built_in == 1)
				return ALIAS_BUILT_IN;
			else
				return ALIAS_TAKEN;
		}
		
		if (alias == cxt->last_default_alias)
			built_in = 0;
		alias = alias->next;
	}
	
	return ALIAS_FREE;
}
