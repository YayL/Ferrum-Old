#include "visitor.h"

#include "ferrum_builtins.h"
#include "common.h"

struct Ast * visitor_lookup(struct List * list, const char * name) {
	for(size_t i = 0; i < list->size; ++i) {
		struct Ast * child_ast = list->items[i];
		if (!child_ast->name)
			continue;

		if(strcmp(child_ast->name, name) == 0) 
			return child_ast;
	}
	return NULL;
}

struct Visitor * init_visitor(struct Ast * root) {
	struct Visitor * visitor = malloc(sizeof(struct Visitor));
	visitor->node = init_ast(AST_COMPOUND);
	visitor->builtins = calloc(1, sizeof(char));
	visitor->section_data = calloc(1, sizeof(char));
	visitor->data_count = 0;
	visitor->jump_targets = 0;

	init_builtins(visitor->node->nodes);

	return visitor;}

struct Ast * visitor_visit_compound(struct Visitor * visitor, struct Ast * node, struct List * list) {
	struct Ast * compound = init_ast(AST_COMPOUND);	

	for (size_t i = 0; i < node->nodes->size; ++i) {
		list_push(compound->nodes, visitor_visit(visitor, list_at(node->nodes, i), list));
	}

	return compound;
}

struct Ast * visitor_visit_function(struct Visitor * visitor, struct Ast * node, struct List * list) {

	struct Ast * function = init_ast(AST_FUNCTION);
	function->name = node->name;
	function->value = init_ast(AST_COMPOUND);
	
	struct List * func_list = init_list(sizeof(struct Ast));

	struct Ast * func = init_ast(AST_VARIABLE);
	func->name = node->name;

	list_push(func_list, func);

	for(size_t i = 0; i < node->value->nodes->size; ++i) {
		struct Ast * curr = node->value->nodes->items[i];
		curr->int_value = (i + 2) << 3;
		list_push(function->value->nodes, visitor_visit(visitor, curr, list));

		struct Ast * argument = init_ast(AST_VARIABLE);
		argument->name = curr->name;
		argument->int_value = curr->int_value;
		list_push(func_list, argument);
	}

	function->value->value = visitor_visit(visitor, node->value->value, func_list);
	list_push(list, function);

	return function;
	
}

struct Ast * visitor_visit_assignment(struct Visitor * visitor, struct Ast * node, struct List * list) {
	
	struct Ast * variable = init_ast(AST_ASSIGNMENT);
	variable->name = node->name;
	variable->value = visitor_visit(visitor, node->value, list);
	
	if (!visitor_lookup(list, variable->name)) {
		println("[Runtime] Error: Variable '{s}' is not defined in scope.", variable->name);
		exit(1);
	}

	return variable;

}

struct Ast * visitor_visit_declare(struct Visitor * visitor, struct Ast * node, struct List * list) {
	
	if (visitor_lookup(list, node->name)) {
		println("[Runtime] Error: '{s}' has already been declared in this scope!", node->name);
		exit(1);
	}

	struct Ast * variable = init_ast(AST_DECLARE);
	variable->name = node->name;
	if (node->value) 
		variable->value = visitor_visit(visitor, node->value, list);
	variable->data_type = node->data_type;
	list_push(list, variable);

	return variable;
}

struct Ast * visitor_visit_variable(struct Visitor * visitor, struct Ast * node, struct List * list) {
	
	struct Ast * variable = visitor_lookup(visitor->node->nodes, node->name);

	if (variable) {
		return variable;
	}

	list_push(visitor->node->nodes, node);

	return node;

}

struct Ast * visitor_visit_return(struct Visitor * visitor, struct Ast * node, struct List * list) {
	return node;
}

struct Ast * visitor_visit_if(struct Visitor * visitor, struct Ast * node, struct List * list) {
	
	node->int_value = visitor->jump_targets++;

	unsigned int prev_size = list->size;
	struct Ast * value;
	for (unsigned int i = 0; i < node->nodes->size; ++i) {	
		value = ((struct Ast *) node->nodes->items[i])->value;
		node->nodes->items[i] = visitor_visit(visitor, node->nodes->items[i], list);
		((struct Ast *)node->nodes->items[i])->value = value;
	}
	while (prev_size != list->size) list_pop(list);
	
	return node;
}

struct Ast * visitor_visit_for(struct Visitor * visitor, struct Ast * node, struct List * list) {
	node->int_value = visitor->jump_targets++;
	unsigned int prev_size = list->size;
	
	struct Ast * scope = list_at(node->nodes, 0);
	if (scope != NULL)
		node->nodes->items[0] = visitor_visit(visitor, scope, list);
	
	while (prev_size != list->size) list_pop(list);
	
	return node;
}

struct Ast * visitor_visit_while(struct Visitor * visitor, struct Ast * node, struct List * list) {

	node->int_value = visitor->jump_targets++;
	unsigned int prev_size = list->size;
	
