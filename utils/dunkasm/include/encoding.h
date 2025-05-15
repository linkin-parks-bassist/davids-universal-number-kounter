#ifndef DUNKASM_ENCODING_H
#define DUNKASM_ENCODING_H

int encode_instruction(dasm_buffer *buf, dunk_instr *instr, parameter *params, int n_params);
uint16_t encode_goto(dasm_context *cxt, dasm_line line);

#endif
