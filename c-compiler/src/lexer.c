#include "include/lexer.h"
#include "token.c"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

lexer_t* init_lexer(char* src) {

	lexer_t* lexer = (lexer_t*) calloc(1, sizeof(struct LEXER_STRUCT));
	lexer->src = src;
	lexer->size = strlen(src);
	lexer->i = 0;
	lexer->c = src[0];

	return lexer;
}

void lexer_advance(lexer_t* lexer) {
	if(lexer->i < lexer->size && lexer->c != '\0') {
		lexer->c = lexer->src[++lexer->i];
	}
}

void lexer_skip_whitespaces(lexer_t* lexer) {
	while(lexer->c == ' ' || lexer->c == '\t' || lexer->c == 13 || lexer->c == '\n')
		lexer_advance(lexer);
}

char lexer_peek(lexer_t* lexer, int offset) {
	const int index = lexer->i + offset;
	return lexer->src[index < lexer->size ? index : lexer->size];
}

token_t* lexer_parse_id(lexer_t* lexer) {
	char* value = calloc(1, sizeof(char));
	int size = 1;

	while(isalnum(lexer->c)) {
		value = realloc(value, (++size) * sizeof(char));
		strncat(value, (char[]) {lexer->c, '\0'}, 2);
		lexer_advance(lexer);
	}

	return init_token(value, TOKEN_ID);
}

token_t* lexer_parse_number(lexer_t* lexer) {
	char* value = calloc(1, sizeof(char));
	int size = 1;

	while(isdigit(lexer->c)) {
		value = realloc(value, (++size) * sizeof(char));
		strncat(value, (char[]) {lexer->c, '\0'}, 2);
		lexer_advance(lexer);
	}

	return init_token(value, TOKEN_NUMBER);
}

token_t* lexer_parse_string(lexer_t* lexer) {

	int start = lexer->i + 1, end = start;

	while(lexer->src[++end] != lexer->c) {
		if(end == lexer->size) {
			printf("\n[Lexer]: End of file found while reading string\n");
			exit(1);
		}
	}
	const int len = end - start;
	char* value = calloc(len, sizeof(char));
	memcpy(value, &lexer->src[start], len - 1);
	value[len] = '\0';
	lexer->i = end;

	return init_token(value, TOKEN_STRING);

}

token_t* lexer_parse_comment(lexer_t* lexer) {
	while(lexer->src[++lexer->i] != '\n');
	lexer->c = lexer->src[lexer->i];
	return lexer_next_token(lexer);
}

token_t* lexer_advance_current(lexer_t* lexer, int type) {
	char *value = malloc(2 * sizeof(char));
	value[0] = lexer->c;
	value[1] = '\0';

	token_t* token = init_token(value, type);
	lexer_advance(lexer);

	return token;
}


token_t* lexer_advance_with(lexer_t* lexer, token_t* token) {
	lexer_advance(lexer);
	return token;
}

token_t* lexer_next_token(lexer_t* lexer) {

	while(lexer->c != '\0') {
		lexer_skip_whitespaces(lexer);

		if(isalpha(lexer->c))
			return lexer_advance_with(lexer, lexer_parse_id(lexer));
		
		if(isdigit(lexer->c))
			return lexer_advance_with(lexer, lexer_parse_number(lexer));

		switch(lexer->c) {

			case '=': {
				if(lexer_peek(lexer, 1) == '>'){
					return lexer_advance_with(lexer, init_token("=>", TOKEN_DEF));
				}
				return lexer_advance_with(lexer, init_token("=", TOKEN_EQUALS));
			}
			case '(': return lexer_advance_current(lexer, TOKEN_LPAREN);
			case ')': return lexer_advance_current(lexer, TOKEN_RPAREN);
			case '{': return lexer_advance_current(lexer, TOKEN_LBRACE);
			case '}': return lexer_advance_current(lexer, TOKEN_RBRACE);
			case ',': return lexer_advance_current(lexer, TOKEN_COMMA);
			case ';': return lexer_advance_current(lexer, TOKEN_COLON);
			case '<': return lexer_advance_current(lexer, TOKEN_LT);
			case '>': return lexer_advance_current(lexer, TOKEN_GT);
			case '"': return lexer_advance_with(lexer, lexer_parse_string(lexer));
			case '/':
				if (lexer_peek(lexer, 1) == '/')
					return lexer_advance_with(lexer, lexer_parse_comment(lexer));
			case '+':
			case '-':
			case '*': return lexer_advance_current(lexer, TOKEN_OP);
			default: printf("\n[Lexer]: Unexpected characther: %c\n", lexer->c); exit(1); break;
		}

	}

	return init_token(0, TOKEN_EOF);
}