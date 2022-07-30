#include "as_f.h"

#include "common.h"

/* 
 ---------------------------------------------------------------
 |    Stack Frame    |
 ---------------------------------------------------------------
 | [rbp+...]:	R8
 | ...
 |	[rbp+16]:	first function argument
 |	[rbp+8]:	return address
 |	[rbp+0]:	previous stack frame base pointer
 |	[rbp-8]:	first local variable
 | ...
 ---------------------------------------------------------------
 |	   Registers     |
 ---------------------------------------------------------------
 |	R8:		Value to set rsp to after returning from a function
 ---------------------------------------------------------------
 */

struct Ast * lookup(struct List * list, const char * name) {
	for(size_t i = 1; i < list->size; ++i) {
		struct Ast * ret = list->items[i];
		if(strcmp(ret->name, name) == 0) {
			return ret;
		}
	}
	return NULL;
}

char* as_f_compound(struct Ast * ast, struct List * list) {

	char * value = 0, * declerations = 0;

	for(int i = 0; i < ast->nodes->size; ++i) {

		struct Ast * child_ast = (struct Ast *) list_at(ast->nodes, i);
		#ifdef AS_F_DEBUG
			print_ast("\t{s}\n", child_ast);
		#endif
		if (child_ast->type ==  AST_DECLARE)
			declerations = format("{2s}", declerations, as_f(child_ast, list));
		else
			value = format("{2s}", value, as_f(child_ast, list));
	}
	
	value = format("{2s}", declerations, value);
	// print("{s}", value);

	return value;
}

char* as_f_function(struct Ast * ast, struct List * list) {

	const char * template =	"\nglobal {s}\n"	
							"{s}:\n"
							"push rbp\n"
							"mov rbp, rsp\n";

	char * src = format(template, ast->name, ast->name);

	struct Ast * ast_val = ast->value;
	
	#ifdef AS_F_DEBUG
		print("Debug [Assembly Frontend]: Number of Arguments for function: {s}({i})\n", ast->name, ast_val->nodes->size);
	#endif
	struct List * child_list = init_list(sizeof(struct Ast));

	struct Ast * function = init_ast(AST_VARIABLE);
	function->int_value = ast_val->nodes->size;
	function->name = ast->name;
	list_push(child_list, function);


	for (size_t i = 0; i < ast_val->nodes->size; ++i) {
		struct Ast * variable = init_ast(AST_VARIABLE);

		variable->name = ((struct Ast *)ast_val->nodes->items[i])->name;
		variable->int_value = ((i + 2) << 3);

		list_push(child_list, variable);
	}

	char * ast_val_val = as_f(ast_val->value, child_list);
	
	const char * ret_template = "mov rsp, rbp\n"
								"pop rbp\n"
								"ret\n";
	
	char * buf = format("{3s}", src, ast_val_val, ret_template);

	return buf;
}

char* as_f_assignment(struct Ast * ast, struct List * list) {
	
	const char * template = "{s}"
							"pop rax\n"
							"mov [rbp{Si}], rax\n";	

	struct Ast * variable = lookup(list, ast->name);

	if (!variable) {
		println("[Compiler] Error: Variable `{s}` has not been declared in this scope", ast->name);
		exit(1);
	}

	return format(template, as_f(ast->value, list), variable->int_value);
}

char* as_f_variable(struct Ast * ast, struct List * list) {

	struct Ast * variable = lookup(list, ast->name);

	if (!variable) {
		println("[Compiler] Error: Variable `{s}` has not been declared in this scope", ast->name);
		exit(1);
	}

	const char * template = "mov rax, [rbp{Si}]\n"
							"push rax\n";

	return format(template, variable->int_value);
}

char * as_f_declare(struct Ast * ast, struct List * list) {
	const char * template = "push 0\n"
							"{s}";

	struct Ast * variable = init_ast(AST_VARIABLE),*info=list_at(list, 0);
	variable->name = ast->name;
	variable->int_value = -((list->size - info->int_value) << 3);

	list_push(list, variable);
	
	return format(template, as_f_assignment(ast, list));
}

char * as_f_statement(struct Ast * ast, struct List * list) {
	if (strncmp(ast->name, "return", 5) == 0) {
		const char * template = "{s}"
								"pop rax\n"; // mov rdi, %s


		struct Ast * ret_ast = (struct Ast * ) 
									ast->value
										? ast->value
										: (void* )0;

		char * ret_val = format("push {i}\n", ret_ast->int_value);

		if (ret_ast) {
			print_ast("AST: {s}\n", ret_ast);
			switch(ret_ast->type) { 
				case AST_VARIABLE: ret_val = as_f_variable(ast->value, list); break;
				case AST_ACCESS: ret_val = as_f_access(ast->value, list); break;
				case AST_EXPR: ret_val = as_f_expr(ast->value, list); break;
			}
		}

		char * src = format(template, ret_val);

		#ifdef AS_F_DEBUG
			print_ast("\nDebug [Assembly Frontend]:\n\t{s}\n", ast);
			print_ast("\t{s}\n", ast->value);
		#endif

		return src;
	}

	return "; as_f_statement\n";

}

