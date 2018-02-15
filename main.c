#include <stdio.h>
#include "core.h"
#include "ast.h"

const int BUF_SIZE = 1024;

void prompt() {
    int i;
    char s[BUF_SIZE];
    printf("> ");
    fgets(s, BUF_SIZE, stdin);
    for (i = 0; s[i]; i++) {
        if (s[i] == '\n'){
            s[i] = '\x00';
            break;
        }
    }
    Constant *c = run(s);
    if (c != NULL) {
        print_constant(c);
        puts("");
    }
}

int main(void) {
    setup();
    while (1) {
        prompt();
    }
}
