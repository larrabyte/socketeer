#pragma once

#include <stddef.h>

struct fileattr {  // File attribute struct: contains size and pointer to data. 
    size_t size;   // Number of bytes stored by *data.
    char *data;    // Actual data pointer.
};

struct fileattr readfile(const char *filepath);
void exitsock(const char *message, int error);
void *safealloc(void *memory, size_t size);
