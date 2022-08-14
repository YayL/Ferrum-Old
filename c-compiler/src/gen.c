#include "gen.h"
#include "common.h"

void gen_jz(FILE * outfp, char * label, unsigned int u) {
	label = format(label, u);
	writef(outfp, "jz {s}\n", label);
	free(label);
}
void gen_jnz(FILE * outfp, char * label, unsigned int u) {
	label = format(label, u);
	writef(outfp, "jnz {s}\n", label);
	free(label);
}

void gen_func_start(FILE * outfp, char * name) {
	writef(outfp, "\nglobal {s}\n{s}:\n", name, name);
	fputs("push rbp\nmov rbp, rsp\n", outfp);
}

void gen_pre_inc_dec(FILE * outfp, char * op, int var_offset, char push) {
	gen_pop();
	writef(outfp, "{s} rax\n", op);
	gen_store_var(var_offset);
	if (push) {
		fputs("push rax\n", outfp);
	} else {
		fputs("mov [rsp], rax\n", outfp);
	}
}

void gen_post_inc_dec(FILE * outfp, char * op, int var_offset, char push) {
	gen_pop();
	if (push) {
		fputs("push rax\n", outfp);
	} else {
		fputs("mov [rsp], rax\n", outfp);
	}
	writef(outfp, "{s} rax\n", op);
	gen_store_var(var_offset);

}
