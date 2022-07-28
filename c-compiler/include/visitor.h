#pragma once

#include "AST.h"
#include "list.h"

struct Visitor {
	struct Ast * node;
	char * builtins;
	char * section_data;
	int data_count;
};

struct Ast * visitor_lookup(struct List *, const char *);

struct Visitor * init_visitor(struct Ast *);
struct Ast * visitor_visit(struct Visitor *, struct Ast *, struct List *);

struct Ast * visitor_visit_compound(struct Visitor *, struct Ast *, struct List *);
struct Ast * visitor_visit_function(struct Visitor *, struct Ast *, struct List *);
struct Ast * visitor_visit_assignment(struct Visitor *, struct Ast *, struct List *);
struct Ast * visitor_visit_declare(struct Visitor *, struct Ast *, struct List *);
struct Ast * visitor_visit_variable(struct Visitor *, struct Ast *, struct List *);
struct Ast * visitor_visit_statement(struct Visitor *, struct Ast *, struct List *);
struct Ast * visitor_visit_call(struct Visitor *, struct Ast *, struct List *);
struct Ast * visitor_visit_value(struct Visitor *, struct Ast *, struct List *);
struct Ast * visitor_visit_array(struct Visitor *, struct Ast *, struct List *);
struct Ast * visitor_visit_access(struct Visitor *, struct Ast *, struct List *);
