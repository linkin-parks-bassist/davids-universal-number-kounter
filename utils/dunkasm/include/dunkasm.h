#include <stdint.h>

#ifndef DUNKASM_H
#define DUNKASM_H

#include "instr.h"
#include "param.h"
#include "lines.h"
#include "alias.h"
#include "label.h"
#include "files.h"
#include "tokenizer.h"

extern uint8_t* output_buffer;
extern int current_position;

extern int n_labels;
extern int n_label_refs;

extern uint16_t* word_ptr;

uint16_t encode_goto(line_data_str line);
void buffer_word(uint16_t v);

#endif
