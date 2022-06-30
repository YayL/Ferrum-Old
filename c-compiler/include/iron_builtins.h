#pragma once

#include "visitor.h"
#include "AST.h"
#include "list.h"

struct Ast * f_ptr_print(struct Visitor *, struct Ast *, struct List *);

void init_builtins(struct List *);