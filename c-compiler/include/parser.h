#pragma once

#include "lexer.h"
#include "AST.h"

struct Parser {

	struct Lexer * lexer;
	struct Token * token;
	struct Token * _prev;
};

struct Parser * init_parser(struct Lexer *);

unsigned int is_statement(char *);

struct Token * parser_advance(struct Parser *, int);

struct Ast * parser_parse(struct Parser *);
struct Ast * parser_parse_id(struct Parser *);
struct Ast * parser_parse_list(struct Parser *);
struct Ast * parser_parse_int(struct Parser *);
struct Ast * parser_parse_string(struct Parser *);
struct Ast * parser_parse_statement(struct Parser *);

struct Ast * parser_parse_eval(struct Parser *);	
struct Ast * parser_parse_expr(struct Parser *);

struct Ast * parser_parse_scope(struct Parser *);
struct Ast * parser_parse_compound(struct Parser *);
