#include <string.h>
#include "dunkasm.h"

/* A DUNK instruction is encoded as a 16bit word by the following function.
 * The second byte of the word is the opcode, while the first byte is used
 * for parameters. If the needed parameters will not fit in the 16bit word
 * (such as, for instance, a constant, which needs its own word), they are
 * "overflow parameters", written to the binary after the instruction.
 * 
 * The dunk_instr struct includes information about how the parameters are
 * positioned, described by the values of the parameter_positions array.
 * The first byte of the opcode consists of two 4 bit "nibbles". Since there
 * are 16 general-purpose registers, and fewer than 16 special registers,
 * registers can be specified by a single nibble. Different instructions
 * will interpret the values of the nibbles differently. For instance, the
 * instruction that sets register r5 to *(sr1+1) is given by the word
 * 0x5120 followed by the word 0x0001. Here, the first nibble encodes the
 * ID of the register to be written to, while the second encodes which 
 * special register is referenced. The opcode encodes the isntruction
 * "set rN to *(srM+offs)", and the following word is the offset to be used.
 */
 
/* The positions are encoded as integers. If the intended position is one of
 * the nibbles in the first word, this is set to FIRST_NIBBLE or SECOND_NIBBLE,
 * which are macros. For overflow parameters, there are macros FOLLOWING(n),
 * where n is the number of words after the instruction word that the parameter
 * is to be written (n can be larger than 1). The number n is encoded as the last
 * 4 bits of the parameter position integer, so that it can be extracted by
 * bitwise "and"-ing with the macro FOLLOWING_POS_MASK, defined as 0b1111.
 * Similarly, itcan be checked whether a parameter is an overflow parameter 
 * by anding with FOLLOWING_MASK, which includes all the bits which are
 * nonzero in the FOLLOWING(n) macros. I've also included the possibility
 * to encode overflow parameters as individual nibbles in the following words,
 * but this is not currently in use.
 */
 
 /* Finally, for some instructions (namely, the unary ALU operations),
  * there is a single parameter, written in both the first and second nibble
  * of the instruction. This is why I am using bitflags; so the parameter
  * position can be FIRST_NIBBLE | SECOND_NIBBLE and appear in both positions. */
int encode_instruction(dasm_buffer *buf, const dunk_instr *instr, parameter *params, int n_params)
{
	if (buf == NULL || instr == NULL || params == NULL)
		return BAD_ARGUMENTS;
	
	uint16_t code = instr->code;
	uint16_t c;
	int n_following = 0;
			
	for (int j = 0; j < n_params; j++)
	{
		if (instr->parameter_positions[j] & FIRST_NIBBLE)
		{
			code += params[j].value * 0x1000;
		}
		if (instr->parameter_positions[j] & SECOND_NIBBLE)
		{
			code += params[j].value * 0x0100;
		}
	}

	append_buffer(buf, code);
	
	for (int i = 0; i < instr->n_parameters; i++)
	{
		if ((instr->parameter_positions[i] & FOLLOWING_MASK) && (instr->parameter_positions[i] & FOLLOWING_POS_MASK) > n_following)
			n_following = instr->parameter_positions[i] & FOLLOWING_POS_MASK;
	}
	
	for (int i = 1; i < n_following + 1; i++)
	{
		c = 0;
		
		for (int j = 0; j < n_params; j++)
		{
			/* I have to use if-else here because switches need
			 * cases that reduce to an integer constant at compile-time.
			 * I assumed gcc would just unroll the loop and it would be fine
			 * but even when I wrote an unrollable version, it wouldn't compile*/
			
			if ((instr->parameter_positions[j] & FOLLOWING_MASK) == FOLLOWING(i))
			{
				c = params[j].value;
			}
			else if ((instr->parameter_positions[j] & FOLLOWING_MASK) == OFFSET_FOLLOWING(i))
			{
				c = params[j].offset;
			}
			else if ((instr->parameter_positions[j] & FOLLOWING_MASK) == FOLLOWING_FN(i))
			{
				c += (params[j].value & 0b1111) * 0x1000;
			}
			else if ((instr->parameter_positions[j] & FOLLOWING_MASK) == FOLLOWING_SN(i))
			{
				c += (params[j].value & 0b1111) * 0x0100;
			}
			else if ((instr->parameter_positions[j] & FOLLOWING_MASK) == FOLLOWING_TN(i))
			{
				c += (params[j].value & 0b1111) * 0x0010;
			}
			else if ((instr->parameter_positions[j] & FOLLOWING_MASK) == FOLLOWING_LN(i))
			{
				c += (params[j].value & 0b1111) * 0x0001;
			}
		}
		
		append_buffer(buf, c);
	}
	
	return SUCCESS;
}
