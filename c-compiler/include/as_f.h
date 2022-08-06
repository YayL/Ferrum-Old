#pragma once
#include "AST.h"

struct Ast * lookup(struct List *, char *);

char * as_f_compound(struct Ast *, struct List *);
char * as_f_assignment(struct Ast *, struct List *);
char * as_f_variable(struct Ast *, struct List *);
char * as_f_declare(struct Ast *, struct List *);
char * as_f_return(struct Ast *, struct List *);
char * as_f_if(struct Ast *, struct List *);
char * as_f_for(struct Ast *, struct List *);
char * as_f_while(struct Ast *, struct List *);
char * as_f_do_while(struct Ast *, struct List *);
char * as_f_do(struct Ast *, struct List *);
char * as_f_break(struct Ast *, struct List *);
char * as_f_continue(struct Ast *, struct List *);
char * as_f_call(struct Ast *, struct List *);
char * as_f_1_binop(char);
char * as_f_expr(struct Ast *, struct List *);
char * as_f_value(struct Ast *, struct List *);
char * as_f_array(struct Ast *, struct List *);
char * as_f_access(struct Ast *, struct List *);
char * as_f_const_string(struct Ast *, struct List *);

char * as_f_root(struct Ast *, struct Visitor *, struct List *);
char * as_f(struct Ast *, struct List *);
