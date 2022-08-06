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
	
	char * ast_val_val = as_f(ast_val->value, child_list);
	
	const char * template =	"\nglobal {s}\n"	
							"{s}:\n"
							"push rbp\n"
							"mov rbp, rsp\n"
							"sub rsp, {u}\n";

	char * ret_template = ".{s}_end:\n"
								"mov rsp, rbp\n"
								"pop rbp\n"
								"ret\n";
	ret_template = format(ret_template, ast->name);

	char * src = format(template, ast->name, ast->name, 
						(child_list->size - ast_val->nodes->size - 1) << 3);
	
	char * buf = format("{3s}", src, ast_val_val, ret_template);

	free(ast_val_val);
	free(ret_template);
	free(src);

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
							"pop rax\n"
							"jmp .{s}_end\n";

	struct Ast * func = list_at(list, 0);

	char * src = format(template, as_f(ast->value, list), func->name);

	#ifdef AS_F_DEBUG
		print_ast("\nDebug [Assembly Frontend]:\n\t{s}\n", ast);
		print_ast("\t{s}\n", ast->value);
	#endif

	return src;
}
 
char * as_f_if (struct Ast * ast, struct List * list) {
	
	const unsigned int index = ast->int_value;
	const char  * end_template = ".L{u}_end";

	char * src = 0,
		 * end = format(end_template, index);

	if (ast->value) {
		println("condition:");
	}

	const char * condition_template =	"{s}\n"
										".L{u}_{u}:\n"
										"{3s}"
										"jmp {s}\n",
				* else_template =	"{s}\n"
									".L{u}_{u}:\n"
									"{s}\n";
	
	unsigned int prev_size = list->size;

	struct Ast * expr;

	for (unsigned int i = 0; i < ast->nodes->size; ++i) {
		expr = list_at(ast->nodes, i);
		if (expr->value ) { // this is if and else if
			char * eval = as_f(expr->value, list);
			char * scope = as_f(expr, list), * cond = 0;
			if ((i + 1) < ast->nodes->size) {
				if (list_at(ast->nodes, i + 1) != NULL)
					cond = format("pop rax\ncmp al, 0\njz .L{u}_end\n", index);
				else
					cond = format("pop rax\ncmp al, 0\njz .L{u}_{u}\n", index, i + 1);
			}

			//print_ast("expr: {s}", expr);
			src = format(condition_template, src, index, i, eval, cond, scope, end);
			free(eval);
		} else { // this is for else statements
			char * scope = as_f(expr, list);
			src = format(else_template, src, index, i, scope);
		}
	}

	while (prev_size != list->size) list_pop(list);
	
	src = format("{2s}:\n", src, end);

	free(end);

	return src;
}

char * as_f_for(struct Ast * ast, struct List * list) {

	const char * template = "{s}"
							".L{u}_s:\n" // jump_target
							"{s}"		// evaluation
							"pop rax\n"
							"cmp al, 0\n"
							"jnz .L{u}_end\n"
							"{s}"		// scope
							".L{u}_c:\n" // jump_target
							"{s}"		// evaluation
							"jmp .L{u}_s\n"
							".L{u}_end:\n"; //jump_target

	const unsigned int index = ast->int_value, prev_size = list->size;

	struct Ast * node = list_at(ast->nodes, 0);
	char * scope = node != NULL ? as_f(node, list) : 0;
	node = ast->left;
	char * pre = node != NULL ? as_f(node, list) : 0;
	node = ast->right;
	char * post = node != NULL ? as_f(node, list) : 0;

	char * condition = as_f(ast->value, list);
	while (prev_size != list->size) list_pop(list);
	
	char * src = format(template, pre, index, condition, index, scope, index, post, index, 
						index);

	free(scope);
	free(pre);
	free(post);
	free(condition);

	return src;

}

char * as_f_while(struct Ast * ast, struct List * list) {

	const char * template = ".L{u}_c:\n"
							"{s}"
							"pop rax\n"
							"cmp al, 0\n"
							"jnz .L{u}_end\n"
							"{s}"
							"jmp .L{u}_c\n"
							".L{u}_end:\n";


	const unsigned int index = ast->int_value, prev_size = list->size;
	
	char * scope = as_f(ast->value, list);
	char * condition = as_f(ast->left, list);

	while (prev_size != list->size) list_pop(list);
	
	char * ret = format(template, index, condition, index, scope, index, index);

	free(scope);
	free(condition);

	return ret;
	
}

