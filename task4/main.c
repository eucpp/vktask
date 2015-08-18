#include <stdio.h>
#include <assert.h>

int bsearch(const int* a, size_t n, int x)
{
    const int* fst = a;
    const int* lst = a + n;
    const int* it  = fst;
    while (fst < lst) {
        size_t mid = (lst - fst) / 2;
        it = fst + mid;
        if (*it <= x) {
            fst = ++it;
        } else {
            lst = it;
        }
    }
    if (it == a + n) {
        return -1;
    }
    return it - a;
}

int main(void)
{
    int a1[] = {0, 1, 2, 3, 4};
    size_t n1 = sizeof(a1) / sizeof(a1[0]);
    assert(bsearch(a1, n1, -1) == 0);
    assert(bsearch(a1, n1, 0) == 1);
    assert(bsearch(a1, n1, 3) == 4);
    assert(bsearch(a1, n1, 4) == -1);

    int a2[] = {0, 0, 1, 1};
    size_t n2 = sizeof(a2) / sizeof(a2[0]);
    assert(bsearch(a2, n2, 0) == 2);
    assert(bsearch(a2, n2, 1) == -1);

    int a3[] = {1};
    size_t n3 = sizeof(a3) / sizeof(a3[0]);
    assert(bsearch(a3, n3, -1) == 0);
    assert(bsearch(a3, n3, 5) == -1);

    int a4[] = {-1, 1, 3, 5};
    size_t n4 = sizeof(a4) / sizeof(a4[0]);
    assert(bsearch(a4, n4, 2) == 2);

    return 0;
}

