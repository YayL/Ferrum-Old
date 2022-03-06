#include "include/iron.h"
#include "io.c"
#include "lexer.c"
#include "parser.c"

void iron_compile(char* src) {

	lexer_t* lexer = init_lexer(src);
	parser_t* parser = init_parser(lexer);
	ast_t* root = parser_parse(parser);

	token_t* tok = 0;

	while((tok = lexer_next_token(lexer))->type != TOKEN_EOF) {
		printf("%s\n", token_to_str(tok));
	}

}

void iron_compile_file(char* filename) {

	char* src = read_file(filename);
	iron_compile(src);
	free(src);

}
