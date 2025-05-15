#ifndef DUNK_TOKENIZER_H
#define DUNK_TOKENIZER_H

dasm_line_linked_list *tokenize_file(const char *input_path);
dasm_line *tokenize_line(char* line, int line_number);
char **tokenize_string(const char *src, int *n_tokens);

#endif
