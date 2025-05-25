#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#ifndef DUNKASM_H
#define DUNKASM_H

#define SUCCESS			0
#define FAILURE			1
#define BAD_ARGUMENTS 	2
#define MEMORY_FAILURE  3

typedef struct dasm_context dasm_context;
typedef struct dasm_buffer dasm_buffer;
typedef struct dasm_string dasm_string;
typedef struct dasm_error dasm_error;
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
#include "error.h"

#endif
