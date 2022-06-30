#include "iron_builtins.h"

#include "common.h"
#include "as_f.h"

struct Ast * print(struct Visitor * visitor, struct Ast * node, struct List * list) {
	
	const char * func_template = 	"print:\n"
									"push rsi\n"
									"push rdx\n"
									"push rax\n"
									"push rdi\n"
									"mov rsi, rdi\n"
									"xor rdx, rdx\n"
									"strlen:\n"
									"mov al, [rsi + rdx]\n"
									"inc rdx\n"
									"cmp al, 0x0\n"
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
									"pop rdi\n"
									"pop rax\n"
									"pop rdx\n"
									"pop rsi\n"
									"ret\n";
	
	size_t size = strlen(visitor->root->str_value) + strlen(func_template) + 1;
	visitor->root->str_value = realloc(visitor->root->str_value, size);
	strcat(visitor->root->str_value, func_template);
	visitor->root->str_value[size - 1] = 0;

	return node;
}

struct Ast * show(struct Visitor * visitor, struct Ast * node, struct List * list) {

	const char * func_template = 	"\nshow:\n"
									"push rdx\n"
									"push rsi\n"
									"push rcx\n"
									"push rbx\n"
									"mov rax, rdi\n"
									"xor rdx, rdx\n"
									"mov rsi, 10\n"
									"mov rcx, 1\n"
									"push 10\n"
									"print_num_push_to_stack:\n"
										"inc rcx\n"
										"xor rdx, rdx\n"
										"idiv rsi\n"
										"add rdx, 30h\n"
										"push rdx\n"
										"cmp rax, 0\n"
										"jg print_num_push_to_stack\n"
									"mov rbx, rcx\n"
									"mov rax, 1\n"
									"mov rdi, 1\n"
									"mov rdx, 1\n"
									"print_num_print_stack:\n"
										"mov rsi, rsp\n"
										"syscall\n"
										"pop rsi\n"
										"dec rbx\n"
										"cmp rbx, 0\n"
										"jg print_num_print_stack\n"
									"pop rbx\n"
									"pop rcx\n"
									"pop rsi\n"
									"pop rdx\n"
									"ret\n";
	
	size_t size = strlen(visitor->root->str_value) + strlen(func_template) + 1;
	visitor->root->str_value = realloc(visitor->root->str_value, size);
	strcat(visitor->root->str_value, func_template);
	visitor->root->str_value[size - 1] = 0;

	return node;
}


void builtins_register_f_ptr(struct List * list, const char * name, struct Ast * (*f_ptr)()) {

	struct Ast * f_ptr_print_var = init_ast(AST_VARIABLE);

	char * function_name = calloc(strlen(name) + 1, sizeof(char));
	strcpy(function_name, name);

	f_ptr_print_var->name = function_name;
	f_ptr_print_var->f_ptr = f_ptr;

	list_push(list, f_ptr_print_var);
}

void init_builtins(struct List * list) {

	builtins_register_f_ptr(list, "print", print);
	builtins_register_f_ptr(list, "show", show);

}