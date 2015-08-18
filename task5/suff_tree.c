#include "suff_tree.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static const char START_ASCII_CODE  = 32;
static const char END_ASCII_CODE    = 127;

int32_t st_suflen(st_node* node)
{
    return node->m_right - node->m_left;
}

int16_t st_get_children_ind(st_node* node, char ch)
{
    for (int16_t i = 0; i < node->m_children.m_size; ++i) {
        st_pair* kv = &node->m_children.m_array[i];
        if (ch == kv->m_key) {
            return i;
        }
    }
    return -1;
}

int32_t st_get_children(st_node* node, char ch)
{
    int16_t ind = st_get_children_ind(node, ch);
    if (ind == -1) {
        return -1;
    }
    return node->m_children.m_array[ind].m_value;
}

void st_set_children(st_node* node, char ch, int32_t v)
{
    int16_t ind = st_get_children_ind(node, ch);
    if (ind != -1) {
        node->m_children.m_array[ind].m_value = v;
        return;
    }
    int16_t size = node->m_children.m_size;
    if (size == node->m_children.m_capacity) {
        int16_t cap = size > 0 ? 2 * size : 1;
        st_pair* mem = (st_pair*) malloc(cap * sizeof(st_pair));
        memcpy((void*) mem, (void*) node->m_children.m_array, size * sizeof(st_pair));
        free(node->m_children.m_array);
        node->m_children.m_array    = mem;
        node->m_children.m_capacity = cap;
    }
    node->m_children.m_array[size].m_key    = ch;
    node->m_children.m_array[size].m_value  = v;
    ++node->m_children.m_size;
}

void st_init_node(st_node* node, int32_t left, int32_t right, int32_t parent)
{
    node->m_left        = left;
    node->m_right       = right;
    node->m_parent      = parent;
    node->m_suflink     = -1;

    node->m_children.m_array    = NULL;
    node->m_children.m_size     = 0;
    node->m_children.m_capacity = 0;
}

// append new node to the suffix tree
int32_t st_append_node(st_tree* tree, int32_t left, int32_t right, int32_t parent)
{
    int32_t ind = tree->m_size++;
    st_init_node(&tree->m_nodes[ind], left, right, parent);
    return ind;
}

st_state st_edge_transition(const char* str, st_tree* tree, st_state state, int32_t pos, int32_t len)
{
    if (str == tree->m_str) {
        state.m_pos += len;
        return state;
    }
    int32_t offst = tree->m_nodes[state.m_node].m_left;
    for (int32_t i = 0; i < len; ++i) {
        if (str[pos++] != tree->m_str[offst + state.m_pos++]) {
            state.m_node = -1;
            state.m_pos  = -1;
            return state;
        }
    }
    return state;
}

// perform a transition on suffix tree
// starting position in tree specified by state
// transitional substring specified by indexes left and right in string str
st_state st_transition(const char* str, st_tree* tree, st_state state, int32_t left, int32_t right)
{
    while (left < right) {
        st_node* node = &tree->m_nodes[state.m_node];
        int32_t nodelen = st_suflen(node);
        if (state.m_pos == nodelen) {
            // transition to next node
            char ch = str[left];
            int32_t nextni = st_get_children(node, ch);
            if (nextni == -1) {
                state.m_node = -1;
                state.m_pos  = -1;
                return  state;
            } else {
                state.m_node = nextni;
                state.m_pos  = 0;
            }
        } else {
            // transition within current vertex
            int32_t pos = node->m_left + state.m_pos;
            int32_t trlen = right - left;
            int32_t suflen = nodelen - state.m_pos;
            if (tree->m_str[pos] != str[left]) {
                state.m_node = -1;
                state.m_pos  = -1;
                return state;
            } else if (trlen < suflen) {
                return st_edge_transition(str, tree, state, left, trlen);
            } else {
                state = st_edge_transition(str, tree, state, left, suflen);
                left += suflen;
                if (state.m_node == -1) {
                    return state;
                }
            }
        }
    }
    return state;
}

// split edge specified by state in suffix tree
int32_t st_split(st_tree* tree, st_state state)
{
    st_node* node = &tree->m_nodes[state.m_node];
    st_node* parent = &tree->m_nodes[node->m_parent];
    // trivial cases
    if (state.m_pos == st_suflen(node)) {
        return state.m_node;
    } else if (state.m_pos == 0) {
        return node->m_parent;
    }
    // insert new node between current node and its parent
    int32_t piv = node->m_left + state.m_pos;
    char lch = tree->m_str[node->m_left];
    char pch = tree->m_str[piv];
    int32_t newni = st_append_node(tree, node->m_left, piv, node->m_parent);
    st_node* new_node = &tree->m_nodes[newni];

    st_set_children(parent, lch, newni);
    st_set_children(new_node, pch, state.m_node);
    node->m_parent = newni;
    node->m_left = piv;

    return newni;
}

// lazy initialize and return suffix link of node in suffix tree
int32_t st_suflink(st_tree* tree, st_node* node)
{
    if (node->m_suflink != -1) {
        return node->m_suflink;
    }
    if (node->m_parent == -1) {
        return 0;
    }

    int32_t plink = st_suflink(tree, &tree->m_nodes[node->m_parent]);
    st_state state;
    state.m_node = plink;
    state.m_pos  = st_suflen(&tree->m_nodes[plink]);

    int32_t left = node->m_left;
    int32_t right = node->m_right;
    if (node->m_parent == 0) {
        ++left;
    }
    st_state tr = st_transition(tree->m_str, tree, state, left, right);
    node->m_suflink = st_split(tree, tr);
    return node->m_suflink;
}

// extend suffix tree corresponded to str from [0 ... pos] to [0 ... pos+1]
st_state st_extend_tree(st_tree* tree, st_state state, int32_t pos)
{
    while(true) {
        st_state newst = st_transition(tree->m_str, tree, state, pos, pos + 1);
        if (newst.m_node != -1) {
            return newst;
        }
        int32_t midni = st_split(tree, state);
        int32_t newni = st_append_node(tree, pos, tree->m_strlen, midni);
        st_node* mid_node = &tree->m_nodes[midni];
        st_set_children(mid_node, tree->m_str[pos], newni);

        state.m_node = st_suflink(tree, mid_node);
        state.m_pos = st_suflen(&tree->m_nodes[state.m_node]);
        if (midni == 0) {
            return state;
        }
    }
}

st_tree* st_create_tree(const char* str, int32_t len)
{
    st_tree* tree = (st_tree*) malloc(sizeof(st_tree));
    tree->m_str   = str;
    tree->m_strlen= len;
    tree->m_nodes = (st_node*) malloc(sizeof(st_node) * (2 * len + 1));
    tree->m_size  = 1;
    // init root
    st_init_node(&tree->m_nodes[0], 0, 0, -1);
    // build suffix tree
    st_state state;
    state.m_node = 0;
    state.m_pos  = 0;
    for (int32_t i = 0; i < len; ++i) {
        state = st_extend_tree(tree, state, i);
    }

    return tree;
}

void st_destroy_tree(st_tree* tree)
{
    for (int16_t i = 0; i < tree->m_size; ++i) {
        free(tree->m_nodes[i].m_children.m_array);
    }
    free((void*) tree->m_nodes);
    free((void*) tree);
}


bool st_contains(st_tree* tree, const char* needle, int32_t len)
{
    st_state state;
    state.m_node = 0;
    state.m_pos  = 0;
    state = st_transition(needle, tree, state, 0, len);
    if (state.m_node == -1) {
        return false;
    }
    return true;
}
