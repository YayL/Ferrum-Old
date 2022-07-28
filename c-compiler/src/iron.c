#include "iron.h"

#include <unistd.h>

#include "io.h"
#include "lexer.h"
#include "parser.h"
#include "as_f.h"
#include "visitor.h"
#include "common.h"

void iron_compile(char * src) {

	struct Lexer * lexer = init_lexer(src);
	struct Parser * parser = init_parser(lexer);
	struct Ast * root = parser_parse(parser);
	struct Visitor * runtime = init_visitor(root);
	struct List * context = init_list(sizeof(struct Ast));
	
	struct Ast * visited_root = visitor_visit(runtime, root, init_list(sizeof(struct Ast)));	

	char * src_code = as_f_root(visited_root, runtime, init_list(sizeof(struct Ast)));

	char * new_filename = "iron.nasm";

	write_file("iron.nasm", src_code);

	char * output = exec("nasm -f elf64 iron.nasm -o iron.o && ld iron.o -o iron");

	println("{s}", output);
}

void iron_compile_file(char * filename) {

	char* src = read_file(filename);
	iron_compile(src);
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
