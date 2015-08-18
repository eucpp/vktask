#include <stdio.h>
#include <stdint.h>

#include "suff_tree.h"
#include "utils.h"


int main(int argc, char** argv)
{
    if (argc < 2) {
        printf("%s", "Filename of dictionary should be passed as first argument !");
        return 0;
    }

    const char* fn  = argv[1];
    char* dict      = NULL;
    int32_t size    = 0;
    int error       = 0;

    read_file(fn, &dict, &size, &error);
    if (error != 0) {
        printf("Error reading dictionary %d", error);
        return 0;
    }
    st_tree* tree = st_create_tree(dict, size);

    stream_handle* stream = create_stream(stdin);
    const int32_t BUF_SIZE = 200;
    char buf[BUF_SIZE];
    while (!stream->m_endflag) {
        error = 0;
        int32_t len = get_next_string(stream, buf, BUF_SIZE, &error);
        if (stream->m_endflag) {
            break;
        }
        if (error != 0) {
            printf("Error reading stdin %d", error);
            return 0;
        }
        if (st_contains(tree, buf, len)) {
            puts("YES");
        } else {
            puts("NO");
        }
    }

    return 0;
}


