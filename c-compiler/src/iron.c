#include "include/iron.h"
#include "io.c"
#include "lexer.c"

void mu_compile(char* src) {

	
}

void mu_compile_file(char* filename) {

	char* src = read_file(filename);
	lexer_t* lexer = init_lexer(src);
	token_t* tok = 0;
	size_t depth = 0;
	
	printf("\n");
	while((tok = lexer_next_token(lexer))->type != TOKEN_EOF) {
		printf("\t%s\n", token_to_str(tok));
	}

}
