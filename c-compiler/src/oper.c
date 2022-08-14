#include "oper.h"
#include "common.h"

#define op_not_found(op) println("[Error]: Unknown operator '{s}' used", op); exit(1)

unsigned int str_to_op(char * op) {
	if (op == NULL || op[0] == 0)
		return OP_ERR;
	unsigned int len = 0;
	while (op[++len]);

	if (len == 1)
		return op[0];

	switch (op[0]) {
	case '=':
		switch (op[1]) {
		case '=': return OP_EQ;
		op_not_found(op);
		}
	case '<': 
		switch (op[1]) {
		case '<': return OP_SHL;
		case '=': return OP_LE;
		op_not_found(op);
		}
	case '>':
		switch (op[1]) {
		case '>': return OP_SHR;
		case '=': return OP_GE;
		op_not_found(op);
		}
	case '!':
		switch (op[1]) {
		case '=': return OP_NE;
		op_not_found(op);
		}
	case '+':
		switch (op[1]) {
		case '+': return OP_INC;
		op_not_found(op);
		}
	case '-':
		switch (op[1]){
		case '-': return OP_DEC;
		op_not_found(op);
		}
	case '|':
		switch (op[1]) {
		case '|': return OP_LOGOR;
		op_not_found(op);
		}
	case '&':
		switch(op[1]) {
		case '&': return OP_LOGAND;
		}
	}
	op_not_found(op);
}
