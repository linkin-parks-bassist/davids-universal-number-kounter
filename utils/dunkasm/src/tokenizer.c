#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "dunkasm.h"


dasm_line_linked_list *tokenize_file(const char *input_path)
{
	if (input_path == NULL) {
		fprintf(stderr, "Error: file path is NULL pointer");
		exit(EXIT_FAILURE);
	}

	FILE* input_file = fopen(input_path, "r");
	if (input_file == NULL) {
		perror("Error: unable to open file");
		exit(EXIT_FAILURE);
	}

	dasm_line_linked_list *list = NULL;
	dasm_line_linked_list *tail = NULL;
	dasm_line line;
	
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

		line = new_dasm_line(trimmed_line, line_number);
		
		tail = dasm_line_linked_list_append(tail, line);
		
		if (tail == NULL) {
			perror("Memory allocation failure (tokenizer.c:44)");
			exit(EXIT_FAILURE);
		}
		
		if (list == NULL)
			list = tail;

		line_number++;
	}
	
	fclose(input_file);
	return list;
}

char **tokenize_string(const char *src, int *n_tokens)
{
	if (src == NULL || n_tokens == NULL)
		return NULL;
	
	char **tokens = malloc(sizeof(char*) * strlen(src));
	
	if (tokens == NULL) {
		perror("Memory allocation failure (tokenizer.c:66)");
		exit(EXIT_FAILURE);
	}
	
	int len = strlen(src);
	int token_start = 0;
	const char *delims = " ,\t\n,\0";
	int n_delims = strlen(delims);
	int mode = 0;
	int token_finished = 0;
	
	*n_tokens = 0;
	
	for (int i = 0; i < len + 1 && mode != 2; i++)
	{
		if (src[i] == '"')
		{
			if (mode == 0)
			{
				mode = 1;
				token_finished = 1;
			}
			else if (src[i-1] != '\'')
			{
				token_finished = 1;
			}
		}
		
		if (src[i] == '\%' && mode != 1)
		{
			mode = 2;
			token_finished = 1;
		}
		
		for (int j = 0; j < n_delims && mode != 1 && !token_finished; j++)
		{
			if (src[i] == delims[j])
			{
				token_finished = 1;
				break;
			}
		}
		
		if (token_finished)
		{
			if (i > token_start)
			{
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
	
	return tokens;
}
