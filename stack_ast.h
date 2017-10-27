#include "ast.h"
typedef struct lnode {
    Ast* ast;
    struct lnode *next;
} AstStack;

Ast* pop_ast_stack(AstStack **list);
void Push(AstStack **list, Ast *new_ast);
