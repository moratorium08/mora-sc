#include <stdio.h>
#include "../pair.h"
#include "test.h"

int main() {
    describe("Pair unit test");
    int x = 10;
    int y = 15;
    Pair *p = make_pair(&x, &y);
    int *z = p->fst;
    int *w = p->snd;
    expect_int(*z, x);
    expect_int(*w, y);
    summary();
}
