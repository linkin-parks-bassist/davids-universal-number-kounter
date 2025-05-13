#include <string.h>
#include "dunkasm.h"

int encode_instruction(dasm_buffer *buf, dunk_instr *instr, parameter *params, int n_params)
{
	if (buf == NULL || instr == NULL || params == NULL)
		return BAD_ARGUMENTS;
	
	uint16_t code = instr->code;
			
	for (int j = 0; j < n_params; j++) {
		if (instr->arg_positions[j] & FIRST_NIBBLE) {
			code += params[j].value * 0x1000;
		}
		if (instr->arg_positions[j] & SECOND_NIBBLE) {
			code += params[j].value * 0x0100;
		}
	}

	append_buffer(buf, code);
	
	int n_following = 0;
	
	for (int i = 0; i < instr->n_args; i++) {
		if ((instr->arg_positions[i] & FOLLOWING_MASK) && (instr->arg_positions[i] & FOLLOWING_POS_MASK) > n_following)
			n_following = instr->arg_positions[i] & FOLLOWING_POS_MASK;
	}
	
	if (strcmp(instr->name, "return") == 0 || strcmp(instr->name, "call") == 0) {
		printf("\n");
	}
	
	uint16_t c;
	
	for (int i = 1; i < n_following + 1; i++) {
		c = 0;
		
		for (int j = 0; j < n_params; j++) {
			/* I have to use if-else here because apparently switches need
			 * cases that reduce to an integer constant at compile-time.
			 * I assumed gcc would just unroll the loop and it would be fine
			 * but even when I wrote an unrollable version it wouldn't compile*/
			
			if ((instr->arg_positions[j] & FOLLOWING_MASK) == FOLLOWING(i))
			{
				c = params[j].value;
			}
			else if ((instr->arg_positions[j] & FOLLOWING_MASK) == OFFSET_FOLLOWING(i))
			{
				c = params[j].offset;
			}
			else if ((instr->arg_positions[j] & FOLLOWING_MASK) == FOLLOWING_FN(i))
			{
				c += (params[j].value & 0b1111) * 0x1000;
			}
			else if ((instr->arg_positions[j] & FOLLOWING_MASK) == FOLLOWING_SN(i))
			{
				c += (params[j].value & 0b1111) * 0x0100;
			}
			else if ((instr->arg_positions[j] & FOLLOWING_MASK) == FOLLOWING_TN(i))
			{
				c += (params[j].value & 0b1111) * 0x0010;
			}
			else if ((instr->arg_positions[j] & FOLLOWING_MASK) == FOLLOWING_LN(i))
			{
				c += (params[j].value & 0b1111) * 0x0001;
			}
		}
		
		append_buffer(buf, c);
	}
	
	return SUCCESS;
}
