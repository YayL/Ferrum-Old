#pragma once

#include "fmt.h"

static FILE * outfp;

#define gen_push() fputs("push rax\n", outfp)
#define gen_pop() fputs("pop rax\n", outfp)

#define gen_test() fputs("test rax, rax\n", outfp)

#define gen_push_num(n) writef(outfp, "push {Li}\n", n)

#define gen_op(op) writef(outfp, "{s} rax, rbx\n", op)

#define gen_sop(op) writef(outfp, "{s} rax\n",op)

#define gen_call(name) writef(outfp, "call {s}\n", name)

#define gen_store_var(i) writef(outfp, "mov [rbp{Si}], rax\n", i)

#define gen_load_var(i) writef(outfp, "mov rax, [rbp{Si}]\n", i)
#define gen_load_creg(r) writef(outfp, "mov {s}, rax\n", r)

#define gen_ret() writef(outfp, "mov rsp, rbp\n"\
								"pop rbp\n"\
								"ret\n")

#define gen_stack_alloc(s) writef(outfp, "sub rsp, {u}\n", s)

void gen_jz(FILE *, char *, unsigned int);
void gen_jnz(FILE *, char *, unsigned int);
void gen_func_start(FILE *, char *);
void gen_pre_inc_dec(FILE *, char *, int, char);
void gen_post_inc_dec(FILE *, char *, int, char);
