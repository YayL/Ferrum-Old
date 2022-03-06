#ifndef IRON_PARSER_H
#define IRON_PARSER_H

#include "lexer.h"
#include "AST.h"

typedef struct PARSER_STRUCT {

	lexer_t* lexer;
	token_t* token;
} parser_t;

parser_t* init_parser(lexer_t* lexer);

token_t* parser_advance(parser_t* parser, int type);

ast_t* parser_parse(parser_t* parser);
ast_t* parser_parse_id(parser_t* parser);
ast_t* parser_parse_list(parser_t* parser);
ast_t* parser_parse_current(parser_t* parser);

ast_t* parser_parse_expr(parser_t* parser);

ast_t* parser_parse_scope(parser_t* parser);
ast_t* parser_parse_compound(parser_t* parser);

#endif