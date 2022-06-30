#include "common.h"
#include "iron.h"

void _help(char *_arg1) {
	printf("\nUsage: %s <subcommand> [args]\n", _arg1);
	printf("Available subcommands:\n");
	printf("\tcom\tCompile a program to executable\n");
	printf("\tsim\tSimulate a program\n");
}

int _callCommand(int count, char **arguments) {

	if(!strncmp(arguments[1], "com", 3) || !strncmp(arguments[1], "c", 1)) {
		if(count < 3) {
			_help(arguments[0]);
			printf("ERROR: Missing filename\n");
			exit(1);
		}
		iron_compile_file(arguments[2]);
		printf("\n");
	} else if(!strncmp(arguments[1], "sim", 3) || !strncmp(arguments[1], "s", 1)) {
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