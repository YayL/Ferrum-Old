#include "io.h"

#include "common.h"

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

void write_file(const char * filename, char * write_buffer) {
	FILE * fp;
	
	fp = fopen(filename, "wb");
	if (fp == NULL) {
		printf("Error: Unable to open file: '%s'\n", filename);
		exit(1);
	}

	if (!fputs(write_buffer, fp)) {
		printf("Error: Unable to write buffer to file: '%s'", filename);
		exit(1);
	}
	fclose(fp);
}