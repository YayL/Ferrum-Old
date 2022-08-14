#pragma once
#include "AST.h"

struct Ast * lookup(struct Ast *, char *);

void next(struct Visitor *, struct Ast *, struct Ast *);

void as_f_compound(struct Ast *);
void as_f_assignment(struct Ast *);
void as_f_variable(struct Ast *);
void as_f_declare(struct Ast *);
void as_f_return(struct Ast *);
void as_f_if(struct Ast *);
void as_f_for(struct Ast *);
void as_f_while(struct Ast *);
void as_f_do_while(struct Ast *);
void as_f_do(struct Ast *);
void as_f_break(struct Ast *);
void as_f_continue(struct Ast *);
void as_f_call(struct Ast *);
int as_f_int_arith(char, struct Ast *, struct Ast *);
int as_f_float_arith(char, struct Ast *, struct Ast *);
void as_f_expr(struct Ast *);
void as_f_array(struct Ast *);
void as_f_access(struct Ast *);
void as_f_const_string(struct Ast *);

void as_f_root(struct Ast *, struct Visitor *, FILE *);
void as_f(struct Ast *);
