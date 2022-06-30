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
	if (ast->f_ptr) {
		free(ast->f_ptr);
	}
	free(ast);
}

const char * ast_type_to_str(int type) {
	switch(type) {
		case AST_COMPOUND: return "AST_COMPOUND";
		case AST_FUNCTION: return "AST_FUNCTION";
		case AST_ASSIGNMENT: return "AST_ASSIGNMENT";
		case AST_DEF_TYPE: return "AST_DEF_TYPE";
		case AST_VARIABLE: return "AST_VARIABLE";
		case AST_INT: return "AST_INT";
		case AST_STRING: return "AST_STRING";
		case AST_ARRAY: return "AST_ARRAY";
		case AST_ACCESS: return "AST_ACCESS";
		case AST_STATEMENT: return "AST_STATEMENT";
		case AST_VALUE: return "AST_VALUE";
		case AST_CALL: return "AST_CALL";
		case AST_NOOP: return "AST_NOOP";
	}
	return "UNDEFINED";
}

char * ast_to_str(struct Ast * ast) {
	const char* type_str = ast_type_to_str(ast->type);
	const char* template = "<name='%s', type='%s', value='%p', data_type='%d', nodes='%d', int_value='%d'>";
	size_t nodes_size = 0;

	if (ast->nodes != NULL) 
		nodes_size = ast->nodes->size;

	const unsigned int size = strlen(type_str) + strlen(template) + sizeof(void *) + 8 + nodes_size / 10;

	char* str = calloc(size, 1);
	snprintf(str, size, template, ast->name, type_str, ast->value, ast->data_type, nodes_size, ast->int_value);
	str[size - 1] = 0;

	return str;
}