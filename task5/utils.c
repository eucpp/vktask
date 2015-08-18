#include "utils.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

void read_file(const char* filename, char** content, int32_t* size, int* error)
{
    // to be sure
    assert(sizeof(char) == 1);

    FILE* file = fopen(filename, "rb");
    fseek(file, 0L, SEEK_END);
    *size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    *content = (char*) malloc(*size);
    int32_t cnt = fread((void*) *content, 1, *size, file);
    if (cnt != *size) {
        *error = ferror(file);
    } else {
        *error = 0;
    }
}

stream_handle* create_stream(FILE* stream)
{
    stream_handle* handle = (stream_handle*) malloc(sizeof(stream_handle));
    handle->m_stream      = stream;
    handle->m_endflag     = false;
    return handle;
}

void destroy_stream(stream_handle* handle)
{
    free((void*) handle);
}

int32_t get_next_string(stream_handle* handle, char* str, int32_t n, int* error)
{
    if (handle->m_endflag) {
        return -1;
    }
    char* res = fgets(str, n, handle->m_stream);
    if (res == NULL) {
        *error = ferror(handle->m_stream);
        handle->m_endflag = true;
        return -1;
    }
    char* pch = strpbrk(str, " \n\0");
    int32_t len = pch - str;
    int32_t mn = (len < 4) ? len : 4;
    if (strncmp(str, "exit", mn) == 0) {
        handle->m_endflag = true;
    }
    return len;
}
