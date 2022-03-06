#include "include/types.h"

#include "string.h"

int typename_to_int(const char* name) {

	int ret = 0;
	size_t len = strlen(name);

	for(unsigned int i = 0; i < len; ++i) {
		ret += name[i];
	}

	return ret;
}