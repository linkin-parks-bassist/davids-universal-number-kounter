#include <assert.h>
#include <string.h>
#include "dunkasm.h"

void test_tokenizer()
{
	const char *str1 = "set r0 \"test\\nstring\"";
	char **tokens1;
	int n_tokens;
	
	tokens1 = tokenize_string(str1, &n_tokens);
	
	assert(n_tokens == 3);
	assert(strcmp(tokens1[0], "set") == 0);
	assert(strcmp(tokens1[1], "r0") == 0);
	assert(strcmp(tokens1[2], "\"test\\nstring\"") == 0);
	
	for (int i = 0; i < n_tokens; i++)
		free(tokens1[i]);
	
	free(tokens1);
	
	const char *str2 = "set rA 0x1 %test comment";
	char **tokens2;
	
	tokens2 = tokenize_string(str2, &n_tokens);
	
	assert(n_tokens == 3);
	assert(strcmp(tokens2[0], "set") == 0);
	assert(strcmp(tokens2[1], "rA")  == 0);
	assert(strcmp(tokens2[2], "0x1") == 0);
	
	for (int i = 0; i < n_tokens; i++)
		free(tokens2[i]);
	
	free(tokens2);
}

void test_encoder()
{
	dasm_buffer buf;
	dunk_instr instr = UNARY___INSTR("inc",	0x58, REGISTER, FIRST_NIBBLE | SECOND_NIBBLE);
	parameter p = {2, 8, 0};
	
	encode_instruction(&buf, &instr, &p, 1);
	
	assert(buf.data[0] == 0x8800 + 0x58);
}

void test_parameter_parser()
{
	dasm_context cxt;
	init_context(&cxt);
	
	parameter p = parse_parameter(&cxt, "argument2");
	
	assert(p.type = REGISTER);
	assert(p.value = 9);
}

int main(int argc, char **argv)
{
	printf("Running unit tests...\n");
	test_tokenizer();
	test_parameter_parser();
	printf("All unit tests passed sucessfully!\n");
	return 0;
}
