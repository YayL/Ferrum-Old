#include "parser.h"

#include "AST.h"
#include "types.h"
#include "common.h"

struct Parser * init_parser(struct Lexer * lexer) {

	struct Parser * parser = calloc(1, sizeof(struct Parser));
	parser->lexer = lexer;
	parser->token = lexer_next_token(lexer);

	return parser;
}

unsigned int is_statement(char * name) {
	const size_t count = 1;
	const char statement_list[][10] = { "return" };

	for (int i = 0; i < count; ++i) {
		if (strcmp(statement_list[i], name) == 0) { 
			return 1;
		}
	}
	return 0;
}

struct Token * parser_eat(struct Parser * parser, int type) {

	if(parser->token->type != type) {
		print_token("[Parser]: Unexpected token: {s}, expecting: ", parser->token);
		println("'{s}'", token_type_to_str(type));
		exit(1);
	}

	parser->_prev = parser->token;
	parser->token = lexer_next_token(parser->lexer);
	#ifdef PARSER_DEBUG
		print_token("Debug [Parser]: {s}, ", parser->_prev);
		print_token("{s}\n", parser->token);
	#endif
	return parser->token;

}

struct Ast * parser_parse(struct Parser * parser) {

	return parser_parse_compound(parser);

}

struct Ast * parser_parse_id(struct Parser * parser) {
	parser_eat(parser, TOKEN_ID);

	struct Ast * ast = init_ast(AST_VARIABLE);
	ast->name = parser->_prev->value;

	#ifdef AST_DEBUG
		println("Debug: [AST] {s}", ast->name);
	#endif

	if (is_statement(ast->name)) {
		
		ast->name = parser->_prev->value;
		ast->type = AST_STATEMENT;
		ast->value = parser_parse_expr(parser);

		return ast;

	} else if (parser->token->type == TOKEN_COLON) {
		parser_eat(parser, TOKEN_COLON);

		ast->data_type = typename_to_int(parser->token->value);

		#ifdef PARSER_DEBUG
			println("Debug: [Parser] (ID: {s} == {i} | Type : {s} == {i})", ast->name, ast->type, parser->token->value, ast->data_type);
		#endif
		parser_eat(parser, TOKEN_ID);

		if(parser->token->type == TOKEN_LBRACKET){
			parser_eat(parser, TOKEN_LBRACKET);
			#ifdef PARSER_DEBUG
				print_token("\t\t{s}\n", parser->token);
			#endif
			ast->type = AST_ARRAY;
			parser_eat(parser, TOKEN_RBRACKET);

		} else if (parser->token->type == TOKEN_EQUALS) {
			parser_eat(parser, TOKEN_EQUALS);
			ast->type = AST_DECLARE;

			#ifdef PARSER_DEBUG
				print_ast("Debug: [AST] {s}\n", ast);
			#endif
			
			ast->value = parser_parse_expr(parser);

		} else if (parser->token->type == TOKEN_LPAREN) {
			ast->value = parser_parse_list(parser);
			ast->type = AST_FUNCTION;
		}
	} else if (parser->token->type == TOKEN_LBRACKET) {
		parser_eat(parser, TOKEN_LBRACKET);
		ast->type = AST_ACCESS;
		ast->value = parser_parse_expr(parser);
		parser_eat(parser, TOKEN_RBRACKET);

	} else if (parser->token->type == TOKEN_LPAREN) {
		#ifdef PARSER_DEBUG
			print_token("Debug [Parser]: {s}\n", parser->token);
			print_token("\t\t{s}\n", parser->_prev);
		#endif
		char * name = ast->name;
		ast = parser_parse_list(parser);
		ast->type = AST_CALL;
		ast->name = name;
	} else if (parser->token->type == TOKEN_EQUALS) {
		parser_eat(parser, TOKEN_EQUALS);
		ast->type = AST_ASSIGNMENT;
		ast->value = parser_parse_expr(parser);
	}

	return ast;

}

struct Ast * parser_parse_list(struct Parser * parser) {

