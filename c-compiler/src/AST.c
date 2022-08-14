#include "AST.h"

#include "list.h"
#include "common.h"

struct Ast * init_ast (int type) {

	struct Ast * ast = calloc(1, sizeof(struct Ast));
	ast->type = type;

	if (type == AST_COMPOUND)
		ast->nodes = init_list(sizeof(struct Ast));

	ast->variables = init_list(sizeof(struct Ast));
	ast->v_variables = init_list(sizeof(struct Ast));
	
	return ast;
}

void free_ast(struct Ast * ast) {

	free_list(ast->nodes);
	free(ast->name);
	if (ast->value) {
		free_ast(ast->value);
	}
	free(ast);
}

void set_ast(struct Ast * dest, struct Ast * src) {
	memcpy(dest, src, sizeof(struct Ast));
}

const char * ast_type_to_str(int type) {
	switch(type) {
		case AST_COMPOUND: return "AST_COMPOUND";
		case AST_FUNCTION: return "AST_FUNCTION";
		case AST_ASSIGNMENT: return "AST_ASSIGNMENT";
		case AST_DECLARE: return "AST_DECLARE";
		case AST_VARIABLE: return "AST_VARIABLE";
		case AST_INT: return "AST_INT";
		case AST_STRING: return "AST_STRING";
		case AST_ARRAY: return "AST_ARRAY";
		case AST_ACCESS: return "AST_ACCESS";
		case AST_FOR: return "AST_FOR";
		case AST_RETURN: return "AST_RETURN";
		case AST_IF: return "AST_IF";
		case AST_WHILE: return "AST_WHILE";
		case AST_DO: return "AST_DO";
		case AST_DO_WHILE: return "AST_DO_WHILE";
		case AST_CALL: return "AST_CALL";
		case AST_BINOP: return "AST_BINOP";
		case AST_EXPR: return "AST_EXPR";
		case AST_BREAK: return "AST_BREAK";
		case AST_CONTINUE: return "AST_CONTINUE";
		case AST_ROOT: return "AST_ROOT";
		case AST_NOOP: return "AST_NOOP";
	}
	return "UNDEFINED";
}

void print_ast(const char * template, struct Ast * ast) {
	const char * type_str = ast_type_to_str(ast->type);
	const char * scope = ast->scope ? ast_type_to_str(ast->scope->type) : "(NULL)";

	unsigned int nodes_size = 0, 
				 vars = ast->variables->size,
				 v_vars = ast->v_variables->size;

	if (ast->nodes != NULL) {
		nodes_size = ast->nodes->size;
	}
	
	char * ast_str = format("<name='{s}', type='{s}', [{b}::{b}::{b}], scope='{s}', data='{i}', nodes='{u}', vars='{u}:{u}', s='{s}', int='{i}', p='{b}'>", ast->name,type_str,ast->left,ast->value,ast->right,scope,ast->data_type,nodes_size,vars,v_vars,ast->str_value,ast->int_value,ast->push);

	print(template, ast_str);
	free(ast_str);
}
