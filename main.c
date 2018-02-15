#include <stdio.h>
#include "core.h"
#include "ast.h"

int main(void) {
    setup();
    Constant *c;
    c = run("(+ 1 2)");
    print_constant(c);
    puts("");
}
