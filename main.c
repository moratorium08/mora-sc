#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "linklist.h"
#include "ast.h"

Variable *envs;
int main(void) {
    char *s = "(+ 1 2)";

    return 0;
}

void parser(AST *ast, char *code) {
    int n = strlen(code);
    int i = 0;
    for (; i < n; i++) {
        char c = code[i];
        char *cur = "";
        char *tmp;
        int cur_n = strlen(cur);
        LinkListString *linklist;

        switch(c) {
            case '(':
                // nop
                break;
            case ')':
                break;
            case ' ':
                append_link_string(&linklist, cur);
                cur = "";
                break;
            default:
                tmp = (char *)malloc(cur_n + 1 + 1);
                strcpy(tmp, cur);
                tmp[cur_n] = c;
                tmp[cur_n] = '\x00';
                free(cur);
                cur = tmp;
                break;
        }
    }
}

void run(char *line) {
	AST *ast;
    parser(ast, line);
}
