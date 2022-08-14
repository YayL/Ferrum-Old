/* TODO:
	
   Add a basic implementation of the ferrum type system

*/

#include "as_f.h"
#include "common.h"
#include "gen.h"
#include "oper.h"

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

struct Ast * lookup(struct Ast * scope, char * name) {
	do {
		for(size_t i = 0; i < scope->v_variables->size; ++i) {
			struct Ast * child_ast = list_at(scope->v_variables, i);
			if (child_ast->name == NULL)
				continue;

			if(strcmp(child_ast->name, name) == 0) 
				return child_ast;
		}
		scope = scope->scope;
	} while (scope->type != AST_ROOT);
	return NULL;
}

void as_f_compound(struct Ast * node) {

	char * src = 0;

	for(int i = 0; i < node->nodes->size; ++i) {
		struct Ast * child = list_at(node->nodes, i);
		#ifdef AS_F_DEBUG
			print_ast("\t{s}\n", child);
		#endif
		as_f(child);
	}
}

void as_f_function(struct Ast * node) {
	
	#ifdef AS_F_DEBUG
		print("[Assembly Frontend]: Number of Arguments for function: {s}({i})\n", node->name, node->nodes->size);
	#endif
		
	gen_func_start(outfp, node->name);
	gen_stack_alloc(node->int_value);

	as_f(node->value); // scope
	
	writef(outfp, ".{s}_end:\n", node->name);
	gen_ret();
}

void as_f_assignment(struct Ast * node) {	
	struct Ast * variable = lookup(node->scope, node->name);

	if (!variable) {
		println("[Compiler Error]: Variable `{s}` has not been declared in this scope", node->name);
		exit(1);
	}
	
	as_f(node->value);
	gen_pop();
	gen_store_var(variable->int_value);
	if (node->push)
		gen_push();
}

void as_f_variable(struct Ast * node) {
	struct Ast * variable = lookup(node->scope, node->name);

	if (variable == NULL) {
		println("[Compiler Error]: Variable `{s}` has not been declared in this scope", node->name);
		exit(1);
	}

	const char * template =	"; variable\n"
							"mov rax, [rbp{Si}]\n"
							"push rax\n";

	gen_load_var(variable->int_value);
	gen_push();
}

void as_f_declare(struct Ast * node) {
	struct Ast * variable = visitor_lookup(node->scope, node->name);
	list_push(node->scope->variables, variable);

	if (node->value) {
		as_f_assignment(node);
	}
}

void as_f_return(struct Ast * node) {
	as_f(node->value);
	gen_pop();
	writef(outfp, "jmp .{s}_end\n", node->str_value);

	#ifdef AS_F_DEBUG
		print_ast("\n[Assembly Frontend]:\n\t{s}\n", node);
		print_ast("\t{s}\n", node->value);
	#endif
}
 
void as_f_if (struct Ast * node) {
	for (unsigned int i = 0; i < node->nodes->size; ++i) {
		struct Ast * expr = node->nodes->items[i];
		if (expr->left) { // if and else if blocks
			writef(outfp, ".L{u}_{u}:\n", node->int_value, i); // start label
			as_f(expr->left); // eval
			gen_pop();
			gen_test();
			println("111");
			if (list_at(node->nodes, i + 1) == NULL) {
				gen_jz(outfp, ".L{u}_end\n", node->int_value);
			} else {
				writef(outfp, "jz .L{u}_{u}\n", node->int_value, i + 1);
			}
			as_f(expr);
			writef(outfp, "jmp .L{u}_end\n", node->int_value);
		} else { // else block
			writef(outfp, ".L{u}_{u}:\n", node->int_value, i);
			as_f(expr);
		}
	}
	
	writef(outfp, ".L{u}_end:\n", node->int_value);
}

void as_f_for(struct Ast * node) {
	if (node->left) 
		as_f(node->left); // pre

	writef(outfp, ".L{u}_s:\n", node->int_value);

	as_f(node->value); // condition
	
	gen_pop();
	gen_test();
	gen_jz(outfp, ".L{u}_end", node->int_value);
	
	struct Ast * scope = list_at(node->nodes, 0);
	if (scope != NULL)
		as_f(scope);

	writef(outfp, ".L{u}_c:\n", node->int_value);

	if (node->right != NULL)
		as_f(node->right);

	writef(outfp, "jmp .L{u}_s\n", node->int_value);
	writef(outfp, ".L{u}_end:\n", node->int_value);
}

