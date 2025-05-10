#include <stdlib.h>
#include <stdio.h>
#include "dunkasm.h"

char labels[MAX_LABELS][MAX_LABEL_LENGTH];
uint16_t label_positions[MAX_LABELS];
label_ref label_refs[MAX_LABEL_REFS];
