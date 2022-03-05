#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/mu.c"

void _help(char *_arg1) {
	printf("\nUsage: %s <subcommand> [args]\n", _arg1);
	printf("Available subcommands:\n");
	printf("\tcom\tCompile a program to executable\n");
	printf("\tsim\tSimulate a program\n");
}

int _callCommand(int count, char **arguments) {

	if(!strcmp(arguments[1], "com") || !strcmp(arguments[1], "c")) {
		if(count < 3) {
			_help(arguments[0]);
			printf("ERROR: Missing filename\n");
			exit(1);
		}
		// printf("Compiling program...\n");
		mu_compile_file(arguments[2]);
		printf("\n");
	} else if(!strcmp(arguments[1], "sim") || !strcmp(arguments[1], "s")) {
		printf("Simulating program...\n");
	} else {
		_help(arguments[0]);
		printf("ERROR: Subcommand not found\n");
		return 1;
	}

	return 0;
}


int main(int count, char **arguments) {

	if(count < 2) {
		_help(arguments[0]);
		printf("ERROR: Subcommand not provided\n");
		return 1;
	} 
	
	if(!_callCommand(count, arguments)) {

	}
	return 0;
}