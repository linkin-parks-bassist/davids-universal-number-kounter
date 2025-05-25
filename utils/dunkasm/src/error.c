#include <stdio.h>
#include <string.h>
#include "dunkasm.h"

void print_error(dasm_error err)
{
	char buf[256];
	
	fprintf(stderr, "\e[01;31mError (code %d):\e[0m %s\n", err.error_code & ~CODE_ERROR, err.msg);
	
	if (err.error_code & CODE_ERROR)
	{
		
		int bad_token_start = 0;
		int bad_token_end = 0;
		int bad_token_middle;
		int position = 0;
		
		if (err.file)
			sprintf(buf, "    (%s:%d): ", err.file->given_path, err.line.line_number);
		else
			sprintf(buf, "    Line ");
		
		fprintf(stderr, buf);
		position = strlen(buf);
		
		for (int i = 0; i < err.line.n_tokens; i++)
		{
			if (i == err.bad_token || (err.bad_token == -2 && i > 0))
			{
				fprintf(stderr, "\e[01;31m");
				#ifdef UNDERLINE_BAD_TOKENS
				fprintf(stderr, "\e[4m");
				#endif
				
				if (err.bad_token != -2 || i == 1)
					bad_token_start = position;
			}
			fprintf(stderr, "%s", err.line.tokens[i]);
			
			position += strlen(err.line.tokens[i]) + 1;
			
			if (i == err.bad_token || (err.bad_token == -2 && i > 0))
			{
				fprintf(stderr, "\e[0m");
				bad_token_end = position - 1;
			}
			
			if (i != err.line.n_tokens - 1)
				fputc(' ', stderr);
		}
		
		fputc('\n', stderr);
		
		if (err.bad_token == -1)
		{
			bad_token_start = position + 1;
			bad_token_end   = position + 1;
		}
		
		if (bad_token_start != 0)
		{
			bad_token_middle = (bad_token_start + bad_token_end) / 2;
			for (int i = 0; i < bad_token_middle; i++)
				fputc(' ', stderr);
			fprintf(stderr, "^\n");
		}
		
		fputc('\n', stderr);
	}
}
