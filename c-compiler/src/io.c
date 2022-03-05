#include "include/io.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char* read_file(const char* filename) {

	FILE * fp;
	char * line = NULL;
	size_t len = 0;

	fp = fopen(filename, "rb");
	if (fp == NULL) {
		printf("Cannot open file: '%s'\n", filename);
		exit(1);
	}

	char* buffer = calloc(1, sizeof(char));
	buffer[0] = '\0';

	while (getline(&line, &len, fp) != -1) {
		buffer = realloc(buffer, (strlen(buffer) + strlen(line) + 1) * sizeof(char));
		strcat(buffer, line);
	}

	fclose(fp);
	if (line)
		free(line);

	return buffer;
}