	if (node->value != NULL)
		node->value = visitor_visit(visitor, node->value, list);
	
	while (prev_size != list->size) list_pop(list);
	
	return node;
}

struct Ast * visitor_visit_do(struct Visitor * visitor, struct Ast * node, struct List * list) {
	node->int_value = visitor->jump_targets++;
	unsigned int prev_size = list->size;

	node->value = visitor_visit(visitor, node->value, list);
	
	while (prev_size != list->size) list_pop(list);
	
	return node;
}

struct Ast * visitor_visit_continue(struct Visitor * visitor, struct Ast * node, struct List * list) {

	node->int_value = visitor->jump_targets - node->int_value;
	return node;
}

struct Ast * visitor_visit_break(struct Visitor * visitor, struct Ast * node, struct List * list) {
	node->int_value = visitor->jump_targets - node->int_value;
	return node;
}

struct Ast * visitor_visit_call(struct Visitor * visitor, struct Ast * node, struct List * list) {
	struct Ast * variable = visitor_lookup(visitor->node->nodes, node->name);

	if (variable) {
		if (!variable->int_value && variable->f_ptr) {
			variable->int_value = 1;
			for (unsigned int i = 0; i < node->nodes->size; ++i) {
				visitor_visit(visitor, (struct Ast *)list_at(node->nodes, i), list);
			}
			return variable->f_ptr(visitor, node, list);
		} else {
			node->push = variable->push;
			node->int_value = variable->int_value;
		}
	} else {
		node->int_value = node->nodes->size;
	}	

	for (unsigned int i = 0; i < node->nodes->size; ++i) {
		visitor_visit(visitor, (struct Ast *)list_at(node->nodes, i), list);
	}

	return node;

}

struct Ast * visitor_visit_value(struct Visitor * visitor, struct Ast * node, struct List * list) {

	return node;

}

struct Ast * visitor_visit_array(struct Visitor * visitor, struct Ast * node, struct List * list) {
	
	struct Ast * variable = visitor_lookup(visitor->node->nodes, node->name);

	if (variable) {
		#ifdef VISITOR_DEBUG
			print_ast("{s}\n", variable);
		#endif
		return variable;
	}

	list_push(visitor->node->nodes, node);

	return node;

}

struct Ast * visitor_visit_access(struct Visitor * visitor, struct Ast * node, struct List * list) {

	return node;

}

struct Ast * visitor_visit_string(struct Visitor * visitor, struct Ast * node, struct List * list) {	
	
    unsigned int str_id = visitor->data_count++;

    const char * data_template = 	"{s}"
									"{s} db \"{s}\", 0xA, 0xD\n"
									"{s}_len equ $-{s}\n";

	char * const_str = format("str_{u}", str_id);

	visitor->section_data = format(data_template,	visitor->section_data, 
													const_str,
													node->name,
													const_str,
													const_str);
	
	node->name = const_str;
	node->int_value = str_id;

	return node;
}

struct Ast * visitor_visit(struct Visitor * visitor, struct Ast * node, struct List * list) {

	#ifdef VISITOR_DEBUG
		print_ast("Debug [Visitor]: {s}\n", node);
	#endif

	//println("{i}", node->type);

	switch(node->type) {
		case AST_COMPOUND: return visitor_visit_compound(visitor, node, list);
		case AST_FUNCTION: return visitor_visit_function(visitor, node, list);
		case AST_ASSIGNMENT: return visitor_visit_assignment(visitor, node, list);
		case AST_DECLARE: return visitor_visit_declare(visitor, node, list);
		case AST_VARIABLE: return visitor_visit_variable(visitor, node, list);
		case AST_RETURN: return visitor_visit_return(visitor, node, list);
		case AST_IF: return visitor_visit_if(visitor, node, list);
		case AST_WHILE: return visitor_visit_while(visitor, node, list);
		case AST_FOR: return visitor_visit_for(visitor, node, list);
		case AST_DO: return visitor_visit_do(visitor, node, list);
		case AST_DO_WHILE: return visitor_visit_do(visitor, node, list); //might error
		case AST_CALL: return visitor_visit_call(visitor, node, list);
		case AST_VALUE: return visitor_visit_value(visitor, node, list);
		case AST_ARRAY: return visitor_visit_array(visitor, node, list);
		case AST_ACCESS: return visitor_visit_access(visitor, node, list);
		case AST_STRING: return visitor_visit_string(visitor, node, list);
		case AST_BREAK: return visitor_visit_break(visitor, node, list);
		case AST_CONTINUE: return visitor_visit_continue(visitor, node, list);
		case AST_EXPR: return node; //visitor_visit_eval(visitor, node, list);
		case AST_BINOP: return node;
		case AST_INT: return node;
		default: println("[Visitor]: Unable to handle AST type '{u}' == '{s}'", node->type, ast_type_to_str(node->type)); exit(1);
	}

}
