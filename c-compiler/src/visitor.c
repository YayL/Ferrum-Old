#include "visitor.h"

#include "iron_builtins.h"
#include "common.h"

struct Ast * visitor_lookup(struct List * list, const char * name) {
	for(size_t i = 0; i < list->size; ++i) {
		struct Ast * child_ast = list->items[i];
		if (child_ast->type != AST_VARIABLE || !child_ast->name)
			continue;

		if(strcmp(child_ast->name, name) == 0) 
			return child_ast;
	}
	return NULL;
}

struct Visitor * init_visitor(struct Ast * root) {
	struct Visitor * visitor = calloc(1, sizeof(struct Visitor));
	visitor->node = init_ast(AST_COMPOUND);
	visitor->root = root;
	visitor->root->str_value = calloc(1, sizeof(char));

	init_builtins(visitor->node->nodes);

	return visitor;
}

struct Ast * visitor_visit_compound(struct Visitor * visitor, struct Ast * node, struct List * list) {
	struct Ast * compound = init_ast(AST_COMPOUND);

	for (size_t i = 0; i < node->nodes->size; ++i) {
		list_push(compound->nodes, visitor_visit(visitor, node->nodes->items[i], list));
	}

	return compound;
}

struct Ast * visitor_visit_function(struct Visitor * visitor, struct Ast * node, struct List * list) {

	struct Ast * function = init_ast(AST_FUNCTION);
	function->name = node->name;
	function->value = init_ast(AST_COMPOUND);

	for(size_t i = 0; i < node->value->nodes->size; ++i) {
		((struct Ast *) node->value->nodes->items[i])->int_value = (i + 2) << 3;
		list_push(function->value->nodes, visitor_visit(visitor, node->value->nodes->items[i], list));
	}

	function->value->value = visitor_visit(visitor, node->value->value, list);
	list_push(list, function);

	return function;
	
}

struct Ast * visitor_visit_assignment(struct Visitor * visitor, struct Ast * node, struct List * list) {
	
	struct Ast * variable = init_ast(AST_ASSIGNMENT);
	variable->name = node->name;
	variable->value = visitor_visit(visitor, node->value, list);
	list_push(list, variable);

	return variable;

}

struct Ast * visitor_visit_variable(struct Visitor * visitor, struct Ast * node, struct List * list) {
	
	struct Ast * variable = visitor_lookup(visitor->node->nodes, node->name);

	if (variable) {
		printf("%s\n", ast_to_str(variable));
		return variable;
	}

	list_push(visitor->node->nodes, node);

	return node;

}

struct Ast * visitor_visit_statement(struct Visitor * visitor, struct Ast * node, struct List * list) {

	return node;

}

struct Ast * visitor_visit_call(struct Visitor * visitor, struct Ast * node, struct List * list) {
	struct Ast * variable = visitor_lookup(visitor->node->nodes, node->name);

	if (variable) {
		if (!variable->int_value && variable->f_ptr) {
			variable->int_value = 1;
			return variable->f_ptr(visitor, node, node->nodes);
		}
	}

	return node;

}

struct Ast * visitor_visit_value(struct Visitor * visitor, struct Ast * node, struct List * list) {

	return node;

}

struct Ast * visitor_visit_array(struct Visitor * visitor, struct Ast * node, struct List * list) {
	
	struct Ast * variable = visitor_lookup(visitor->node->nodes, node->name);

	if (variable) {
		printf("%s\n", ast_to_str(variable));
		return variable;
	}

	list_push(visitor->node->nodes, node);

	return node;

}

struct Ast * visitor_visit_access(struct Visitor * visitor, struct Ast * node, struct List * list) {

	return node;

}

struct Ast * visitor_visit(struct Visitor * visitor, struct Ast * node, struct List * list) {

	#ifdef VISITOR_DEBUG
		printf("Debug [Visitor]: %s\n", ast_to_str(node));
	#endif

	switch(node->type) {
		case AST_COMPOUND: return visitor_visit_compound(visitor, node, list);
		case AST_FUNCTION: return visitor_visit_function(visitor, node, list);
		case AST_ASSIGNMENT: return visitor_visit_assignment(visitor, node, list);
		case AST_VARIABLE: return visitor_visit_variable(visitor, node, list);
		case AST_STATEMENT: return visitor_visit_statement(visitor, node, list);
		case AST_CALL: return visitor_visit_call(visitor, node, list);
		case AST_VALUE: return visitor_visit_value(visitor, node, list);
		case AST_ARRAY: return visitor_visit_array(visitor, node, list);
		case AST_ACCESS: return visitor_visit_access(visitor, node, list);
		case AST_INT: return node;
		default: printf("[Visitor]: Unable to handle AST type '%d'\n", node->type); exit(1);
	}

}