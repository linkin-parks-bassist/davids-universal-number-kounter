#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "dunkasm.h"

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
		default: return 0;
	}
}

uint16_t parse_char(const char *str) {
	if (!str) return 0;
	
	if (str[1] == '\\')
		return (uint16_t)unescape(str[2]);
	
	return str[1];
}

long parse_number(const char* str)
{
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
	}
}

parameter parse_parameter(const char* input)
{
	parameter result = { 0, 0, 0 }; // Initialize result with default values
	char tmp;
	char* plus = NULL;
	const char* inside = NULL;
	const char* close_paren = NULL;

	size_t len, leng;     // = close_paren - inside;
	char* sub_exp;        // = strndup(inside, len);
	parameter sub_result; // = parse_string(sub_expr);

	//printf("Parsing parameter ``%s\"\n", input);

	if (input[0] == '*') {
		//printf("It's a pointer, ");
		if (input[1] == '(') {
			//printf("parenthesised, with inner expression");
			inside = &input[2];
			close_paren = strchr(inside, ')');

			if (close_paren == NULL) {
				//printf("... but not closing paren :(\n");
				result.type = INVALID;
				return result;
			}
			if (close_paren[1] != 0) {
				//printf("... but does not end after its closing paren :(\n");

				result.type = INVALID;
				return result;
			}

			len = close_paren - inside;
			sub_exp = strndup(inside, len);

			//printf(" \"%s\", ", sub_exp);

			plus = &sub_exp[strcspn(sub_exp, "+-")];

			if (plus != NULL) {
				if (plus[0] == '+') {
					sub_result = parse_parameter(&plus[1]);
				} else {
					sub_result = parse_parameter(plus);
				}

				if (sub_result.type != CONSTANT) {
					//printf("... but it contains an offset which is not a constant :(\n");

					result.type = INVALID;
					return result;
				}
				if (sub_result.offset != 0) {
					//printf("... but it contains a double-offset :(\n");

					result.type = INVALID;
					return result;
				}

				result.offset = sub_result.value;
				//printf(" which has offset %i\n", result.offset);
				*plus = 0;
			}

		} else {
			//printf("non-parenthesised, ");
			inside = &input[1];
			len = strlen(inside);
			sub_exp = strndup(inside, len);
		}

		//printf("with sub-expression %s\n", sub_exp);
		sub_result = parse_parameter(sub_exp);
		free(sub_exp);

		if (input[1] != '(' && sub_result.offset != 0) {
			//printf("... which has an offset, which is obviously not valid without parenthesisation >:/\n", sub_exp);
			result.type = INVALID;
			return result;
		}

		result.type = sub_result.type | POINTER;
		result.value = sub_result.value;
	} else {
		//printf("It's not a pointer... ");
		if (input[0] == 's' && input[1] == 'r' && is_number(&input[2])) {
			// "srN" form
			result.type = S_REGISTER;
			result.value = parse_number(&input[2]);
			//printf("it's a special register !\n");
		} else if (input[0] == 'r' && is_number(&input[1])) {
			// "rN" form
			//printf("it's a register !\n");
			result.type = REGISTER;
			result.value = parse_number(&input[1]);
		} else if (is_number(input)) {
			result.type = CONSTANT;
			result.value = parse_number(input);
			//printf("it's a number !\n");
		} else if (is_char(input)) {
			result.type = CONSTANT;
			result.value = parse_char(input);
		} else {
			for (int i = 0; i < num_aliases; i++) {
				//printf("Checking aliases...\n");
				if (strcmp(input, aliases[i].replacee) == 0) {
					//printf("Ah! an alias! I will parse %s in place of %s\n",
                 //aliases[i].replacer,
                 //input);
					return parse_parameter(aliases[i].replacer);
				}
			}
			//printf("idk what this is...\n");
			result.type = INVALID;
		}
	}

	if (result.offset != 0) {
		result.type = result.type | W_OFFSET;
	}

	return result;
}
