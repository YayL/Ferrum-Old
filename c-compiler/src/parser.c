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

struct Ast * parser_parse_statement(struct Parser * parser) {
	struct Ast * ast = init_ast(AST_NOOP);

	char * name = parser->_prev->value;

	if (strcmp(name, "return") == 0) {
		ast->type = AST_STATEMENT_RETURN;
		ast->value = parser_parse_expr(parser);
	} else if (strcmp(name, "for") == 0) {
		ast->type = AST_STATEMENT_FOR;
		ast->nodes = init_list(sizeof(struct Ast));
		parser_eat(parser, TOKEN_LPAREN);
		list_push(ast->nodes, parser_parse_expr(parser));
		parser_eat(parser, TOKEN_SEMI);
		list_push(ast->nodes, parser_parse_expr(parser));
		parser_eat(parser, TOKEN_SEMI);
		list_push(ast->nodes, parser_parse_expr(parser));
		parser_eat(parser, TOKEN_RPAREN);

		ast->value = parser_parse_scope(parser);

	} else if (strcmp(name, "while") == 0) {
		ast->type = AST_STATEMENT_WHILE;
		parser_eat(parser, TOKEN_LPAREN);
		ast->nodes = parser_parse_list(parser)->nodes;
		parser_eat(parser, TOKEN_RPAREN);
		ast->value = parser_parse_scope(parser);
	} else {
		free(ast);
		return 0;
	}

	return ast;
}

struct Ast * parser_parse_id(struct Parser * parser) {
	parser_eat(parser, TOKEN_ID);

	struct Ast * ast = init_ast(AST_VARIABLE);
	ast->name = parser->_prev->value;

	#ifdef AST_DEBUG
		println("Debug: [AST] {s}", ast->name);
	#endif

	char * name = parser->_prev->value;

  if (parser->token->type == TOKEN_COLON) {
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

		} else if (parser->token->type == TOKEN_LPAREN) {
			ast->value = parser_parse_list(parser);
			ast->type = AST_FUNCTION;
		} else {
			if (parser->token->type == TOKEN_EQUALS) {
				parser_eat(parser, TOKEN_EQUALS);
				ast->value = parser_parse_expr(parser);
			}
			ast->type = AST_DECLARE;
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
	} else {
		struct Ast * statement = parser_parse_statement(parser);
		if (statement)
			return statement;
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

struct Ast * parser_parse_op(struct Parser * parser) {

	struct Ast * op = init_ast(AST_BINOP);
	op->name = parser->token->value;
	
	parser_eat(parser, TOKEN_OP);
	
	return op;
}

struct Ast * parser_parse_expr(struct Parser * parser) {
	struct Ast * expr = init_ast(AST_EXPR);
	expr->nodes = init_list(sizeof(struct Ast));

	if (parser->token->type == TOKEN_OP) {
		expr->value = parser_parse_op(parser);
	}

	switch (parser->token->type) {
		case TOKEN_ID: {
							struct Ast * val = parser_parse_id(parser);
							val->push = 1;
							if (expr->value)
								expr->right = val;
							else
								expr->left = val;
							break;
						}
		case TOKEN_LPAREN: {
							parser_eat(parser, TOKEN_LPAREN);
							if (expr->value)
								expr->right = parser_parse_expr(parser);
							else
								expr->left = parser_parse_expr(parser);
							parser_eat(parser, TOKEN_RPAREN);
							break;
						}
		case TOKEN_INT: {
							struct Ast * val = parser_parse_int(parser);
							val->push = 1;
							if (expr->value)
								expr->right = val;
							else
								expr->left = val;
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
		expr->value = parser_parse_op(parser);
		if (parser->token->type != TOKEN_SEMI)
			expr->right = parser_parse_expr(parser);
	} else if (expr->left && !expr->value) {
		//print_ast("expr: {s}\n", expr);
		expr = expr->left;
	} else {
		print_ast("not: {s}\n", expr);
		print_ast("value: {s}\n", expr->value);
	}

	return expr;
}

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
