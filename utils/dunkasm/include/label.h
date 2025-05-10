#ifndef DUNKASM_LABEL_H
#define DUNKASM_LABEL_H

#define MAX_LABEL_LENGTH 	64
#define MAX_LABELS			1024
#define MAX_LABEL_REFS		1024

typedef struct
{
	const char *fname;
	char label[MAX_LABEL_LENGTH];
	uint16_t position;
	int line_number;
} label_ref;

extern char labels[MAX_LABELS][MAX_LABEL_LENGTH];
extern uint16_t label_positions[MAX_LABELS];
extern label_ref label_refs[MAX_LABEL_REFS];

#endif
