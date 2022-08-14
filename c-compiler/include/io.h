#pragma once

#include "common.h"

FILE * open_file(const char *, const char *);
char* read_file(const char*);
void write_file(const char *, char *);
