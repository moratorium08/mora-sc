#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "vector.h"

Application *make_application() {
    Application *ap = malloc(sizeof(Application));
    ap->asts = make_vector(5);
    return ap;
}

Ast *make_apply_ast() {
    Ast *ast;
    ast = malloc(sizeof(Ast));
    ast->type = APPLY_AST;
    ast->ap = make_application();
    return ast;
}

Variable *make_variable(VariableType t, char *id) {
    Variable *val = malloc(sizeof(Variable));
    memset(val, 0, sizeof(Variable));
    val->identifier = id;
    val->type = t;
    val->defined = 0;
    return val;
}

Ast *make_variable_ast(VariableType t, char *id) {
    Ast *ast;
    ast = malloc(sizeof(Ast));
    ast->type = VARIABLE_AST;
    ast->val = make_variable(t, id);
    return ast;
}

