#pragma once

#include "list.h"
#include "visitor.h"

struct Ast {

	enum Ast_t {
		AST_COMPOUND,
		AST_FUNCTION,
		AST_ASSIGNMENT,
		AST_DECLARE,
		AST_VARIABLE,
		AST_ARRAY,
		AST_INT,
		AST_STRING,
		AST_ACCESS,
		AST_VALUE,
		AST_CALL,
		AST_BINOP,
		AST_EXPR,
		AST_RETURN,
		AST_FOR,
		AST_IF,
		AST_WHILE,
		AST_DO,
		AST_DO_WHILE,
		AST_BREAK,
		AST_CONTINUE,
		AST_NOOP,
	} type;

	struct List * nodes;
	char * name;
	struct Ast * left;
	struct Ast * value;
	struct Ast * right;
	int data_type;
	int int_value;
	char push;
	char * str_value;
	struct Ast * (*f_ptr)();

};

struct Ast * init_ast(int);
void free_ast(struct Ast *);
const char * ast_type_to_str(int);
void print_ast(const char * template, struct Ast *);
