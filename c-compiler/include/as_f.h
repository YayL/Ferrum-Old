#pragma once
#include "AST.h"

struct Ast * lookup(struct List *, const char *);

char * as_f_compound(struct Ast *, struct List *);
char * as_f_assignment(struct Ast *, struct List *);
char * as_f_variable(struct Ast *, struct List *);
char * as_f_declare(struct Ast *, struct List *);
char * as_f_statement(struct Ast *, struct List *);
char * as_f_call(struct Ast *, struct List *);
char * as_f_value(struct Ast *, struct List *);
char * as_f_array(struct Ast *, struct List *);
char * as_f_access(struct Ast *, struct List *);
char * as_f_const_string(struct Ast *, struct List *);

char * as_f_root(struct Ast *, struct Visitor *, struct List *);
char * as_f(struct Ast *, struct List *);
