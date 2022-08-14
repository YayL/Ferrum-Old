#include "ferrum.h"

#include <unistd.h>

#include "io.h"
#include "lexer.h"
#include "parser.h"
#include "as_f.h"
#include "visitor.h"
#include "common.h"

void ferrum_compile(char * src) {

	struct Lexer * lexer = init_lexer(src);
	struct Parser * parser = init_parser(lexer);
	struct Ast * root = parser_parse(parser);

	FILE * out = open_file("ferrum.asm", "wb");

	struct Visitor * runtime = init_visitor(root, out);
	
	visitor_visit_root(runtime, root);

	as_f_root(root, runtime, out);

	fclose(out);

	char * output = exec("nasm -f elf64 ferrum.asm -o ferrum.o && ld ferrum.o -o ferrum");
}


void ferrum_compile_file(char * filename) {

	char* src = read_file(filename);
	ferrum_compile(src);
	free(src);

}

static char * exec(const char * cmd) {
	char * output = 0;

	FILE * fp;
	size_t length;
	char path[1035];

	fp = popen(cmd, "r");

	if (fp == NULL) {
		println("Error: Unable to run command: '{s}'", cmd);
		exit(1);
	}

	while (fgets(path, sizeof(path), fp) != NULL) {
		output = format("{2s}", output, path);
	}

	pclose(fp);
	
	return output;
}
