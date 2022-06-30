#include "as_f.h"

#include "common.h"

struct Ast * lookup(struct List * list, const char * name) {
	for(size_t i = 0; i < list->size; ++i) {
		struct Ast * ret = list->items[i];
		if(strcmp(ret->name, name) == 0) {
			return ret;
		}
	}
	return NULL;
}

char* as_f_compound(struct Ast * ast, struct List * list) {

	size_t length = 1;
	char* value = calloc(length, sizeof(char));

	for(int i = 0; i < ast->nodes->size; ++i) {

		struct Ast * child_ast = (struct Ast *) list_at(ast->nodes, i);
		#ifdef AS_F_DEBUG
			printf("\t%s\n", ast_to_str(child_ast));
		#endif
		char* next_value = as_f(child_ast, list);

		length += strlen(next_value);
		
		value = realloc(value, length * sizeof(char));
		
		strcat(value, next_value);
		value[length - 1] = 0;
	}

	return value;
}

char* as_f_function(struct Ast * ast, struct List * list) {

	const char * template =  "\nglobal %s\n"
								"%s:\n"
								"push rbp\n"
								"mov rbp, rsp\n";

	size_t length = strlen(template) + (2 * strlen(ast->name)) + 1;
	char * src = malloc(length * sizeof(char));

	snprintf(src, length, template, ast->name, ast->name);
	src[length - 1] = 0;

	struct Ast * ast_val = ast->value;
	
	#ifdef AS_F_DEBUG
		printf("Debug [Assembly Frontend]: Number of Arguments for function: %s(%d)\n", ast->name, ast_val->nodes->size);
	#endif

	const size_t size = ast_val->nodes->size; 

	for (size_t i = 0; i < size; ++i) {
		struct Ast * variable = init_ast(AST_VARIABLE);

		variable->name = ((struct Ast *)ast_val->nodes->items[i])->name;
		variable->int_value = (i + 2) << 3;

		list_push(list, variable);
	}

	char * ast_val_val = as_f(ast_val->value, list);
	
	length += strlen(ast_val_val);
	src = realloc(src, length * sizeof(char));
	strcat(src, ast_val_val);
	src[length - 1] = 0;


	return src;

}

char* as_f_assignment(struct Ast * ast, struct List * list) {
	
	const char* template = "mov eax, 128\n";
	char* src = calloc(strlen(template) + 1, sizeof(char));
	strcpy(src, template);

	return src;
}

char* as_f_variable(struct Ast * ast, struct List * list) {

	struct Ast * variable = lookup(list, ast->name);

	if (!variable) {
		printf("Error: Variable `%s` has not been declared in this scope\n", ast->name);
		exit(1);
	}

	const char * template = "mov rdi, [rsp+%d]\n";
	const size_t length = strlen(template) + (variable->int_value / 10);
	char * src = malloc(length * sizeof(char));

	snprintf(src, length, template, variable->int_value);
	src[length - 1] = 0;

	return src;

}

char * as_f_statement(struct Ast * ast, struct List * list) {
	if (strncmp(ast->name, "return", 5) == 0) {
		const char * template = "%s" // mov rdi, %s
								"mov rsp, rbp\n"
								"pop rbp\n"
								"ret\n";


		struct Ast * ret_ast = (struct Ast * ) 
									ast->value
										? ast->value
										: (void* )0;

		// printf("ret_ast: %s\n", ast_to_str(ret_ast));

		char * ret_val = "mov rdi, 0";

		if (ret_ast) {
			switch(ret_ast->type) {
				case AST_VARIABLE: ret_val = as_f_variable(ast->value, list); break;
				case AST_ACCESS: ret_val = as_f_access(ast->value, list); break;
			}
		}

		size_t length = strlen(template) + strlen(ret_val);
		char * src = malloc(length * sizeof(char));

		snprintf(src, length, template, ret_val);
		src[length - 1] = 0;

		#ifdef AS_F_DEBUG
			printf("\nDebug [Assembly Frontend]:\n\t%s\n\t%s\n", ast_to_str(ast), ast_to_str(ast->value));
		#endif

		return src;
	}

	return "as_f_statement\n";

}

