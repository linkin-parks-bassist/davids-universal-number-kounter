#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "dunkasm.h"

parameter parse_parameter(dasm_context *cxt, const char* input)
{
	/* Initialize result with type INVALID so that
	 * if malformed input is encountered, it can simply
	 * be returned, indicating to the calling function
	 * that the input was malformed */
	parameter result;
	result.type   = INVALID; 
	result.value  = 0;
	result.offset = 0;
	
	if (!valid_dasm_context(cxt) || input == NULL)
		return result;
	
	char tmp;
	char* plus = NULL;
	const char* inside = NULL;
	const char* close_paren = NULL;

	size_t len = strlen(input), leng;
	char sub_exp[len];
	parameter sub_result;
	
	// Check for some basic types first
	if (is_number(input))
	{
		result.type = CONSTANT;
		result.value = parse_number(input);
		return result;
	}
	
	if (is_char(input))
	{
		result.type = CONSTANT;
		result.value = parse_char(input);
		return result;
	}
	
	if (is_string(input))
	{
		/* If the input is a string, it will be handled
		 * by the calling function. Simply return the result
		 * type so that this can be handled currectly */
		result.type = STRING_P;
		return result;
	}
	
	// "rN" form
	if (input[0] == 'r' && is_hnumber(&input[1]) && len == 2)
	{
		result.type = REGISTER;
		result.value = strtol(&input[1], NULL, 16);
		return result;
	}
	
	// "srN" form
	if (input[0] == 's' && input[1] == 'r' && is_hnumber(&input[2]) && len == 3)
	{
		result.type = S_REGISTER;
		result.value = strtol(&input[2], NULL, 16);
		return result;
	}
	
	if (input[0] == '*') // Pointer
	{
		/* If the pointer is not parenthesised, copy the sub-expression
		 * consisting of everything after the "*"; this will be passed
		 * to a recursive call to parse_expression */
		if (input[1] != '(')
		{
			strcpy(sub_exp, &input[1]);
		}
		else // If the pointer is parenthesised, extract and handle
		{
			// Reject if it doesn't end with a closing paren
			if (input[len - 1] != ')')
				return result;
			
			strncpy(sub_exp, &input[2], len - 2); // Copy the sub-expression
			sub_exp[len - 3] = 0; // ... and null-terminate it (strncpy doesn't)
			
			/* Find any "+" or "-" in the sub-expression.
			 * Note: this returns a pointer to a char WITHIN sub_exp
			 * (or NULL if there is none). Modifying *plus modifies sub_exp */
			plus = &sub_exp[strcspn(sub_exp, "+-")];

			/* If there is one, i.e., the input is of the form *(x+y)
			 * recurse to parse y, and set that as the offset */
			if (plus != NULL)
			{
				// Recurse to parse y. This should be simply a number 
				sub_result = parse_parameter(cxt, &plus[1]);

				// ... but if it's not, return the (invalid) result
				if (sub_result.type != CONSTANT)
					return result;

				// Set the offset as the value of the returned parameter
				result.offset = sub_result.value;
				
				/* Null terminate sub_expr at the "+", leaving just the x
				 * for the next call to parse_parameter to handle */
				*plus = 0;
			}
		}

		// Now recurse to parse the sub-expression
		sub_result = parse_parameter(cxt, sub_exp);

		// If the sub-expression is malformed, reject
		if (sub_result.type == INVALID)
			return result;
		
		// Reject constants with offsets.
		if (sub_result.type == CONSTANT && result.offset != 0)
			return result;

		/* With everything having gone well, validate result, carrying through the
		 * type and value from the sub-expression, with the pointer flags tacked on */
		result.type  = sub_result.type | POINTER;
		result.value = sub_result.value;
		return result;
	}
	
	// If all checks so far have failed, check the list of aliases
	dasm_alias_linked_list *current = cxt->aliases;
	
	while (current != NULL)
	{
		if (strcmp(current->data.replacee, input) == 0)
			break;
		
		current = current->next;
	}
	
	// If we have a match, return the result of parsing the replacer of the alias
	if (current != NULL)
		return parse_parameter(cxt, current->data.replacer);
	
	/* Otherwise, if the input would be a valid label, assume it is such.
	 * If this label is never defined, this will throw an error later on.
	 * As with strings, further processing will be handled by the calling
	 * function, so simply indicate a latel and pass it back */
	if (is_label(input))
		result.type = LABEL_P;
	
	// If no branches have been followed at this point, result.type is still INVALID
	return result;
}

int handle_label_parameter_no_offset(dasm_context *cxt, dasm_file *file, parameter *param, const char *str, unsigned int line_number)
{
	if (!valid_dasm_context(cxt) || !valid_dasm_file(file) || param == NULL)
		return BAD_ARGUMENTS;
	
	param->type = CONSTANT;
	
	add_label_ref_to_file(file, str, file->text.position, line_number);
	
	return SUCCESS;
}
int handle_label_parameter(dasm_context *cxt, dasm_file *file, parameter *param, const char *str, unsigned int line_number)
{
	if (!valid_dasm_context(cxt) || !valid_dasm_file(file) || param == NULL)
		return BAD_ARGUMENTS;
	
	param->type = CONSTANT;
	
	add_label_ref_to_file(file, str, file->text.position + 1, line_number);
	
	return SUCCESS;
}

int handle_string_parameter(dasm_context *cxt, dasm_file *file, parameter *param, const char *str, unsigned int line_number)
{
	if (!valid_dasm_context(cxt) || !valid_dasm_file(file) || param == NULL)
		return BAD_ARGUMENTS;
	
	dasm_label *label = add_string_to_context(cxt, str, file, file->text.position, line_number);
	
	if (!label)
		return MEMORY_FAILURE;
	
	param->type = CONSTANT;
	param->value = file->text.position;
	
	add_label_ref_to_file(file, label->name, file->text.position + 1, line_number);
	
	return SUCCESS;
}
