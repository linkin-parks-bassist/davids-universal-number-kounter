#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#ifndef DUNKASM_H
#define DUNKASM_H


typedef struct dasm_inclusion dasm_inclusion;
typedef struct dasm_context dasm_context;
typedef struct dasm_options dasm_options;
typedef struct dasm_buffer dasm_buffer;
typedef struct dasm_string dasm_string;
typedef struct dasm_error dasm_error;
typedef struct dasm_file dasm_file;

#include "retval.h"
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
#include "error.h"
#include "options.h"
#include "inclusion.h"
#include "context.h"
#include "encoding.h"

#endif