char * as_f_do(struct Ast * ast, struct List * list) {
	
	const unsigned int index = ast->int_value, prev_size = list->size;

	const char * template = ".L{u}_c\n"
							"{s}";
	
	char * src = as_f(ast->value, list);
	
	free(src);

	while (prev_size != list->size) list_pop(list);

	return format(template, index, src);

}

char * as_f_do_while(struct Ast * ast, struct List * list) {

	const char * template = ".L{u}_s:\n" // jump_target
							"{s}\n"		// scope
							".L{u}_c:\n" // jump_target
							"{s}\n"		// evaluation
							"pop rax\n"
							"cmp al, 0\n"
							"jz .L{u}_s\n"// jump_target
							".L{u}_end:\n"; //jump_target


	const unsigned int index = ast->int_value, prev_size = list->size;
	
	char * scope = as_f(ast->value, list);
	char * eval = as_f(ast->left, list);

	while (prev_size != list->size) list_pop(list);
	
	char * ret = format(template, index, scope, index, eval, index, index);

	free(scope);
	free(eval);

	return ret;
}

char * as_f_break(struct Ast * ast, struct List * list) {

	return format("jmp .L{u}_end ; break\n", ast->int_value);

}

char * as_f_continue(struct Ast * ast, struct List * list) {

	return format("jmp .L{u}_c ; continue\n", ast->int_value);
}

char * as_f_call(struct Ast * ast, struct List * list) {
	const char * template =	"push r8\n"
							"mov r8, rsp\n"
							"{s}"
							"call {s}\n"
							"mov rsp, r8\n"
							"pop r8\n"
							"{s}\n";

	char * argument_src = 0;
	unsigned int len = ast->nodes->size;

	for (size_t i = 0; i < len; ++i) {
		struct Ast * arg = list_at(ast->nodes, len - i - 1);
		char * curr_argument_src = as_f(arg, list);

		argument_src = format("{2s}", argument_src, curr_argument_src);
		free(curr_argument_src);
	}

	#ifdef AS_F_DEBUG
		print_ast("\nDebug [Assembly Frontend]: 11111\n\t{s}\n", ast);
	#endif
	
	char * ret = format(template, argument_src, ast->name, ast->push ? "push rax" : "");
	free(argument_src);
	return ret;

}

const char * cmp_template =	"pop rax\n"
							"pop rbx\n"
							"cmp rax, rbx\n"
							"{s} al\n"
							"push rax\n",
			* op_template =	"pop {s}\n"
							"pop {s}\n"
							"{s}\n"
							"push {s}\n";

char * as_f_1_binop(char op) {
	
	switch (op) {
		case '+': return format(op_template, "rbx", "rax", "add rax, rbx", "rax");
		case '-': return format(op_template, "rax", "rbx", "sub rax, rbx", "rax");
		case '*': return format(op_template, "rbx", "rax", "mul rbx", "rax");
		case '/': return format(op_template, "rax", "rbx", "div rbx", "rax");
		case '%': return format(op_template, "rax", "rbx", "div rbx", "rdx");
		case '^': return format(op_template, "rbx", "rax", "xor rax, rbx", "rax");
		case '|': return format(op_template, "rbx", "rax", "or rax, rbx", "rax");
		case '&': return format(op_template, "rbx", "rax", "and rax, rbx", "rax");
		case '<': return format(cmp_template, "setl");
		case '>': return format(cmp_template, "setg");
		case '!': return format(cmp_template, "sete");
		default:
			println("[Compiler] Error: Operator '{c}' is not supported.", op);
			exit(1);
	}

}

