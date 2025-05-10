#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dunkasm.h"


int num_aliases;
alias aliases[MAX_N_ALIASES];


void init_aliases() {
	num_aliases = N_DEFAULT_ALIASES;
	
	aliases[0].replacee = malloc(strlen("pk") + 1);
	strcpy(aliases[0].replacee, "pk");

	aliases[0].replacer = malloc(strlen("sr0") + 1);
	strcpy(aliases[0].replacer, "sr0");

	aliases[1].replacee = malloc(strlen("sp") + 1);
	strcpy(aliases[1].replacee, "sp");

	aliases[1].replacer = malloc(strlen("sr1") + 1);
	strcpy(aliases[1].replacer, "sr1");

	aliases[2].replacee = malloc(strlen("argument") + 1);
	strcpy(aliases[2].replacee, "argument");

	aliases[2].replacer = malloc(strlen("*(sr1+1)") + 1);
	strcpy(aliases[2].replacer, "*(sr1+1)");

	aliases[3].replacee = malloc(strlen("result") + 1);
	strcpy(aliases[3].replacee, "result");

	aliases[3].replacer = malloc(strlen("*(sr1+1)") + 1);
	strcpy(aliases[3].replacer, "*(sr1+1)");

	int arslen = 9;
	char buf[arslen];
	for (int i = 0; i < N_ARGUMENT_ALIASES; i++) {
		sprintf(buf, "argument%d", i);
		aliases[i+4].replacee = malloc(sizeof(char) * arslen);
		strcpy(aliases[i+4].replacee, buf);
		
		sprintf(buf, "*(sr1+%d)", i);
		aliases[i+4].replacer = malloc(sizeof(char) * arslen);
		strcpy(aliases[i+4].replacer, buf);
	}

	for (int i = num_aliases; i < MAX_N_ALIASES; i++) {
		aliases[i].replacee = NULL;
		aliases[i].replacer = NULL;
	}
}
