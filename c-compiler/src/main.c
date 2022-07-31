#include "common.h"
#include "ferrum.h"

void _help(char *_arg1) {
	println("\nUsage: {s} <subcommand> [args]", _arg1);
	println("Available subcommands:");
	println("\tcom\tCompile a program to executable");
	println("\tsim\tSimulate a program");
}

int _callCommand(int count, char **arguments) {

	if(!strncmp(arguments[1], "com", 3) || !strncmp(arguments[1], "c", 1)) {
		if(count < 3) {
			_help(arguments[0]);
			println("ERROR: Missing filename");
			exit(1);
		}
		ferrum_compile_file(arguments[2]);
		print("\n");
	} else if(!strncmp(arguments[1], "sim", 3) || !strncmp(arguments[1], "s", 1)) {
		println("Simulating program...");
	} else {
		_help(arguments[0]);
		println("ERROR: Subcommand not found");
		return 1;
	}

	return 0;
}


int main(int count, char **arguments) {

	if(count < 2) {
		_help(arguments[0]);
		println("ERROR: Subcommand not provided");
		return 1;
	} 
	
	if(!_callCommand(count, arguments)) {

	}
	return 0;
}
