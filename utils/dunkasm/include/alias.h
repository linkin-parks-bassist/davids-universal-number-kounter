#ifndef ALIAS_H
#define ALIAS_H

#define N_ARGUMENT_ALIASES 9
#define N_DEFAULT_ALIASES 4 + N_ARGUMENT_ALIASES

typedef struct
{
	char* replacee;
	char* replacer;
} alias;

void init_aliases();

extern int num_aliases;
extern alias aliases[MAX_N_ALIASES];

#endif