	parser_eat(parser, TOKEN_LPAREN);

	struct Ast * ast = init_ast(AST_COMPOUND);

	if (parser->token->type != TOKEN_RPAREN) {
		list_push(ast->nodes, parser_parse_expr(parser));

		while(parser->token->type == TOKEN_COMMA) {
			parser_eat(parser, TOKEN_COMMA);
			list_push(ast->nodes, parser_parse_expr(parser));
		}
	}

	parser_eat(parser, TOKEN_RPAREN);

	if(parser->token->type == TOKEN_DEF) {
		parser_eat(parser, TOKEN_DEF);
		ast->type = AST_FUNCTION;
		ast->value = parser_parse_scope(parser);
	}

	return ast;
}

struct Ast * parser_parse_int(struct Parser * parser) {

	struct Ast * ast = init_ast(AST_INT);
	ast->int_value = atoi(parser->token->value);

	parser_eat(parser, TOKEN_INT);

	return ast;
}

struct Ast * parser_parse_string(struct Parser * parser) {

	struct Ast * ast = init_ast(AST_STRING);	
	ast->name = format("{s}", parser->token->value);

	parser_eat(parser, TOKEN_STRING);
	return ast;
}

struct Ast * parser_parse_expr(struct Parser * parser) {
	struct Ast * expr = init_ast(AST_EXPR);
	expr->nodes = init_list(sizeof(struct Ast));

	switch (parser->token->type) {
		case TOKEN_ID: {
						   struct Ast * val = parser_parse_id(parser);
						   val->push = 1;
						   list_push(expr->nodes, val); 
						   break;
					   }
		case TOKEN_LPAREN: {
							   parser_eat(parser, TOKEN_LPAREN); 
							   list_push(expr->nodes, parser_parse_expr(parser));
							   parser_eat(parser, TOKEN_RPAREN);
							   break;
						   }
		case TOKEN_INT: {
							struct Ast * val = parser_parse_int(parser);
							val->push = 1;
							list_push(expr->nodes, val); 
							break;
						}
		case TOKEN_STRING: list_push(expr->nodes, parser_parse_string(parser)); break;
		default: {
			print_token("[Parser]: Unexpected token in expression: '{s}'\n", 
					parser->token);
			exit(1);
		}
	}

	if (parser->token->type == TOKEN_OP) {
		struct Ast * binop = init_ast(AST_BINOP);
		binop->name = parser->token->value;
		list_push(expr->nodes, binop);
		parser_eat(parser, TOKEN_OP);
		list_push(expr->nodes, parser_parse_expr(parser));
	} else {
		expr = list_at(expr->nodes, 0);
	}

	return expr;
}

// struct Ast * parser_parse_expr(struct Parser * parser) {
//
// 	switch(parser->token->type) {
// 		case TOKEN_ID: return parser_parse_id(parser);
// 		case TOKEN_LPAREN: return parser_parse_expr(parser);
// 		case TOKEN_STRING: return parser_parse_string(parser);
// 		case TOKEN_INT: return parser_parse_int(parser);
// 		default: 
// 			print_token("[Parser]: Unexpected token in expression: '{s}'\n", parser->token);
// 			exit(1);
// 	}
//
// }

struct Ast * parser_parse_scope(struct Parser * parser) {

	parser_eat(parser, TOKEN_LBRACE);

	struct Ast * scope = init_ast(AST_COMPOUND);
	while(parser->token->type != TOKEN_RBRACE) {
		struct Ast * curr = parser_parse_expr(parser);
		list_push(scope->nodes, curr);
		parser_eat(parser, TOKEN_SEMI);
	}

	parser_eat(parser, TOKEN_RBRACE);

	return scope;
}


struct Ast * parser_parse_compound(struct Parser * parser) {

	struct Ast * compound = init_ast(AST_COMPOUND);
	while(parser->token->type != TOKEN_EOF ) {
		list_push(compound->nodes, parser_parse_expr(parser));
	}

	return compound;

}
