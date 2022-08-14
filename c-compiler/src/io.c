#include "io.h"

#include "common.h"

FILE * open_file (const char * filename, const char * options) {
	FILE * out = fopen(filename, options);
	if (out == NULL) {
		println("[Error]: Cannot open file: '{s}'", filename);
		exit(1);
	}
	return out;
}

char* read_file(const char* filename) {

	FILE * fp;
	char * line = NULL;
	size_t len = 0;

	fp = open_file(filename, "rb");	

	char* buffer = malloc(sizeof(char));
	buffer[0] = 0;

	while (getline(&line, &len, fp) != -1) {
		buffer = format("{2s}", buffer, line);
	}

	fclose(fp);
	if (line)
		free(line);

	return buffer;
}

void write_file(const char * filename, char * write_buffer) {
	FILE * fp;
	
	fp = open_file(filename, "wb");

	if (!fputs(write_buffer, fp)) {
		println("Error: Unable to write buffer to file: '{s}'", filename);
		exit(1);
	}
	fclose(fp);
}
