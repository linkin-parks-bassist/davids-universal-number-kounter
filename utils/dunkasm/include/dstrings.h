#ifndef DASM_STRINGS_H
#define DASM_STRINGS_H

#define MAX_N_STRINGS 1024

struct dasm_string {
	uint16_t *value;
	dasm_label *label;
};

DECLARE_LINKED_PTR_LIST(char);
DECLARE_LINKED_LIST(dasm_string);

// Just so my IDE knows to highlight it lol
typedef struct    char_ptr_linked_list    char_ptr_linked_list;
typedef struct dasm_string_linked_list dasm_string_linked_list;

int is_number(const char* str);
int is_dnumber(const char* str);
int is_hnumber(const char* str);
int is_bnumber(const char* str);
int is_char(const char* str);
int is_string(const char* str);
int is_label(const char *str);

uint16_t parse_char(const char *str);

long parse_number(const char* str);
char unescape(char token);

dasm_string new_dasm_string(const char *str, dasm_label *label);
int free_dasm_string(dasm_string res);

void dasm_string_destructor(dasm_string str);

#endif
