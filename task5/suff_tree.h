#ifndef SUFF_TREE_H
#define SUFF_TREE_H

#include <stdint.h>
#include <stdbool.h>

// NOTE: we use int32_t everywhere because this type is large enough to index 128 Mb == 2^27 byte.

typedef struct st_pair
{
    char        m_key;
    int32_t     m_value;
} st_pair;

typedef struct st_array
{
    st_pair*    m_array;
    int16_t     m_size;
    int16_t     m_capacity;
} st_array;

typedef struct st_node
{
    int32_t     m_left;
    int32_t     m_right;
    int32_t     m_parent;
    int32_t     m_suflink;
    st_array    m_children;
} st_node;

typedef struct st_state
{
    int32_t     m_node;
    int32_t     m_pos;
} st_state;

typedef struct st_tree
{
    const char* m_str;
    int32_t     m_strlen;
    st_node*    m_nodes;
    int32_t     m_size;
} st_tree;

st_tree* st_create_tree(const char* str, int32_t len);
void st_destroy_tree(st_tree* tree);

bool st_contains(st_tree* tree, const char* needle, int32_t len);

#endif // SUFF_TREE_H
