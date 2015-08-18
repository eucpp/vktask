#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct stream_handle
{
    FILE*   m_stream;
    bool    m_endflag;
} stream_handle;

void read_file(const char* filename, char** content, int32_t* size, int* error);

stream_handle* create_stream(FILE* stream);
void destroy_stream(stream_handle* handle);
int32_t get_next_string(stream_handle* handle, char* str, int32_t n, int* error);

#endif // UTILS_H
