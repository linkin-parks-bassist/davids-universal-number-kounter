#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "dunkasm.h"


line_data_node *tokenize_file(const char *input_path) {
	FILE* input_file = fopen(input_path, "r");
	if (!input_file) {
		perror("Error opening input file");
		exit(EXIT_FAILURE);
	}

	line_data_node *head = NULL, *tail = NULL;
	char line_str[MAX_LINE_LENGTH];
	int line_number = 1;

	while (fgets(line_str, sizeof(line_str), input_file)) {
		char* trimmed_line = line_str;
		while (isspace((unsigned char)*trimmed_line))
			trimmed_line++;
		if (*trimmed_line == '\0') {
			line_number++;
			continue;
		}

		line_data_str* line_data = tokenize_line(trimmed_line, line_number);

		line_data_node* new_node = malloc(sizeof(line_data_node));
		new_node->data = line_data;
		new_node->next = NULL;
		if (!head) {
			head = tail = new_node;
		} else {
			tail->next = new_node;
			tail = new_node;
		}

		line_number++;
	}
	
	fclose(input_file);
	return head;
}

line_data_str *tokenize_line(char* line, int line_number)
{
	line_data_str *line_data = malloc(sizeof(line_data_str));
	if (!line_data) {
		perror("Memory allocation failed");
		exit(EXIT_FAILURE);
	}
	line_data->raw_line = strdup(line);
	line_data->line_number = line_number;
	line_data->tokens = tokenize_string(line, &line_data->token_count);

	
	return line_data;
}

char **tokenize_string(const char *src, int *n_tokens) {
	if (src == NULL || n_tokens == NULL)
		return NULL;
	
	int ta_size = 8;
	char **tokens = malloc(sizeof(char*) * ta_size);
	
	int len = strlen(src);
	int token_start = 0;
	const char *delims = " ,\t\n,\0";
	int n_delims = strlen(delims);
	int mode = 0;
	int token_finished = 0;
	
	*n_tokens = 0;
	
	for (int i = 0; i < len; i++) {
		if (src[i] == '"') {
			if (mode == 0) {
				mode = 1;
				token_finished = 1;
			} else if (src[i-1] != '\'') {
				token_finished = 1;
			}
		}
		
		for (int j = 0; j < n_delims && mode != 1 && !token_finished; j++) {
			if (src[i] == delims[j]) {
				token_finished = 1;
				break;
			}
		}
		
		if (token_finished) {
			if (i > token_start + 1 || (mode == 1 && i > 0 && src[i-1] == '\"')) {
				if (*n_tokens >= ta_size - 1) {
					ta_size += 8;
					
					tokens = realloc(tokens, sizeof(char*) * ta_size);
				}
				
				if (mode == 1)
				{
					token_start--;
					i++;
				}
				
				tokens[*n_tokens] = malloc((i - token_start + 1) * sizeof(char));
				
				strncpy(tokens[*n_tokens], &src[token_start], i - token_start);
				
				tokens[*n_tokens][i - token_start] = 0;
				
				*n_tokens = *n_tokens + 1;
				
				if (mode == 1)
				{
					i--;
					mode = 0;
				}
			}
			
			token_start = i + 1;
			token_finished = 0;
		}
	}
	
	
	tokens = realloc(tokens, sizeof(char*) * (*n_tokens + 1));
	
	return tokens;
}
