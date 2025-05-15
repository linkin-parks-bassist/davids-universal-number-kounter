#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>


#ifndef DUNKASM_H
#define DUNKASM_H

#define SUCCESS			0
#define BAD_ARGUMENTS 	1
#define MEMORY_FAILURE  2

typedef struct dasm_context dasm_context;
typedef struct dasm_buffer dasm_buffer;
typedef struct dasm_string dasm_string;
typedef struct dasm_file dasm_file;

#include "linked_list.h"
#include "instr.h"
#include "param.h"
#include "lines.h"
#include "alias.h"
#include "label.h"
#include "buffer.h"
#include "files.h"
#include "tokenizer.h"
#include "dstrings.h"
#include "context.h"
#include "encoding.h"

//#include "malloc_free.h"

uint16_t encode_goto(dasm_context *cxt, dasm_line line);

#endif
