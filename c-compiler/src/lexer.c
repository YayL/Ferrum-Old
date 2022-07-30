#include "lexer.h"
#include "token.h"

#include "common.h"
#include <ctype.h>
#include <stdio.h>

struct Lexer * init_lexer(char* src) {

	struct Lexer * lexer = (struct Lexer *) calloc(1, sizeof(struct Lexer));
	lexer->src = src;
	lexer->size = strlen(src);
	lexer->i = 0;
	lexer->c = src[0];

	return lexer;
}

void lexer_advance(struct Lexer * lexer) {
	if(lexer->i < lexer->size && lexer->c != EOF)
		lexer->c = lexer->src[++lexer->i];
}

void lexer_skip_whitespaces(struct Lexer * lexer) {
	while(lexer->c == ' ' || lexer->c == '\t' || lexer->c == 13 || lexer->c == '\n')
		lexer_advance(lexer);
}

char lexer_peek(struct Lexer * lexer, int offset) {
	const int index = lexer->i + offset;
    return lexer->src[index < lexer->size ? index : lexer->size];
}

struct Token * lexer_parse_id(struct Lexer * lexer) {
	unsigned int copy = lexer->i, size, i = 0;
	while(isalpha(lexer->c) || lexer->c == '_') lexer_advance(lexer);
	
	size = lexer->i - copy;
	lexer->i -= size;
	char * value = malloc(sizeof(char) * (size + 1));

	value[0] = lexer->src[lexer->i];

	for (unsigned int i = 1; i < size; ++i) {
		lexer_advance(lexer);
		value[i] = lexer->c;
	}
	lexer_advance(lexer);

	value[size] = 0;

	return init_token(value, TOKEN_ID);
}

struct Token * lexer_parse_int(struct Lexer * lexer) {
	char* value = calloc(1, sizeof(char));
	int size = 1;

	while(isdigit(lexer->c)) {
		value = realloc(value, (++size) * sizeof(char));
		strncat(value, (char[]) {lexer->c, '\0'}, 2);
		lexer_advance(lexer);
	}

	return init_token(value, TOKEN_INT);
}

struct Token * lexer_parse_string(struct Lexer * lexer) {

	int start = lexer->i + 1, end = start;

	while(lexer->src[++end] != lexer->c) {
		if(end == lexer->size) {
			println("\n[Lexer]: End of file found while reading string.");
			exit(1);
		}
	}
	const size_t length = end - start + 1;
	char * string = malloc(length * sizeof(char));
	memcpy(string, &lexer->src[start], length - 1);
	string[length - 1] = 0;
	lexer->i = end;

	return init_token(string, TOKEN_STRING);

}

struct Token * lexer_parse_comment(struct Lexer * lexer) {
	while(lexer->src[++lexer->i] != '\n');
	lexer->c = lexer->src[lexer->i];
	return lexer_next_token(lexer);
}

struct Token * lexer_advance_current(struct Lexer * lexer, int type) {
	char *value = malloc(2 * sizeof(char));
	value[0] = lexer->c;
	value[1] = '\0';

	struct Token * token = init_token(value, type);
	lexer_advance(lexer);

	#ifdef LEXER_DEBUG
		println("Debug [Lexer]: {s} [{u}]", token->value, token->type);
	#endif

	return token;
}


struct Token * lexer_advance_with(struct Lexer * lexer, struct Token * token) {
	lexer_advance(lexer);

	#ifdef LEXER_DEBUG
		println("Debug [Lexer]: {s} [{u}]", token->value, token->type);
		println("\t\tNext: {c}", lexer_peek(lexer, 1));
	#endif

	return token;
}

struct Token * lexer_next_token(struct Lexer * lexer) {

	lexer_skip_whitespaces(lexer);
	if (lexer->c != '\0') {

		if(isalpha(lexer->c) || lexer->c == '_') {
			return lexer_parse_id(lexer);
		}
		
		if(isdigit(lexer->c)) {
			return lexer_parse_int(lexer);
		}

		switch(lexer->c) {

			case '=': {
				if(lexer_peek(lexer, 1) == '>'){
					return lexer_advance_with(lexer, lexer_advance_with(lexer, init_token("=>", TOKEN_DEF)));
				}
				return lexer_advance_with(lexer, init_token("=", TOKEN_EQUALS));
			}
			case '(': return lexer_advance_current(lexer, TOKEN_LPAREN);
			case ')': return lexer_advance_current(lexer, TOKEN_RPAREN);
			case '{': return lexer_advance_current(lexer, TOKEN_LBRACE);
			case '}': return lexer_advance_current(lexer, TOKEN_RBRACE);
			case '[': return lexer_advance_current(lexer, TOKEN_LBRACKET);
			case ']': return lexer_advance_current(lexer, TOKEN_RBRACKET);
			case ',': return lexer_advance_current(lexer, TOKEN_COMMA);
			case ':': return lexer_advance_current(lexer, TOKEN_COLON);
			case ';': return lexer_advance_current(lexer, TOKEN_SEMI);
			case '<': return lexer_advance_current(lexer, TOKEN_LT);
			case '>': return lexer_advance_current(lexer, TOKEN_GT);
			case '"': return lexer_advance_with(lexer, lexer_parse_string(lexer));
			case '/':
				if (lexer_peek(lexer, 1) == '/')
					return lexer_parse_comment(lexer);
			case '+':
			case '-':
			case '&':
			case '|':
			case '^':
			case '%':
			case '*': return lexer_advance_current(lexer, TOKEN_OP);
			default: println("\n[Lexer]: Unexpected characther: {c} == {u}", lexer->c, lexer->c); exit(1);
		}

	}

	return init_token(0, TOKEN_EOF);
}
