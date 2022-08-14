#pragma once

#include "AST.h"
#include "list.h"

struct Visitor {
	struct Ast * node;
	struct Ast * root;
	char * builtins;
	char * section_data;
	unsigned int data_count, jump_targets;
	FILE * out;
};

struct Ast * visitor_lookup(struct Ast *, const char *);

struct Visitor * init_visitor(struct Ast *, FILE *);
void visitor_visit(struct Visitor *, struct Ast *);
void visitor_visit_root(struct Visitor *, struct Ast *);

void visitor_visit_compound(struct Visitor *, struct Ast *);
void visitor_visit_function(struct Visitor *, struct Ast *);
void visitor_visit_assignment(struct Visitor *, struct Ast *);
void visitor_visit_declare(struct Visitor *, struct Ast *);
void visitor_visit_variable(struct Visitor *, struct Ast *);
void visitor_visit_return(struct Visitor *, struct Ast *);
void visitor_visit_if(struct Visitor *, struct Ast *);
void visitor_visit_while(struct Visitor *, struct Ast *);
void visitor_visit_for(struct Visitor *, struct Ast *);
void visitor_visit_do_while(struct Visitor *, struct Ast *);
void visitor_visit_do(struct Visitor *, struct Ast *);
void visitor_visit_call(struct Visitor *, struct Ast *);
void visitor_visit_value(struct Visitor *, struct Ast *);
void visitor_visit_array(struct Visitor *, struct Ast *);
void visitor_visit_access(struct Visitor *, struct Ast *);
void visitor_visit_expr(struct Visitor *, struct Ast *);
