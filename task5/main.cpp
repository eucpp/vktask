#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cassert>

//#include "suff_tree.h"

using namespace std;

typedef struct st_list
{
    char        m_key;
    int32_t     m_value;
    st_list*    m_tail;
} st_list;

typedef struct st_node
{
    int32_t     m_left;
    int32_t     m_right;
    int32_t     m_parent;
    int32_t     m_suflink;
    st_list*    m_children;
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

static const char START_ASCII_CODE  = 32;
static const char END_ASCII_CODE    = 127;

int32_t st_suflen(st_node* node)
{
    return node->m_right - node->m_left;
}

int32_t st_get_children(st_node* node, char ch)
{
    st_list* head = node->m_children;
    while (head != NULL) {
        if (ch == head->m_key) {
            return head->m_value;
        }
        head = head->m_tail;
    }
    return -1;
}

void st_set_children(st_node* node, char ch, int32_t v)
{
    st_list* head    = (st_list*) malloc(sizeof(st_list));
    head->m_key      = ch;
    head->m_value    = v;
    head->m_tail     = node->m_children;
    node->m_children = head;
}

void st_init_node(st_node* node, int32_t left, int32_t right, int32_t parent)
{
    node->m_left        = left;
    node->m_right       = right;
    node->m_parent      = parent;
    node->m_suflink     = -1;
    node->m_children    = NULL;
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
    for (int32_t i = 0; i < tree->m_size; ++i) {
        st_list* head = tree->m_nodes[i].m_children;
        while (head != NULL) {
            st_list* next = head->m_tail;
            free(head);
            head = next;
        }
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

int main(void)
{
    ios_base::sync_with_stdio(0);

    ifstream in;
    in.open("dictionary.in");
    ofstream out;
    out.open("dictionary.out");

    assert(in.is_open());
    assert(out.is_open());

    string str;
    in >> str;

    st_tree* tree = st_create_tree(str.c_str(), str.length());

    int n;
    in >> n;
    for (int i = 0; i < n; ++i) {
        string needle;
        in >> needle;
        bool b = st_contains(tree, needle.c_str(), needle.length());
        if (b) {
            out << "Yes" << endl;
        } else {
            out << "No" << endl;
        }
    }

    st_destroy_tree(tree);

    return 0;
}


