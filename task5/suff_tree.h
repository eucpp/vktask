#ifndef SUFF_TREE_H
#define SUFF_TREE_H

#include <stdint.h>

// NOTE: we use int32_t everywhere because this type is large enough to index 128 Mb == 2^27 byte.

typedef struct st_node
{
    int32_t     m_left;
    int32_t     m_right;
    int32_t     m_parent;
    int32_t     m_suflink;
    int32_t*    m_children;
} st_node;

typedef struct st_state
{
    int32_t     m_node;
    int32_t     m_pos;
} st_state;

typedef struct st_tree
{
    st_node*    m_nodes;
    int32_t     m_size;
    int32_t     m_capacity;
} st_tree;


st_tree* st_create_tree(const char* str, int32_t len);
void st_destroy_tree(st_tree* tree);


#endif // SUFF_TREE_H
