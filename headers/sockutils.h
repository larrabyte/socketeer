#pragma once

#include <stdlib.h>
#include <stdio.h>

struct fileattr {
    size_t size;
    char *data;
};

// Prints message to standard error and exits with error.
void exitsock(const char *message, int error) {
    fprintf(stderr, message);
    if(error != 0) fprintf(stderr, "Error code: %d\n", error);
    exit(-1);
}

// Performs malloc/realloc while also checking the resulting pointers.
void *safealloc(void *memory, size_t size) {
    if(memory == NULL) memory = malloc(size);
    else memory = realloc(memory, size);
    if(memory == NULL) exitsock("Socketeer encountered a memory issue.\n", 0);

    return memory;
}

// Returns a struct with file size and pointer to data.
struct fileattr readfile(char *filepath) {
    FILE *fstream = fopen(filepath, "rb");
    if(fstream == NULL) return (struct fileattr) {0, NULL};

    struct fileattr file;
    fseek(fstream, 0L, SEEK_END);
    file.size = ftell(fstream);
    fseek(fstream, 0L, 0);

    file.data = (char*) safealloc(NULL, file.size);
    fread(file.data, 1, file.size, fstream);
    fclose(fstream);
    return file;
}