char * as_f_expr(struct Ast * ast, struct List * list) {
	
	char * left = 0, * right = 0, * operation = 0, * op = 0;

	unsigned int operator_len = 0;

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

	struct Ast * var = NULL;	

	switch (operator_len) {
		case 0:
			println("[Compiler] Error: There was no operator specified."); exit(1);
		case 1:
			operation = as_f_1_binop(op[0]);
			break;
		case 2:
			if (strcmp(op, "++") == 0) {
				if (ast->right != NULL) { // ++var
					if (ast->left == NULL) {
						left = 0;
					}
					var = lookup(list, ast->right->name);
					operation = format("mov rax, [rbp{Si}]\n"
							"inc rax\n"
							"mov [rbp{Si}], rax\n"
							"{s}\n", var->int_value, var->int_value, var->push
																	? "push rax"
																	: "mov [rsp], rax");
				} else { // var++
					if (ast->right == NULL) {
						right = 0;
					}
					var = lookup(list, ast->left->name);
					operation = format ("mov rax, [rbp{Si}]\n"
							"{s}\n"
							"inc rax\n"
							"mov [rbp{Si}], rax\n", var->int_value, var->push
																	? "push rax"
																	: "mov [rsp], rax"
							, var->int_value);
				}
			} else if (strcmp(op, "--") == 0) {
				if (ast->right != NULL) { // --var
					if (ast->left == NULL) {
						left = 0;
					}
					var = lookup(list, ast->right->name);
					operation = format("mov rax, [rbp{Si}]\n"
							"dec rax\n"
							"mov [rbp{Si}], rax\n"
							"{s}\n", var->int_value, var->int_value, var->push
																	? "push rax"
																	: "mov [rsp], rax");
				} else { // var--
					if (ast->right == NULL) {
						right = 0;
					}
					var = lookup(list, ast->left->name);
					operation = format ("mov rax, [rbp{Si}]\n"
							"{s}\n"
							"dec rax\n"
							"mov [rbp{Si}], rax\n", var->int_value, var->push
																	? "push rax"
																	: "mov [rsp], rax"
							, var->int_value);
			}
		} else if (strcmp(op, "<<") == 0) {
			operation = format(op_template, "rax", "rcx", "shl rax, cl", "rax");
		} else if (strcmp(op, ">>") == 0) {
			operation = format(op_template, "rax", "rbx", "shr rax, rbx", "rax");
		} else if (strcmp(op, "==") == 0) {
			operation = format(cmp_template, "sete");
		} else if (strcmp(op, "!=") == 0) {
			operation = format(cmp_template, "setz");
		} else {
			println("[Compiler] Error: MOperator '{s}' is not supported.", op);
			exit(1);
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

	#ifdef AS_F_DEBUG
		print_ast("Debug [Assembly Frontend]: {s}\n", ast);
	#endif

	switch(ast->type) {
		case AST_COMPOUND: return as_f_compound(ast, list);
		case AST_FUNCTION: return as_f_function(ast, list);
		case AST_ASSIGNMENT: return as_f_assignment(ast, list);
		case AST_DECLARE: return as_f_declare(ast, list);
		case AST_VARIABLE: return as_f_variable(ast, list);
		case AST_FOR: return as_f_for(ast, list);
		case AST_IF: return as_f_if(ast, list);
		case AST_RETURN: return as_f_return(ast, list);
		case AST_WHILE: return as_f_while(ast, list);
		case AST_DO: return as_f_do(ast, list);
		case AST_DO_WHILE: return as_f_do_while(ast, list);
		case AST_BREAK: return as_f_break(ast, list);
		case AST_CONTINUE: return as_f_continue(ast, list);
		case AST_CALL: return as_f_call(ast, list);
		case AST_VALUE: return as_f_value(ast, list);
		case AST_ARRAY: return as_f_array(ast, list);
		case AST_ACCESS: return as_f_access(ast, list);
		case AST_NOOP: return ast->str_value;
		case AST_STRING: return as_f_const_string(ast, list);
		case AST_INT: return as_f_int(ast, list);
		case AST_EXPR: return as_f_expr(ast, list);
		default: 
			println("[Assembler Frontend]: Unknown AST type '{s}'({i})", ast_type_to_str(ast->type), ast->type); 
			exit(1);
	}

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
							"{2s:\n}\n";

	char * next_value = as_f(root, list);

	return format(template, visitor->section_data, visitor->builtins, next_value);
}
