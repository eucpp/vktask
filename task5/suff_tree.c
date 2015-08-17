#include "suff_tree.h"

#include <stdlib.h>
#include <string.h>

static const char START_ASCII_CODE  = 32;
static const char END_ASCII_CODE    = 127;

int32_t st_suflen(st_node* node)
{
    return node->m_right - node->m_left;
}

int32_t st_get_children(st_node* node, char ch)
{
    ch -= START_ASCII_CODE;
    return node->m_children[ch];
}

void st_set_children(st_node* node, char ch, int32_t v)
{
    ch -= START_ASCII_CODE;
    node->m_children[ch] = v;
}

void st_init_node(st_node* node, int32_t left, int32_t right, int32_t parent)
{
    node->m_left        = left;
    node->m_right       = right;
    node->m_parent      = parent;
    node->m_children    = (int32_t*) malloc(sizeof(int32_t) * (END_ASCII_CODE - START_ASCII_CODE));
    node->m_suflink     = -1;
}

st_tree* st_create_tree(int32_t size)
{
    st_tree* tree = (st_tree*) malloc(sizeof(st_tree));
    tree->m_nodes = (st_node*) malloc(2 * size * sizeof(st_node));
    tree->m_size  = 1;
    st_init_node(&tree->m_nodes[0], 0, 0, -1);
    return tree;
}

// append new node to the suffix tree
int32_t st_append_node(st_tree* tree, int32_t left, int32_t right, int32_t parent)
{
//    if (tree->m_size == tree->m_capacity) {
//        st_node* newmem = (st_node*) malloc(2 * tree->m_size * sizeof(st_node));
//        memcpy((void*) newmem, (void*) tree->m_nodes, tree->m_size * sizeof(st_node));
//        free(tree->m_nodes);
//        tree->m_nodes = newmem;
//    }
    int32_t ind = tree->m_size++;
    st_init_node(&tree->m_nodes[ind], left, right, parent);
    return ind;
}

// perform a transition on suffix tree
// starting position in tree specified by state
// transitional substring specified by indexes left and right in string str
st_state st_transition(const char* str, st_tree* tree, st_state state, int32_t left, int32_t right)
{
    while (left < right) {
        st_node* node = &tree->m_nodes[state.m_node];
        int32_t sufflen = st_suflen(node);
        // transition within current vertex
        if (state.m_pos != sufflen) {
            int32_t pos = node->m_left + state.m_pos;
            int32_t trlen = right - left;
            if (str[pos] != str[left]) {
                state.m_node = -1;
                state.m_pos  = -1;
                return state;
            } else if (trlen < sufflen - state.m_pos) {
                state.m_pos += trlen;
                return state;
            }
            left += sufflen - state.m_pos;
            state.m_pos = sufflen;
        }
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
    }
    return state;
}

// split edge specified by state in suffix tree
int32_t st_split(const char* str, st_tree* tree, st_state state)
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
    char lch = str[node->m_left];
    char pch = str[piv];
    int32_t newni = st_append_node(tree, node->m_left, piv, node->m_parent);
    st_node* new_node = &tree->m_nodes[newni];

    st_set_children(parent, lch, newni);
    st_set_children(new_node, pch, node);
    node->m_parent = newni;
    node->m_left = piv;

    return newni;
}

// lazy initialize and return suffix link of node in suffix tree
int32_t st_suflink(const char* str, st_tree* tree, st_node* node)
{
    if (node->m_suflink != -1) {
        return node->m_suflink;
    }
    if (node->m_parent == 0) {
        return 0;
    }

    int32_t plink = st_suflink(str, tree, &tree->m_nodes[node->m_parent]);
    st_state state;
    state.m_node = plink;
    state.m_pos = st_suflen(&tree->m_nodes[plink]);

    int32_t left = node->m_left;
    int32_t right = node->m_right;
    if (node->m_parent == 0) {
        ++left;
    }
    state tr = st_transition(str, tree, state, left, right);
    node->m_suflink = st_split(str, tree, tr);
    return node->m_suflink;
}

// extend suffix tree corresponded to str from [0 ... pos] to [0 ... pos+1]
state st_extend_tree(const char* str, int32_t len, st_tree* tree, st_state state, int32_t pos)
{
    while(true) {
        st_state newst = st_transition(str, tree, state, pos, pos + 1);
        if (newst.m_node != -1) {
            return newst;
        }
        int32_t midni = st_split(str, tree, state);
        int32_t newni = st_append_node(tree, pos, len, midni);
        st_node* mid_node = &tree->m_nodes[midni];
        st_set_children(mid_node, str[pos], newni);

        state.m_node = st_suflink(str, tree, mid_node);
        state.m_pos = st_suflen(&tree->m_nodes[state.m_node]);
        if (midni == 0) {
            return state;
        }
    }
}