void as_f_while(struct Ast * node) {
	writef(outfp, ".L{u}_c:\n", node->int_value);
	as_f(node->left); // condition
	gen_pop();
	gen_test();
	gen_jz(outfp, ".L{u}_end\n", node->int_value);
	as_f(node->value); // scope
	writef(outfp, "jmp .L{u}_c\n", node->int_value);
	writef(outfp, ".L{u}_end:\n", node->int_value);
	
}

void as_f_do(struct Ast * node) {	
	
	writef(outfp, ".L{u}_c:\n", node->int_value);
	as_f(node->value);
	writef(outfp, ".L{u}_end:\n", node->int_value);
}

void as_f_do_while(struct Ast * node) {

	writef(outfp, ".L{u}_s:\n", node->int_value);
	as_f(node->value);
	writef(outfp, ".L{u}_c:\n", node->int_value);
	as_f(node->left);
	gen_pop();
	gen_test();
	gen_jnz(outfp, ".L{u}_s\n", node->int_value);
	writef(outfp, ".L{u}_end:\n", node->int_value);
}

void as_f_break(struct Ast * node) {
	writef(outfp, "jmp .L{u}_end\n", node->int_value);
}

void as_f_continue(struct Ast * node) {
	writef(outfp, "jmp .L{u}_c\n", node->int_value);
}

void as_f_call(struct Ast * node) {
	unsigned int len = node->nodes->size;

	fputs("push r8\nmov r8, rsp\n", outfp);
	for (size_t i = 0; i < len; ++i) {
		struct Ast * arg = list_at(node->nodes, len - i - 1);
		as_f(arg);
	}
	
	gen_call(node->name);
	fputs("mov rsp, r8\npop r8\n", outfp);
	if (node->push)
		fputs("push rax\n", outfp);

	#ifdef AS_F_DEBUG
		print_ast("\n[Assembly Frontend]: \n\t{s}\n", node);
	#endif
}

int as_f_int_arith(char op, struct Ast * node, struct Ast * variable) {
	const char * cmp_template = "pop rax\n"
							"pop rbx\n"
							"cmp rax, rbx\n"
							"{s} al\n"
							"push rax\n",
			* op_template =	"pop {s}\n"
							"pop {s}\n"
							"{s}\n"
							"push {s}\n";

	switch (op) {
		case OP_DEC: {
						if (variable == NULL) {
							println("[Syntax Error]: Operator '--' must operate directly on a variable.");exit(1);
						}else if (node->left != NULL) // var++
							gen_post_inc_dec(outfp, "dec", variable->int_value, variable->push != 0);
						else 
							gen_pre_inc_dec(outfp, "dec", variable->int_value, variable->push != 0);
						break;
					 }
		case OP_INC: {
						if (variable == NULL) {
							println("[Syntax Error]: Operator '++' must operate directly upon a variable.");exit(1);
						}else if (node->left != NULL) // var++
							gen_post_inc_dec(outfp, "inc", variable->int_value, variable->push != 0);
						else 
							gen_pre_inc_dec(outfp, "inc", variable->int_value, variable->push != 0);
						break;
					 }
		case '+': writef(outfp, op_template, "rax", "rbx", "add rax, rbx", "rax"); break;
		case '-': writef(outfp, op_template, "rax", "rbx", "sub rax, rbx", "rax"); break;
		case '*': writef(outfp, op_template, "rax", "rbx", "mul rbx", "rax"); break;
		case '/': writef(outfp, op_template, "rax", "rbx", "div rbx", "rax"); break;
		case '%': writef(outfp, op_template, "rax", "rbx", "div rbx", "rdx"); break;
		case '^': writef(outfp, op_template, "rax", "rbx", "xor rax, rbx", "rax"); break;
		case '|': writef(outfp, op_template, "rax", "rbx", "or rax, rbx", "rax"); break;
		case '&': writef(outfp, op_template, "rax", "rbx", "and rax, rbx", "rax"); break;
		case OP_SHR: writef(outfp, op_template, "rax", "rcx", "sar rax, cl", "rax"); break;
		case OP_SHL: writef(outfp, op_template, "rax", "rcx", "sal rax, cl", "rax"); break;
		case OP_EQ: writef(outfp, cmp_template, "sete"); break;
		case OP_NE: writef(outfp, cmp_template, "setne"); break;
		case OP_GE: writef(outfp, cmp_template, "setnb"); break;
		case OP_LE: writef(outfp, cmp_template, "setna"); break;
		case '<': writef(outfp, cmp_template, "setb"); break;
		case '>': writef(outfp, cmp_template, "setg"); break;
		case '!': writef(outfp, cmp_template, "sete"); break;
		case OP_LOGOR: break;
		case OP_LOGAND: break;
		case OP_ERR:
		default: return 1;
	}
	return 0;
}

