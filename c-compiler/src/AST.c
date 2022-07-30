#include "AST.h"

#include "list.h"
#include "common.h"

struct Ast * init_ast (int type) {

	struct Ast * ast = calloc(1, sizeof(struct Ast));
	ast->type = type;

	if (type == AST_COMPOUND)
		ast->nodes = init_list(sizeof(struct Ast));
	
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
		case AST_STATEMENT: return "AST_STATEMENT";
		case AST_VALUE: return "AST_VALUE";
		case AST_CALL: return "AST_CALL";
		case AST_BINOP: return "AST_BINOP";
		case AST_EXPR: return "AST_EXPR";
		case AST_NOOP: return "AST_NOOP";
	}
	return "UNDEFINED";
}

void print_ast(const char * template, struct Ast * ast) {
	const char* type_str = ast_type_to_str(ast->type);
	size_t nodes_size = 0;

	if (ast->nodes != NULL) {
		nodes_size = ast->nodes->size;
	}
	
	char * ast_str = format("<name='{s}', type='{s}', value='{b}', data_type='{i}', nodes='{lu}', int_value='{i}', f_ptr='{b}'>", ast->name,type_str,ast->value,ast->data_type,nodes_size,ast->int_value, ast->f_ptr);

	print(template, ast_str);
	free(ast_str);
}
