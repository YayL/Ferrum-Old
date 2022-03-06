#ifndef IRON_AST_H
#define IRON_AST_H

#include "list.h"

typedef struct AST_STRUCT {

	enum {
		AST_COMPOUND,
		AST_FUNCTION,
		AST_ASSIGNMENT,
		AST_DEF_TYPE,
		AST_VARIABLE,
		AST_STATEMENT,
		AST_NOOP,
	} type;

	list_t* nodes;
	char* name;
	struct AST_STRUCT* value;
	int data_type;

} ast_t;

ast_t* init_ast(int type);

#endif