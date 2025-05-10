#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dunkasm.h"

void free_line_data(line_data_str* line_data)
{
	for (int i = 0; i < line_data->token_count; i++) {
		free(line_data->tokens[i]);
	}
	free(line_data->tokens);
	free(line_data);
}

void free_list(line_data_node* head)
{
	line_data_node* temp;
	while (head) {
		temp = head;
		head = head->next;
		free_line_data(temp->data);
		free(temp);
	}
}

// Function to tokenize a line into words
line_data_str *tokenize_line(char* line, int line_number)
{
	line_data_str *line_data = malloc(sizeof(line_data_str));
	if (!line_data) {
		perror("Memory allocation failed");
		exit(EXIT_FAILURE);
	}
	line_data->line_number = line_number;
	line_data->tokens = malloc(MAX_TOKENS * sizeof(char*));
	line_data->token_count = 0;

	char* token = strtok(line, " \t\n,;");
	while (token && line_data->token_count < MAX_TOKENS) {
		line_data->tokens[line_data->token_count] = strdup(token);
		line_data->token_count++;
		token = strtok(NULL, " \t\n,;");
	}
	return line_data;
}
