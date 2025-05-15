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

parameter parse_parameter(dasm_context *cxt, const char* input)
{
	parameter result = { 0, 0, 0 }; // Initialize result with default values
	
	if (!valid_dasm_context(cxt)){
		result.type = INVALID;
		return result;
	}
	
	char tmp;
	char* plus = NULL;
	const char* inside = NULL;
	const char* close_paren = NULL;

	size_t len, leng;
	char* sub_exp;
	parameter sub_result;

	if (input[0] == '\"') {
		result.type = STRING_P;
		return result;
	} else if (input[0] == '*') {
		if (input[1] == '(') {
			inside = &input[2];
			close_paren = strchr(inside, ')');

			if (close_paren == NULL) {
				result.type = INVALID;
				return result;
			}
			if (close_paren[1] != 0) {
				
				result.type = INVALID;
				return result;
			}

			len = close_paren - inside;
			sub_exp = strndup(inside, len);

			plus = &sub_exp[strcspn(sub_exp, "+-")];

			if (plus != NULL) {
				if (plus[0] == '+') {
					sub_result = parse_parameter(cxt, &plus[1]);
				} else {
					sub_result = parse_parameter(cxt, plus);
				}

				if (sub_result.type != CONSTANT) {
					
					result.type = INVALID;
					return result;
				}
				if (sub_result.offset != 0) {
					
					result.type = INVALID;
					return result;
				}

				result.offset = sub_result.value;
				*plus = 0;
			}

		} else {
			inside = &input[1];
			len = strlen(inside);
			sub_exp = strndup(inside, len);
		}

		sub_result = parse_parameter(cxt, sub_exp);
		free(sub_exp);

		if (input[1] != '(' && sub_result.offset != 0) {
			result.type = INVALID;
			return result;
		}

		result.type = sub_result.type | POINTER;
		result.value = sub_result.value;
	} else {
		if (input[0] == 's' && input[1] == 'r' && is_number(&input[2])) {
			// "srN" form
			result.type = S_REGISTER;
			result.value = parse_number(&input[2]);
		} else if (input[0] == 'r' && is_number(&input[1])) {
			// "rN" form
			result.type = REGISTER;
			result.value = parse_number(&input[1]);
		} else if (is_number(input)) {
			result.type = CONSTANT;
			result.value = parse_number(input);
		} else if (is_char(input)) {
			result.type = CONSTANT;
			result.value = parse_char(input);
		} else {
			dasm_alias_linked_list *current = cxt->aliases;
			
			while (current != NULL) {
				if (strcmp(current->data.replacee, input) == 0)
					break;
				
				current = current->next;
			}
			
			if (current != NULL)
				return parse_parameter(cxt, current->data.replacer);
			
			if (is_label(input)) {
				result.type = LABEL_P;
			} else {
				result.type = INVALID;
			}
		}
	}

	if (result.offset != 0) {
		result.type = result.type | W_OFFSET;
	}

	return result;
}

int handle_label_parameter(dasm_context *cxt, dasm_file *file, parameter *param, const char *str, unsigned int line_number)
{
	if (!valid_dasm_context(cxt) || !valid_dasm_file(file) || param == NULL) {
		return BAD_ARGUMENTS;
	}
	
	param->type = CONSTANT;
	
	add_label_ref_to_file(file, str, file->text.position + 1, line_number);
	
	return SUCCESS;
}

int handle_string_parameter(dasm_context *cxt, dasm_file *file, parameter *param, const char *str, unsigned int line_number)
{
	if (!valid_dasm_context(cxt) || !valid_dasm_file(file) || param == NULL) {
		return BAD_ARGUMENTS;
	}
	
	dasm_label *label = add_string_to_context(cxt, str, file, file->text.position, line_number);
	
	if (!label)
		return MEMORY_FAILURE;
	
	param->type = CONSTANT;
	param->value = file->text.position;
	
	add_label_ref_to_file(file, label->name, file->text.position + 1, line_number);
	
	return SUCCESS;
}
