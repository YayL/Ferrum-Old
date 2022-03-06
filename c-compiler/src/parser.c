#include "include/parser.h"
#include "AST.c"
#include "types.c"

#include <stdlib.h>
#include <string.h>

parser_t* init_parser(lexer_t* lexer) {

	parser_t* parser = calloc(1, sizeof(struct PARSER_STRUCT));
	parser->lexer = lexer;
	parser->token = lexer_next_token(lexer);

	return parser;
}

token_t* parser_eat(parser_t* parser, int type) {

	if(parser->token->type != type) {
		printf("[Parser]: Unexpected token: '%s', expecting: '%s'\n", token_to_str(parser->token), token_type_to_str(type));
		exit(1);
	}

	parser->token = lexer_next_token(parser->lexer);
	return parser->token;

}

ast_t* parser_parse(parser_t* parser) {

	return parser_parse_compound(parser);

}

ast_t* parser_parse_id(parser_t* parser) {

	parser_eat(parser, TOKEN_ID);

	if(parser->token->type == TOKEN_EQUALS) {
		
		parser_eat(parser, TOKEN_EQUALS);
		ast_t* ast = init_ast(AST_ASSIGNMENT);
		ast->name = parser->token->value; 
		
		ast->value = parser_parse_expr(parser);
		return ast;

	}

	ast_t* ast = init_ast(AST_VARIABLE);
	ast->name = parser->token->value; // Possible cause of error here if issuing free on parser

	if (parser->token->type == TOKEN_COLON) {
		parser_eat(parser, TOKEN_COLON);

		while(parser->token->type == TOKEN_ID) {

			ast->data_type = typename_to_int(parser->token->value);
			parser_eat(parser, TOKEN_ID);

			if(parser->token->type == TOKEN_LT) {
				parser_eat(parser, TOKEN_LT);
				ast->data_type += typename_to_int(parser->token->value);
				parser_eat(parser, TOKEN_ID);
				parser_eat(parser, TOKEN_GT);
			}
		}
	} else {
		ast->type = AST_STATEMENT;
		ast->value = parser_parse_expr(parser);
	}

	return ast;

}

ast_t* parser_parse_list(parser_t* parser) {

	parser_eat(parser, TOKEN_LPAREN);
	ast_t* ast = init_ast(AST_COMPOUND);
	list_push(ast->nodes, parser_parse_expr(parser));

	while(parser->token->type == TOKEN_COMMA) {
		parser_eat(parser, TOKEN_COMMA);
		list_push(ast->nodes, parser_parse_expr(parser));

	}

	parser_eat(parser, TOKEN_RPAREN);

	if (parser->token->type == TOKEN_COLON) {
		parser_eat(parser, TOKEN_COLON);

		while(parser->token->type == TOKEN_ID) {

			ast->data_type = typename_to_int(parser->token->value);
			parser_eat(parser, TOKEN_ID);

			if(parser->token->type == TOKEN_LT) {
				parser_eat(parser, TOKEN_LT);
				ast->data_type += typename_to_int(parser->token->value);
				parser_eat(parser, TOKEN_ID);
				parser_eat(parser, TOKEN_GT);
			}
		}
	}

	if(parser->token->type == TOKEN_DEF) {

		parser_eat(parser, TOKEN_DEF);
		ast->type = AST_FUNCTION;
		ast->value = parser_parse_scope(parser);

	}

	return ast;

}

ast_t* parser_parse_current(parser_t* parser) {

	ast_t* ast = init_ast(AST_COMPOUND);
	ast->name = parser->token->value;

	parser_eat(parser, parser->token->type);
	return ast;
}

ast_t* parser_parse_expr(parser_t* parser) {

	switch(parser->token->type) {
		case TOKEN_ID: return parser_parse_id(parser);
		case TOKEN_LPAREN: return parser_parse_list(parser);
		case TOKEN_STRING:
		case TOKEN_NUMBER: return parser_parse_current(parser);
		default: printf("[Parser]: Unexpected token: '%s'\n", token_to_str(parser->token)); exit(1);
	}

}

ast_t* parser_parse_scope(parser_t* parser) {

	parser_eat(parser, TOKEN_LBRACE);

	ast_t* scope = init_ast(AST_COMPOUND);
	while(parser->token->type != TOKEN_RBRACE) {

		list_push(scope->nodes, parser_parse_expr(parser));
		parser_eat(parser, TOKEN_SEMI);

	}

	parser_eat(parser, TOKEN_RBRACE);

	return scope;

}


ast_t* parser_parse_compound(parser_t* parser) {

	ast_t* compound = init_ast(AST_COMPOUND);
	while(parser->token->type != TOKEN_EOF ) {

		printf(" - \n");

		list_push(compound->nodes, parser_parse_expr(parser));

	}

	return compound;

}