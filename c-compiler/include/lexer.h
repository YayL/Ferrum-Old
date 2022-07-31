#pragma once

#include "common.h"
#include "token.h"

struct Lexer {
	
	char *src;
	size_t size;
	char c;
	unsigned int i;
	unsigned int line, pos;

};

struct Lexer * init_lexer(char *);

void lexer_advance(struct Lexer *);
void lexer_skip_whitespace(struct Lexer *);

struct Token * lexer_advance_current(struct Lexer *, int);
struct Token * lexer_advance_with(struct Lexer *, struct Token *);

char lexer_peek(struct Lexer *, int);

struct Token * lexer_parse_id(struct Lexer *);
struct Token * lexer_parse_int(struct Lexer *);
struct Token * lexer_parse_string(struct Lexer *);
struct Token * lexer_parse_comment(struct Lexer *);
struct Token * lexer_next_token(struct Lexer *);
struct Token * lexer_parse_operation(struct Lexer *);
