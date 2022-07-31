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

struct Ast * lookup(struct List * list, char * name) {
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
		value = format("{2s}", value, as_f(child_ast, list));
	}
	
	value = format("{2s}", declerations, value);
	// print("{s}", value);

	return value;
}

char* as_f_function(struct Ast * ast, struct List * list) {

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
	
	print_list(ast_val->value->nodes);
	char * ast_val_val = as_f(ast_val->value, child_list);
	
	const char * template =	"\nglobal {s}\n"	
							"{s}:\n"
							"push rbp\n"
							"mov rbp, rsp\n"
							"sub rsp, {u}\n";

	const char * ret_template = "mov rsp, rbp\n"
								"pop rbp\n"
								"ret\n";
	

	char * src = format(template, ast->name, ast->name, 
						(child_list->size - ast_val->nodes->size - 1) << 3);
	
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

	print_ast("assignment: {s}\n", ast);

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
	const char * template = "{s}";

	struct Ast * variable = init_ast(AST_VARIABLE),*info=list_at(list, 0);
	variable->name = ast->name;
	variable->int_value = -((list->size - info->int_value) << 3);

	list_push(list, variable);
	if (ast->value)
		return format(template, as_f_assignment(ast, list));
	return "";
}

char * as_f_return(struct Ast * ast, struct List * list) {
	const char * template = "{s}"
							"pop rax\n"; // mov rdi, %s

	struct Ast * ret_ast = (struct Ast * ) 
								ast->value
									? ast->value
									: (void* )0;

	char * ret_val = format("push {i}\n", ret_ast->int_value);

	if (ret_ast) {
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
 
char * as_f_if (struct Ast * ast, struct List * list) {
	
	

	return "; as_f_if\n";

}

char * as_f_for(struct Ast * ast, struct List * list) {

	return "; as_f_for\n";

}

char * as_f_while(struct Ast * ast, struct List * list) {

	return "; as_f_while\n";

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

	println("");

	for (size_t i = len; i > 0; --i) {
		struct Ast * arg = list_at(ast->nodes, i - 1);
		print_ast("Arg: {s}\n", arg);
		char * curr_argument_src = as_f(arg, list);

		argument_src = format("{2s}", argument_src, curr_argument_src);
		free(curr_argument_src);
	}

	#ifdef AS_F_DEBUG
		print_ast("\nDebug [Assembly Frontend]: 11111\n\t{s}\n", ast);
	#endif
	
	return format(template, argument_src, ast->name, ast->push ? "push rax" : "");

}

char * as_f_1_binop(char * template, char op, char left, char right) {
	switch (op) {
		case '+': return format(template, "rbx", "rax", "add rax, rbx", "rax");
		case '-': return format(template, "rax", "rbx", "sub rax, rbx", "rax");
		case '*': return format(template, "rbx", "rax", "mul rbx", "rax");
		case '/': return format(template, "rax", "rbx", "div rbx", "rax");
		case '%': return format(template, "rax", "rbx", "div rbx", "rdx");
		case '^': return format(template, "rbx", "rax", "xor rax, rbx", "rax");
		case '|': return  format(template, "rbx", "rax", "or rax, rbx", "rax");
		case '&': return format(template, "rbx", "rax", "and rax, rbx", "rax");
		default:
			println("[Compiler] Error: Operator '{c}' is not supported.", op);
			exit(1);
	}
}

char * as_f_2_binop(struct Ast * ast, char * template, char * op, char left, char right) {
	if (strcmp(op, "++") == 0) {
		if (right) { // ++var
			return format("mov rax, [rbp{Si}]\n"
							"inc rax\n"
							"mov [rbp{Si}], rax\n"
							"push rax\n", ast->int_value, ast->int_value);
		} else { // var++
			return format (	"mov rax, [rbp{Si}]\n"
							"push rax\n"
							"inc rax\n"
							"mov [rbp{Si}], rax\n", ast->int_value, ast->int_value);
		}
	} else if (strcmp(op, "--") == 0) {
		if (right) { // ++var
			return format("mov rax, [rbp{Si}]\n"
							"dec rax\n"
							"mov [rbp{Si}], rax\n"
							"push rax\n", ast->int_value, ast->int_value);
		} else { // var++
			return format (	"mov rax, [rbp{Si}]\n"
							"push rax\n"
							"dec rax\n"
							"mov [rbp{Si}], rax\n", ast->int_value, ast->int_value);
		}
	} else if (strcmp(op, "<<") == 0) {
		return format(template, "rax", "rcx", "shl rax, cl", "rax");
	} else if (strcmp(op, ">>") == 0) {
		return format(template, "rax", "rbx", "shr rax, rbx", "rax");
	} else {
		println("[Compiler] Error: Operator '{s}' is not supported.", op);
		exit(1);
	}
}

char * as_f_expr(struct Ast * ast, struct List * list) {
	
	char * left = 0, * right = 0, * operation = 0,
		 * template =	"pop {s}\n"
						"pop {s}\n"
						"{s}\n"
						"push {s}\n", 
		 * op = 0;

	unsigned int operator_len = 0;

	print_ast("Expr: {s}\n", ast);

	if (ast->left) {
		left = as_f(ast->left, list);
	} else {
		left = "push 0\n";
	}
	if (ast->value) {
		op = ast->value->name;
		while (op[operator_len++]);
		operator_len--;
	}
	if (ast->right) {
		right = as_f(ast->right, list);
	} else {
		right = "push 0\n";
	}
	
	switch (operator_len) {
		case 0:
			println("[Compiler] Error: There was no operator specified."); exit(1);
		case 1:
			operation = as_f_1_binop(template, op[0], ast->left != 0, ast->right != 0); break;
		case 2: {
			right = 0, left = 0;
			struct Ast * var = lookup(list,  (ast->left ? ast->left : ast->right)->name);
			operation = as_f_2_binop(var, template, op, ast->left != 0, ast->right != 0); break;
			}
	}


	return format("{3s}", right, left, operation);

}

char * as_f_value(struct Ast * ast, struct List * list) {

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
		case AST_STATEMENT_FOR: next_value = as_f_for(ast, list); break;
		case AST_STATEMENT_IF: next_value = as_f_if(ast, list); break;
		case AST_STATEMENT_RETURN: next_value = as_f_return(ast, list); break;
		case AST_STATEMENT_WHILE: next_value = as_f_while(ast, list); break;
		case AST_CALL: next_value = as_f_call(ast, list); break;
		case AST_VALUE: next_value = as_f_value(ast, list); break;
		case AST_ARRAY: next_value = as_f_array(ast, list); break;
		case AST_ACCESS: next_value = as_f_access(ast, list); break;
		case AST_NOOP: next_value = ast->str_value; break;
		case AST_STRING: next_value = as_f_const_string(ast, list); break;
		case AST_INT: next_value = as_f_int(ast, list); break;
		case AST_EXPR: next_value = as_f_expr(ast, list); break;
		default: println("[Assembler Frontend]: Unknown AST type '{s}'({i})", ast_type_to_str(ast->type), ast->type); exit(1);
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
