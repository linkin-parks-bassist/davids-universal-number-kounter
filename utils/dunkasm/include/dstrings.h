#ifndef DASM_STRINGS_H
#define DASM_STRINGS_H

#define MAX_N_STRINGS 1024

struct dasm_string {
	uint16_t *value;
	int label_n;
};

char unescape(char token);

dasm_string new_dasm_string(const char *str, int label_n);
int free_dasm_string(dasm_string res);

int handle_string_parameter(dasm_context *cxt, pa_file *file, parameter *param, const char *str, unsigned int line_number);

#endif
