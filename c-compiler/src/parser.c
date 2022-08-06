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
		ast->type = AST_RETURN;
		ast->value = parser_parse_expr(parser);
	} else if (strcmp(name, "if") == 0) {

		ast->nodes = init_list(sizeof(struct Ast));
		
		ast->type = AST_IF;

		loop_if: {	
			parser_eat(parser, TOKEN_LPAREN);
			struct Ast * if_statement;
			ast->value = parser_parse_expr(parser);
			parser_eat(parser, TOKEN_RPAREN);
			if_statement = parser_parse_scope(parser);
			if_statement->value = ast->value;
			list_push(ast->nodes, if_statement);

			if (strcmp(parser->token->value, "else") == 0) {
				parser_eat(parser, TOKEN_ID);
				if (strcmp(parser->token->value, "if") == 0) {
					parser_eat(parser, TOKEN_ID);
					goto loop_if;
				}
				list_push(ast->nodes, parser_parse_scope(parser));
			}
			ast->value = NULL;
		}
	
	} else if (strcmp(name, "for") == 0) {
		ast->type = AST_FOR;
		ast->nodes = init_list(sizeof(struct Ast));
		parser_eat(parser, TOKEN_LPAREN);
		
		if (parser->token->type != TOKEN_SEMI)
			ast->left = parser_parse_expr(parser);
		parser_eat(parser, TOKEN_SEMI);
		
		ast->value = parser_parse_expr(parser);
		parser_eat(parser, TOKEN_SEMI);

		if (parser->token->type != TOKEN_RPAREN)
			ast->right = parser_parse_expr(parser);
		parser_eat(parser, TOKEN_RPAREN);

		if (parser->token->type != TOKEN_SEMI)
			list_push(ast->nodes, parser_parse_scope(parser));

	} else if (strcmp(name, "while") == 0) {
		ast->type = AST_WHILE;
		parser_eat(parser, TOKEN_LPAREN);
		ast->left = parser_parse_expr(parser);
		parser_eat(parser, TOKEN_RPAREN);
		if (parser->token->type != TOKEN_SEMI)
			ast->value = parser_parse_scope(parser);
	} else if (strcmp(name, "do") == 0) {
		ast->type = AST_DO;
		ast->value = parser_parse_scope(parser);
		if (parser->token->type == TOKEN_ID) {
			ast->type = AST_DO_WHILE;
			parser_eat(parser, TOKEN_ID);
			if (strcmp(parser->_prev->value, "while") != 0) {
				println("[Syntax Error]: Possible do or do-while loop written incorrectly");
			}
			parser_eat(parser, TOKEN_LPAREN);
			ast->left = parser_parse_expr(parser);
			parser_eat(parser, TOKEN_RPAREN);
		}
	} else if (strcmp(name, "continue") == 0) {
		if (parser->token->type == TOKEN_INT) {
			free(ast);
			ast = parser_parse_int(parser);
		} else {
			ast->int_value = 1;
		}
		ast->type = AST_CONTINUE;
	} else if (strcmp(name, "break") == 0) {
		if (parser->token->type == TOKEN_INT) {
			free(ast);
			ast = parser_parse_int(parser);
		} else {
			ast->int_value = 1;
		}
		ast->type = AST_BREAK;
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

	// char * name = parser->_prev->value;

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
		struct Ast * statement = parser_parse_statement(parser);
		if (statement)
			return statement;
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

	if (parser->token->type == TOKEN_OP) {
		expr->value = parser_parse_op(parser);
		switch (parser->token->type) {
			case TOKEN_ID:
							expr->right = parser_parse_id(parser);
							expr->right->push = 1;
							break;
			case TOKEN_LPAREN:
							parser_eat(parser, TOKEN_LPAREN);
							expr->right = parser_parse_expr(parser);
							parser_eat(parser, TOKEN_RPAREN);
							break;
			case TOKEN_INT:
							expr->right = parser_parse_int(parser);
							expr->right->push = 1;
							break;
			default:
				print_token("[Parser]: Unexpected token in expression: '{s}'\n",parser->token);
				exit(1);
		}
	} else {
		switch (parser->token->type) {
			case TOKEN_ID:
							expr->left = parser_parse_id(parser);
							expr->left->push = 1;
							break;
			case TOKEN_LPAREN:
							parser_eat(parser, TOKEN_LPAREN);
							expr->left = parser_parse_expr(parser);
							parser_eat(parser, TOKEN_RPAREN);
							break;
			case TOKEN_INT:
							expr->left = parser_parse_int(parser);
							expr->left->push = 1;
							break;
			case TOKEN_STRING: return parser_parse_string(parser); break;
			default:
				print_token("[Parser]: Unexpected token in expression: '{s}'\n",parser->token);
				exit(1);
		}
	}

	if (parser->token->type == TOKEN_OP) {
		expr->value = parser_parse_op(parser);
		if (parser->token->type != TOKEN_SEMI && parser->token->type != TOKEN_RPAREN)
			expr->right = parser_parse_expr(parser);
	} else if (expr->left && !expr->value) {
		//print_ast("expr: {s}\n", expr);
		expr = expr->left;
	}

	return expr;
}

struct Ast * parser_parse_scope(struct Parser * parser) {

	parser_eat(parser, TOKEN_LBRACE);

	struct Ast * scope = init_ast(AST_COMPOUND);
	while(parser->token->type != TOKEN_RBRACE) {
		struct Ast * curr = parser_parse_expr(parser);
		list_push(scope->nodes, curr);
		if (parser->_prev->type != TOKEN_RBRACE)
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
