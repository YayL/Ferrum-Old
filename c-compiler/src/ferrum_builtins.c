#include "ferrum_builtins.h"

#include "common.h"
#include "as_f.h"

void set_arg_length(struct Visitor * visitor, struct Ast * node, unsigned int len) {

	struct Ast * func = visitor_lookup(visitor->root, node->name);
	func->int_value = len;
	node->int_value = len;

}

struct Ast * _builtin_print(struct Visitor * visitor, struct Ast * node, struct List * list) {
	
	const char * func_template = "print:\n"
				"push rbp\n"
				"mov rbp, rsp\n"
				"mov rsi, [rbp+0x10]\n"
				"xor rdx, rdx\n"
				"strlen:\n"
					"mov al, [rsi + rdx]\n"
					"inc rdx\n"
					"test al\n"
					"jnz strlen\n"
				"dec rdx\n"
				"mov rax, 1\n"
				"mov rdi, 1\n"
				"syscall\n"
				"mov rsi, newline\n"
				"mov rdx, newline_len\n"
				"mov rax, 1\n"
				"mov rdi, 1\n"
				"syscall\n"
				"pop rbp\n"
				"ret\n";
	
	visitor->builtins = format(visitor->builtins, func_template);
	node->push = 0;
	
	set_arg_length(visitor, node, 1);

	return node;
}

struct Ast * _builtin_put(struct Visitor * visitor, struct Ast * node, struct List * list) {

    if (node->nodes->size != 1 || ((struct Ast *) node->nodes->items[0])->type != AST_STRING) {
      println("Error [Runtime]: Funtion put takes only one string literal!");
    }

    const char * func_template = "put:\n"
                "mov rsi, [rsp + 0x10]\n" // length
                "mov rdx, [rsp + 0x08]\n" // str address
                "mov rax, 1\n"
                "mov rdi, 1\n"
                "syscall\n"
                "ret\n";
	
	visitor->builtins = format("{2s}", visitor->builtins, func_template);
	node->push = 0;
	
	set_arg_length(visitor, node, 2);

	return node;

}

struct Ast * _builtin_show(struct Visitor * visitor, struct Ast * node, struct List * list) {

	const char * func_template = "\nshow:\n"
				"push rbp\n"
				"mov rbp, rsp\n"
				"mov rax, [rbp+0x10]\n"
				"xor rdx, rdx\n"
				"mov rsi, 10\n"
				"mov rcx, 1\n"
				"push 10\n"
				"show_push_to_stack:\n"
					"inc rcx\n"
					"xor rdx, rdx\n"
					"idiv rsi\n"
					"add rdx, 30h\n"
					"push rdx\n"
					"cmp rax, 0\n"
				"jg show_push_to_stack\n"
				"mov rax, 1\n"
				"mov rdi, 1\n"
				"mov rdx, 1\n"
				"show_print_stack:\n"
					"mov rsi, rsp\n"
					"push rcx\n"
					"syscall\n"
					"pop rcx\n"
					"pop rsi\n"
					"dec rcx\n"
					"cmp rcx, 0\n"
				"jg show_print_stack\n"
				"pop rbp\n"
				"ret\n";


	visitor->builtins = format("{2s}", visitor->builtins, func_template);
	node->push = 0;

	set_arg_length(visitor, node, 1);

	return node;
}


void builtins_register_f_ptr(struct List * list, const char * name, struct Ast * (*f_ptr)()) {

	struct Ast * f_ptr_print_var = init_ast(AST_VARIABLE);

	f_ptr_print_var->name = format("{s}", name);
	f_ptr_print_var->f_ptr = f_ptr;

	list_push(list, f_ptr_print_var);
}

void init_builtins(struct List * list) {

	builtins_register_f_ptr(list, "print", _builtin_print);
	builtins_register_f_ptr(list, "show", _builtin_show);
	builtins_register_f_ptr(list, "put", _builtin_put);

}