char* as_f_call(struct Ast * ast, struct List * list) {
	const char * template = "%s"
							"call %s\n";

	size_t size = 1, length;
	char * argument_src = calloc(size, sizeof(char));

	for (size_t i = 0; i < ast->nodes->size; ++i) {

		char * curr_argument_src = as_f(ast->nodes->items[i], list);

		length = strlen(curr_argument_src);
		size += length;

		argument_src = realloc(argument_src, size);
		strncat(argument_src, curr_argument_src, length);
		argument_src[size - 1] = 0;
	}

	size += strlen(template) + strlen(ast->name);
	char * src = malloc(size * sizeof(char));

	snprintf(src, size, template, argument_src, ast->name);
	src[size - 1] = 0;

	#ifdef AS_F_DEBUG
		printf("\nDebug [Assembly Frontend]:\n\t%s\n", ast_to_str(ast));
	#endif
	
	return src;

}

char* as_f_value(struct Ast * ast, struct List * list) {

	return "as_f_value\n";

}

char * as_f_array(struct Ast * ast, struct List * list) {
	return "as_f_array";
}

char * as_f_access(struct Ast * ast, struct List * list) {
	
	char * src = calloc(1, sizeof(char));

	const int index = ast->int_value;
	struct Ast * variable = lookup(list, ast->name);

	if (!variable) {
		printf("Error: Variable `%s` has not been declared in this scope\n", ast->name);
		exit(1);
	}

	const int id = variable->int_value + (ast->value->int_value << 3);

	const char * template = "mov rdi, [rsp+%d]\n";
	const size_t length = strlen(template) + (id / 10);
	src = malloc(length * sizeof(char));

	snprintf(src, length, template, id);
	src[length - 1] = 0;

	return src;

}

char* as_f(struct Ast * ast, struct List * list) {
	
	char * next_value = "";

	#ifdef AS_F_DEBUG
		printf("Debug [Assembly Frontend]: %s\n", ast_to_str(ast));
	#endif

	switch(ast->type) {
		case AST_COMPOUND: next_value = as_f_compound(ast, list); break;
		case AST_FUNCTION: next_value = as_f_function(ast, list); break;
		case AST_ASSIGNMENT: next_value = as_f_assignment(ast, list); break;
		case AST_VARIABLE: next_value = as_f_variable(ast, list); break;
		case AST_STATEMENT: next_value = as_f_statement(ast, list); break;
		case AST_CALL: next_value = as_f_call(ast, list); break;
		case AST_VALUE: next_value = as_f_value(ast, list); break;
		case AST_ARRAY: next_value = as_f_array(ast, list); break;
		case AST_ACCESS: next_value = as_f_access(ast, list); break;
		case AST_NOOP: next_value = ast->str_value; break;
		default: printf("[Assembler Frontend]: Unknown AST type '%d'\n", ast->type); exit(1);
	}

	const size_t size = strlen(next_value) + 1;
	char* value = malloc(size);
	strncpy(value, next_value, size);
	value[size - 1] = 0;

	return value;
}

char * as_f_root(struct Ast * root, struct List * list) {

	const char * start = 	"BITS 64\n"
							"section .data\n"
							"newline db 0xA, 0xD\n"
							"newline_len equ $-newline\n\n"
							"section .text\n"
							"global _start\n"
							"_start:\n"
							"call main\n"
							"mov eax, 60\n"
							"syscall\n"
							"\n%s";

	char * next_value = as_f(root, list);
	const size_t next_size = strlen(next_value);

	size_t size = strlen(start) + strlen(root->str_value);
	char * value = malloc(size + next_size + 1);
	// strncpy(value, start, size);
	snprintf(value, size, start, root->str_value);
	strncat(value, next_value, next_size);
	value[size + next_size] = 0;

	return value;
}