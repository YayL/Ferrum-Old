#pragma once

enum OP_t {
	OP_ERR,
	OP_SHR,
	OP_SHL,
	OP_NE,
	OP_EQ,
	OP_LE,
	OP_GE,
	OP_LOGOR,
	OP_LOGAND,
	OP_INC,
	OP_DEC,
};

unsigned int str_to_op(char *);