int as_f_float_arith(char op, struct Ast * node, struct Ast * variable) {
		const char * cmp_template = "pop rax\n"
									"pop rbx\n"
									"cmp rax, rbx\n"
									"{s} al\n"
									"push rax\n",
			* op_template =	"pop {s}\n"
							"pop {s}\n"
							"{s}\n"
							"push {s}\n";
	
	return 0;
}

void as_f_expr(struct Ast * node) {
	
	struct Ast * var = NULL;

	if (node->right) {
		#ifdef AS_F_DEBUG
			print_ast("right: {s}\n", node->right);
		#endif
		as_f(node->right);
		if (node->right->name != NULL)
			var = lookup(node->scope, node->right->name);
	}

	if (node->left) {
		#ifdef AS_F_DEBUG
			print_ast("left: {s}\n", node->left);
		#endif
		as_f(node->left);
		if (node->left->name != NULL)
			var = lookup(node->scope, node->left->name);
	}
	
	if (node->value == NULL) {
		println("[Warning]: No operator in expression.");
		return;
	}

	char op = str_to_op(node->value->name);
	if (as_f_int_arith(op, node, var)) {
		println("[Error]: Unknown operator: {s}", node->value->name);
		print_ast("{s}\n", node->value);
		exit(1);
	}

}

void as_f_array(struct Ast * node) {
	fputs("; as_f_array\n", outfp);
}

void as_f_access(struct Ast * node) {
	struct Ast * variable = lookup(node->scope, node->name);

	if (!variable) {
		println("Error: Variable `{s}` has not been declared in this scope", node->name);
		exit(1);
	}
	
	const int id = variable->int_value + (node->value->int_value << 3);

	writef(outfp, "mov rax, [rbp{Si}]\n", id);

}

void as_f_const_string(struct Ast * node) {

	writef(outfp,	"push {s}\n"
					"push {s}_len\n", node->name, node->name);	
}

void as_f_int(struct Ast * node) {
	writef(outfp,"mov rax, {i}\n", node->int_value);
	if (node->push)
		fputs("push rax\n", outfp);
}

void as_f(struct Ast * node) {

	#ifdef AS_F_DEBUG
		print_ast("[Assembly Frontend]: {s}\n", node);
	#endif

	switch(node->type) {
		case AST_COMPOUND: as_f_compound(node); break;
		case AST_FUNCTION: as_f_function(node); break;
		case AST_ASSIGNMENT: as_f_assignment(node); break;
		case AST_DECLARE: as_f_declare(node); break;
		case AST_VARIABLE:as_f_variable(node); break;
		case AST_FOR: as_f_for(node); break;
		case AST_IF: as_f_if(node); break;
		case AST_RETURN: as_f_return(node); break;
		case AST_WHILE: as_f_while(node); break;
		case AST_DO: as_f_do(node); break;
		case AST_DO_WHILE: as_f_do_while(node); break;
		case AST_BREAK: as_f_break(node); break;
		case AST_CONTINUE: as_f_continue(node); break;
		case AST_CALL: as_f_call(node); break;
		case AST_ARRAY: as_f_array(node); break;
		case AST_ACCESS: as_f_access(node); break;
		case AST_NOOP: fputs(node->str_value, outfp); break;
		case AST_STRING: as_f_const_string(node); break;
		case AST_INT: as_f_int(node); break;
		case AST_EXPR: as_f_expr(node); break;
		default: 
			println("[Error]: Unknown AST type '{s}'({i})", ast_type_to_str(node->type), node->type); 
			exit(1);
	}

}

void as_f_root(struct Ast * root, struct Visitor * visitor, FILE * out) {

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
		
	outfp = out;

	fputs("BITS 64\n", out);
	if (visitor->section_data != NULL)
		fputs(visitor->section_data, out);
	fputs(	"section .text\n"
			"global _start:\n"
			"_start:\n"
			"call main\n"
			"mov edi, eax\n"
			"mov eax, 60\n"
			"syscall\n\n", out);
	if (visitor->builtins != NULL)
		fputs(visitor->builtins, out);
	as_f(root);
}
