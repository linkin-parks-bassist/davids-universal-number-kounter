#ifndef DUNK_TOKENIZER_H
#define DUNK_TOKENIZER_H

line_data_node *tokenize_file(const char *input_path);
line_data_str *tokenize_line(char* line, int line_number);
char **tokenize_string(const char *src, int *n_tokens);

#endif
