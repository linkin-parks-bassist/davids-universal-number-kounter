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
