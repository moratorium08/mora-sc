#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack_ast.h"


Ast* pop_ast_stack(AstStack **list) {
    AstStack *st = *list;
    *list = st->next;
    Ast *ret = st->ast;
    free(st);
    return ret;
}

void Push(AstStack **list, Ast *new_ast) {
    AstStack *old_top = (AstStack *)malloc(sizeof(AstStack));
    memset(old_top, 0, sizeof(AstStack));
    old_top->ast = (*list)->ast;
    old_top->next = (*list)->next;
    (*list)->ast = new_ast;
    (*list)->next = old_top;
}

