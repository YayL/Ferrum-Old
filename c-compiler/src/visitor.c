#include "visitor.h"

#include "ferrum_builtins.h"
#include "common.h"

struct Ast * visitor_lookup(struct Ast * scope, const char * name) {
	lookup_start: {
		for(size_t i = 0; i < scope->v_variables->size; ++i) {
			struct Ast * child_ast = list_at(scope->v_variables, i);
			if (!child_ast->name)
				continue;

			if(strcmp(child_ast->name, name) == 0) 
				return child_ast;
		}
		if (scope->type != AST_ROOT) {
			scope = scope->scope;
			goto lookup_start;
		}
	}

	return NULL;
}

struct Ast * get_scope(struct Ast * scope, unsigned int type) {
	while (scope->type != AST_ROOT) {
		if (scope->type == type) {
			return scope;
		}
		scope = scope->scope;
	}
	return scope;
}

void next(struct Visitor * visitor, struct Ast * node, struct Ast * scope) {
	if (node->left != NULL) {
		node->left->scope = scope;
		visitor_visit(visitor, node->left);
	}
	if (node->value != NULL) {
		node->value->scope = scope;
		visitor_visit(visitor, node->value);
	}
	if (node->right != NULL) {
		node->right->scope = scope;
		visitor_visit(visitor, node->right);
	}
}

struct Visitor * init_visitor(struct Ast * root, FILE * file) {
	struct Visitor * visitor = malloc(sizeof(struct Visitor));
	visitor->root = root;
	visitor->builtins = 0;
	visitor->section_data = 0;
	visitor->data_count = 0;
	visitor->jump_targets = 0;
	visitor->out = file;

	init_builtins(visitor->root->v_variables);

	return visitor;
}

void visitor_visit_compound(struct Visitor * visitor, struct Ast * node) {
	for (unsigned int i = 0; i < node->nodes->size; ++i) {
		struct Ast * next = list_at(node->nodes, i);
		next->scope = node->scope;
		visitor_visit(visitor, next);
	}
}

void visitor_visit_function(struct Visitor * visitor, struct Ast * node) {
	
	if (node->scope->type != AST_ROOT) {
		print_ast("scope: {s}\n", node->scope);
		println("[Error]: Functions must be declared in a package scope");
		exit(1);
	}

	free(node->variables);

	for(size_t i = 0; i < node->nodes->size; ++i) {
		struct Ast * parameter = list_at(node->nodes, i);
		if (parameter->type != AST_DECLARE || parameter->value != NULL) {
			println("[Syntax Error]: Function parameters can only be strict declarations");
			exit(1);
		}
		parameter->scope = node;
		parameter->int_value = (i + 2) << 3;
		list_push(node->v_variables, parameter);
	}

	node->int_value = 0;
	node->variables = list_copy(node->v_variables);
	next(visitor, node, node);
	node->int_value <<= 3;

	list_push(node->scope->v_variables, node);
}

void visitor_visit_assignment(struct Visitor * visitor, struct Ast * node) {
	
	struct Ast * variable = visitor_lookup(node->scope, node->name);	

	if (variable == NULL) {
		println("[Error]: Variable '{s}' is not defined in scope", node->name);
		exit(1);
	}

	println("1");
	
	next(visitor, node, node->scope);
}

void visitor_visit_declare(struct Visitor * visitor, struct Ast * node) {
	
	struct Ast * scope = get_scope(node->scope, AST_FUNCTION);

	if (scope->type == AST_ROOT) {
		println("[Error]: Global variables are not supported yet");
		exit(1);
	}

	struct Ast * variable = visitor_lookup(node->scope, node->name);

	if (variable != NULL) {
		print_ast("var: {s}\n", variable);
		println("[Error]: '{s}' has already been declared", node->name);
		exit(1);
	} else if (node->value != NULL) {
		node->value->scope = node->scope;
		visitor_visit(visitor, node->value);
	}
	
	scope->int_value++;
	node->int_value = -((scope->int_value) << 3);

	list_push(node->scope->v_variables, node);
}

void visitor_visit_variable(struct Visitor * visitor, struct Ast * node) {	

	struct Ast * variable = visitor_lookup(node->scope, node->name);

	if (variable == NULL) {
		println("[Error]: Variable '{s}' is not declared in scope", node->name);
		exit(1);
	}
		
	set_ast(node, variable);
	node->type = AST_VARIABLE;
}

void visitor_visit_return(struct Visitor * visitor, struct Ast * node) {
	
	struct Ast * scope = get_scope(node->scope, AST_FUNCTION);
	
	if (scope->type == AST_ROOT) {
		println("[Error]: Return can only be used inside of functions");
		exit(1);
	}

	node->str_value = scope->name;

	if (node->value != NULL) {
		next(visitor, node, node->scope);
	}
}

void visitor_visit_if(struct Visitor * visitor, struct Ast * node) {
	node->int_value = visitor->jump_targets++;

	for (unsigned int i = 0; i < node->nodes->size; ++i) {
		struct Ast * branch = list_at(node->nodes, i);
		branch->scope = node->scope;
		
		if (branch->left != NULL) {
			branch->left->scope = node->scope;
			visitor_visit(visitor, branch->left);
		}
		
		visitor_visit_compound(visitor, branch);
	}
}

void visitor_visit_for(struct Visitor * visitor, struct Ast * node) {
	node->int_value = visitor->jump_targets++;
	
	next(visitor, node, node);

	struct Ast * scope = list_at(node->nodes, 0);

	print_list(node->nodes);

	if (scope != NULL) {
		scope->scope = node;
		visitor_visit(visitor, scope);
	}
}

