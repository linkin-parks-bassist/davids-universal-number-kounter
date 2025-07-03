#include <assert.h>
#include <string.h>
#include "dunkasm.h"

void test_tokenizer()
{
	#define N_TOKENIZER_TESTS 2
	const char *test_strings[N_TOKENIZER_TESTS];
	const int expected_n_tokens[N_TOKENIZER_TESTS] = {3, 3};
	const char **expected_tokens[N_TOKENIZER_TESTS];
	
	test_strings[0] = strdup("set r0 \"test\\nstring\"");
	expected_tokens[0] = malloc(3 * sizeof (char*));
	expected_tokens[0][0] = strdup("set");
	expected_tokens[0][1] = strdup("r0");
	expected_tokens[0][2] = strdup("\"test\\nstring\"");
	
	test_strings[1] = strdup("set *(sr0+13) 0x1 %test comment");
	expected_tokens[1] = malloc(3 * sizeof (char*));
	expected_tokens[1][0] = strdup("set");
	expected_tokens[1][1] = strdup("*(sr0+13)");
	expected_tokens[1][2] = strdup("0x1");
	
	char **tokens;
	int n_tokens;
	
	for (int i = 0; i < N_TOKENIZER_TESTS; i++)
	{
		tokens = tokenize_string(test_strings[i], &n_tokens);
		
		assert(n_tokens = expected_n_tokens[i]);
		
		for (int j = 0; j < n_tokens; j++)
		{
			assert(strcmp(tokens[j], expected_tokens[i][j]) == 0);
			free(tokens[j]);
		}
		
		free(tokens);
		free(expected_tokens[i]);
	}
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
	
	#define N_PP_TESTS 2
	
	char *test_strings [N_PP_TESTS] = {"argument2", "*(sr0+2)"};
	int expected_types [N_PP_TESTS] = {REGISTER, S_REGISTER | POINTER};
	int expected_values[N_PP_TESTS] = {9, 0};
	int expect_offset  [N_PP_TESTS] = {0, 1};
	int expected_offset[N_PP_TESTS] = {0, 2};
	parameter p;
	
	for (int i = 0; i < N_PP_TESTS; i++)
	{
		p = parse_parameter(&cxt, test_strings[i]);
		
		assert(p.type == expected_types[i]);
		assert(p.value == expected_values[i]);
		if (expect_offset[i])
		{
			assert(p.offset == expected_offset[i]);
		}
	}
}

int main(int argc, char **argv)
{
	printf("Running unit tests...\n");
	test_tokenizer();
	test_parameter_parser();
	printf("All unit tests passed sucessfully!\n");
	return 0;
}