char* as_f_call(struct Ast * ast, struct List * list) {
	const char * template =	"push r8\n"
							"mov r8, rsp\n"
							"{s}"
							"call {s}\n"
							"mov rsp, r8\n"
							"pop r8\n"
							"{s}\n";

	char * argument_src = 0;
	unsigned int len = ast->nodes->size;

	for (size_t i = len; i > 0; --i) {
		char * curr_argument_src = as_f(ast->nodes->items[i - 1], list);

		argument_src = format("{2s}", argument_src, curr_argument_src);
		free(curr_argument_src);
	}

	#ifdef AS_F_DEBUG
		print_ast("\nDebug [Assembly Frontend]: 11111\n\t{s}\n", ast);
	#endif
	
	return format(template, argument_src, ast->name, ast->push ? "push rax" : "");

}

char * as_f_expr(struct Ast * ast, struct List * list) {
	
	print_ast("expr: {s}\n", ast);
	print_list(ast->nodes);

	char * left = as_f(list_at(ast->nodes, 0), list);
	char * right = as_f(list_at(ast->nodes, 2), list);
	char *	operation, 
			* template =	"pop {s}\n"
							"pop {s}\n"
							"{s}\n"
							"push {s}\n";
	

	struct Ast * op_ast = list_at(ast->nodes, 1);
	
	char op = op_ast->name[0];
	switch (op) {
		case '+': operation = format(template, "rbx", "rax", "add rax, rbx", "rax"); break;
		case '-': operation = format(template, "rbx", "rax", "sub rax, rbx", "rax"); break;
		case '^': operation = format(template, "rbx", "rax", "xor rax, rbx", "rax"); break;
		case '|': operation = format(template, "rbx", "rax", "or rax, rbx", "rax"); break;
		case '&': operation = format(template, "rbx", "rax", "and rax, rbx", "rax"); break;
		case '*': operation = format(template, "rbx", "rax", "mul rbx", "rax"); break;
		case '/': operation = format(template, "rax", "rbx", "div rbx", "rax"); break;
		case '%': operation = format(template, "rax", "rbx", "div rbx", "rdx"); break;
		default:
			println("[Compiler] Error: Operator '{c}' is not supported yet!", op);
			exit(1);
	}

	return format("{3s}", right, left, operation);

}

char* as_f_value(struct Ast * ast, struct List * list) {

	return "; as_f_value\n";

}

char * as_f_array(struct Ast * ast, struct List * list) {
	return "; as_f_array\n";
}

char * as_f_access(struct Ast * ast, struct List * list) {
	const char * template = "mov rax, [rbp{Si}]\n";

	struct Ast * variable = lookup(list, ast->name);

	if (!variable) {
		println("Error: Variable `{s}` has not been declared in this scope", ast->name);
		exit(1);
	}

	const int id = variable->int_value + (ast->value->int_value << 3);

	return format(template, id);

}

char * as_f_const_string(struct Ast * ast, struct List * list) {

	const char * template = "push {s}\n"
							"push {s}_len\n";
	
	return format(template, ast->name, ast->name);
}

char * as_f_int(struct Ast * ast, struct List * list) {
	return format("mov rax, {i}\n{s}", ast->int_value, ast->push ? "push rax\n" : "\n");
}

char* as_f(struct Ast * ast, struct List * list) {
	
	char * next_value = "";

	#ifdef AS_F_DEBUG
		print_ast("Debug [Assembly Frontend]: {s}\n", ast);
	#endif

	switch(ast->type) {
		case AST_COMPOUND: next_value = as_f_compound(ast, list); break;
		case AST_FUNCTION: next_value = as_f_function(ast, list); break;
		case AST_ASSIGNMENT: next_value = as_f_assignment(ast, list); break;
		case AST_DECLARE: next_value = as_f_declare(ast, list); break;
		case AST_VARIABLE: next_value = as_f_variable(ast, list); break;
		case AST_STATEMENT: next_value = as_f_statement(ast, list); break;
		case AST_CALL: next_value = as_f_call(ast, list); break;
		case AST_VALUE: next_value = as_f_value(ast, list); break;
		case AST_ARRAY: next_value = as_f_array(ast, list); break;
		case AST_ACCESS: next_value = as_f_access(ast, list); break;
		case AST_NOOP: next_value = ast->str_value; break;
		case AST_STRING: next_value = as_f_const_string(ast, list); break;
		case AST_INT: next_value = as_f_int(ast, list); break;
		case AST_EXPR: next_value = as_f_expr(ast, list); break;
		default: println("[Assembler Frontend]: Unknown AST type '{i}'", ast->type); exit(1);
	}

	return next_value;
}

char * as_f_root(struct Ast * root, struct Visitor * visitor, struct List * list) {

	const char * template = 	"BITS 64\n"
							"section .data\n"
							"newline db 0xA, 0xD\n"
							"newline_len equ $-newline\n"
							"{s}\n"
							"section .text\n"
							"global _start\n"
							"_start:\n"
							"call main\n"
							"mov rdi, rax\n"
							"mov eax, 60\n"
							"syscall\n"
							"\n"
							"{s}\n"
							"{s}\n";

	char * next_value = as_f(root, list);

	return format(template, visitor->section_data, visitor->builtins, next_value);
}