void visitor_visit_while(struct Visitor * visitor, struct Ast * node) {
	node->int_value = visitor->jump_targets++;
	next(visitor, node, node);
}

void visitor_visit_do(struct Visitor * visitor, struct Ast * node) {
	node->int_value = visitor->jump_targets++;
	next(visitor, node, node);
}

void visitor_visit_continue(struct Visitor * visitor, struct Ast * node) {
	
	unsigned int t = 0;
	struct Ast * scope = node;
	do {
		scope = scope->scope;
		t = scope->type;
		if (t == AST_WHILE || t == AST_FOR || t == AST_DO_WHILE || t == AST_DO) {
			node->int_value = scope->int_value;
			return;
		}
	} while (t != AST_ROOT);

	println("[Error]: Continue can only used within the scope of a loop.");
	exit(1);
}

void visitor_visit_break(struct Visitor * visitor, struct Ast * node) {
	unsigned int t = 0;
	struct Ast * scope = node;
	do {
		scope = scope->scope;
		t = scope->type;
		if (t == AST_WHILE || t == AST_FOR || t == AST_DO_WHILE || t == AST_DO) {
			node->int_value = scope->int_value;
			return;
		}
	} while (t != AST_ROOT);

	println("[Error]: Break can only used within the scope of a loop.");
	exit(1);
}

void visitor_visit_call(struct Visitor * visitor, struct Ast * node) {
	struct Ast * function = visitor_lookup(visitor->root, node->name),
			   * next;

	for (unsigned int i = 0; i < node->nodes->size; ++i) {
		next = list_at(node->nodes, i);
		next->scope = node->scope;
		visitor_visit(visitor, next);
	}

	if (function) {
		if (!function->int_value && function->f_ptr) {
			function->int_value = 1;
			set_ast(node, function->f_ptr(visitor, node, node->scope->v_variables));
			return;
		} else {
			node->push = function->push;
			node->int_value = function->int_value;
		}
	} else {
		node->int_value = node->v_variables->size;
	}
}

void visitor_visit_expr(struct Visitor * visitor, struct Ast * node) {
	
	print_ast("expr: {s}\n", node);

	if (node->right != NULL) {
		node->right->scope = node->scope;
		visitor_visit(visitor, node->right);
	}
	if (node->value != NULL) {
		node->value->scope = node->scope;
		visitor_visit(visitor, node->value);
	}
	if (node->left != NULL) {
		node->left->scope = node->scope;
		visitor_visit(visitor, node->left);
	}

}

void visitor_visit_array(struct Visitor * visitor, struct Ast * node) {
	
	struct Ast * variable = visitor_lookup(node->scope, node->name);

	if (variable) {
		#ifdef VISITOR_DEBUG
			print_ast("{s}\n", variable);
		#endif
		set_ast(node, variable);
		node->type = AST_ARRAY;
	} else {
		list_push(node->scope->v_variables, node);
	}
}

void visitor_visit_access(struct Visitor * visitor, struct Ast * node) {

}

struct Ast * visitor_visit_string(struct Visitor * visitor, struct Ast * node) {	
	
    unsigned int str_id = visitor->data_count++;

    const char * data_template = 	"{s}"
									"{s} db \"{s}\", 0xA, 0xD\n"
									"{s}_len equ $-{s}\n";

	char * const_str = format("str_{u}", str_id);

	visitor->section_data = format(data_template, visitor->section_data, const_str, node->name, const_str, const_str);
	
	node->name = const_str;
	node->int_value = str_id;
}

void visitor_visit(struct Visitor * visitor, struct Ast * node) {
	
	#ifdef VISITOR_DEBUG
		print_ast("[Visitor]: {s}\n", node);
	#endif

	switch(node->type) {
		case AST_COMPOUND: visitor_visit_compound(visitor, node); break;
		case AST_FUNCTION: visitor_visit_function(visitor, node); break;
		case AST_ASSIGNMENT: visitor_visit_assignment(visitor, node); break;
		case AST_DECLARE: visitor_visit_declare(visitor, node); break;
		case AST_VARIABLE: visitor_visit_variable(visitor, node); break;
		case AST_RETURN: visitor_visit_return(visitor, node); break;
		case AST_IF: visitor_visit_if(visitor, node); break;
		case AST_WHILE: visitor_visit_while(visitor, node); break;
		case AST_FOR: visitor_visit_for(visitor, node); break;
		case AST_DO: visitor_visit_do(visitor, node); break;
		case AST_DO_WHILE: visitor_visit_do(visitor, node); break;
		case AST_CALL: visitor_visit_call(visitor, node); break;
		case AST_ARRAY: visitor_visit_array(visitor, node); break;
		case AST_ACCESS: visitor_visit_access(visitor, node); break;
		case AST_STRING: visitor_visit_string(visitor, node); break;
		case AST_BREAK: visitor_visit_break(visitor, node); break;
		case AST_CONTINUE: visitor_visit_continue(visitor, node); break;
		case AST_EXPR: visitor_visit_expr(visitor, node); break;
		case AST_BINOP: 
		case AST_INT: break;
		default: 
			println("[Visitor]: Unable to handle AST type '{u}' == '{s}'", node->type, ast_type_to_str(node->type)); exit(1);
	}
}

void visitor_visit_root(struct Visitor * visitor, struct Ast * node) {

	struct Ast * root = init_ast(AST_ROOT);
	root->scope = root;
	root->nodes = init_list(sizeof(struct Ast));
	list_push(root->nodes, node);
	node->scope = root;

	visitor_visit(visitor, node);
}
