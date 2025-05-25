#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "dunkasm.h"

IMPLEMENT_LINKED_PTR_LIST(char);
IMPLEMENT_LINKED_LIST(dasm_string);

int is_number(const char* str)
{
	if (str[0] == '-') {
		str++; // skip the minus sign
	}
	if (str[0] == '0') {
		if (str[1] == 'x' || str[1] == 'X') { // hexadecimal
			str += 2;
			while (*str) {
				if (!isxdigit(*str))
					return 0;
				str++;
			}
			return 1;
		} else if (str[1] == 'b' || str[1] == 'B') { // binary
			str += 2;
			while (*str) {
				if (*str != '0' && *str != '1')
					return 0;
				str++;
			}
			return 1;
		}
	}
	while (*str) {
		if (!isdigit(*str))
			return 0;
		str++;
	}
	return 1;
}

int is_dnumber(const char* str)
{
	if (str[0] == '-') {
		str = &str[1];
	}
	for (int i = 0; i < strlen(str); i++) {
		if (!(str[i] >= '0' && str[i] <= '9')) {
			return 0;
		}
	}
	return 1;
}

int is_hnumber(const char* str)
{
	for (int i = 0; i < strlen(str); i++) {
		if (!(str[i] >= '0' && str[i] <= '9') &&
				!(str[i] >= 'a' && str[i] <= 'f')) {
			return 0;
		}
	}
	return 1;
}

int is_bnumber(const char* str)
{
	for (int i = 0; i < strlen(str); i++) {
		if (!(str[i] >= '0' && str[i] <= '1')) {
			return 0;
		}
	}
	return 1;
}

int is_char(const char* str)
{
	if (!str) return 0;
	
	int len = strlen(str);
	if (len < 3) return 0;
	
	if (str[0] != '\'') return 0;
	
	if (str[1] == '\\') {
		if (len != 4) return 0;
		if (str[3] != '\'') return 0;
	} else {
		if (str[2] != '\'') return 0;
	}
	
	return 1;
}

int is_string(const char* str)
{
	if (!str) return 0;
	
	if (str[0] != '\"')
		return 0;
	
	int len = strlen(str);
	
	if (str[len - 1] != '\"')
		return 0;
	
	int escape = 0;
	int string_length = 0;
	
	for (int i = 1; i < len - 1; i++)
	{
		if (escape)
		{
			escape = 0;
		}
		else
		{
			if (str[i] == '\\')
				escape = 1;
			else if (str[i] == '\"')
				break;
		}
		
		string_length++;
	}
	
	return string_length == (len - 2);
}

int is_label(const char *str) {
	if (str == NULL)
		return 0;
	
	int len = strlen(str);
	
	if (len == 0)
		return 0;
	
	for (int i = 0; i < len; i++) {
		if ('a' <= str[i] <= 'z')
			continue;
		
		if ('A' <= str[i] <= 'Z')
			continue;
		
		if ('0' <= str[i] <= '9')
			continue;
		
		if (str[i] == '_')
			continue;
		
		return 0;
	}
	
	return 1;
}

uint16_t parse_char(const char *str) {
	if (!str) return SUCCESS;
	
	if (str[1] == '\\')
		return (uint16_t)unescape(str[2]);
	
	return str[1];
}

long parse_number(const char* str)
{
	if (str == NULL) {
		fprintf(stderr, "Error: NULL pointer passed to function `parse_number'\n");
		exit(EXIT_FAILURE);
	}
	
	if (str[0] == '+')
		return parse_number(&str[1]);
	if (str[0] == '-')
		return -parse_number(&str[1]);

	if (str[0] == '0') {
		if (str[1] == 'x' && is_hnumber(&str[2])) {
			return strtol(&str[2], NULL, 16);
		} else if (str[1] == 'b' && is_bnumber(&str[2])) {
			return strtol(&str[2], NULL, 2);
		}
	} else if (is_dnumber(str)) {
		return strtol(str, NULL, 10);
	} else {
		fprintf(stderr, "Error: non-number string \"%s\" passed to function `parse_number'\n", str);
		exit(EXIT_FAILURE);
	}
	
	return 0;
}

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


dasm_string new_dasm_string(const char *str, dasm_label *label)
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
	
	res.label = label;
	
	return res;
}

void dasm_string_destructor(dasm_string str)
{
	free(str.value);
}

int free_dasm_string(dasm_string res)
{
	free(res.value);
	return SUCCESS;
}
