#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "dunkasm.h"

char unescape(char token)
{
	switch (token) {
		case 'a': return '\a';
		case 'b': return '\b';
		case 'f': return '\f';
		case 'n': return '\n';
		case 'r': return '\r';
		case 't': return '\t';
		case 'v': return '\v';
		case '\\': return '\\';
		case '\'': return '\'';
		case '\"': return '\"';
		case '\?': return '\?';
		default: return SUCCESS;
	}
}


dasm_string new_dasm_string(const char *str, int label_n)
{
	dasm_string res;
	if (str == NULL) {
		res.value = NULL;
		return res;
	}
	
	int len = strlen(str);
	res.value = malloc(len * sizeof(uint16_t));
	
	int p = 0;
	
	for (int i = 1; i < len; i++) {
		if (str[i] == '\"') {
			break;
		} else if (str[i] == '\\') {
			i++;
			res.value[p] = (uint16_t)unescape(str[i]);
			p++;
		} else {
			res.value[p] = (uint16_t)str[i];
			p++;
		}
	}
	
	res.value[p] = 0;
	
	res.label_n = label_n;
	
	return res;
}

int free_dasm_string(dasm_string res)
{
	free(res.value);
}

int handle_string_parameter(dasm_context *cxt, pa_file *file, parameter *param, const char *str, unsigned int line_number)
{
	if (!valid_dasm_context(cxt) || !valid_pa_file(file) || param == NULL) {
		return BAD_ARGUMENTS;
	}
	
	int label_n = add_string_to_context(cxt, str, file, file->text.position);
	
	if (label_n < 0)
		return -label_n;
	
	param->type = CONSTANT;
	param->value = file->text.position;
	
	add_label_ref_to_file(file, cxt->labels[label_n].name, file->text.position + 1, line_number);
	
	return SUCCESS;
}
