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

Ast *make_define_ast() {
    Ast *ast;
    ast = malloc(sizeof(Ast));
    memset(ast, 0, sizeof(Ast));
    ast->type = DEFINE_AST;
    return ast;
}

Ast *make_apply_ast() {
    Ast *ast;
    ast = malloc(sizeof(Ast));
    ast->type = APPLY_AST;
    ast->ap = make_application();
    return ast;
}

Variable *make_variable(char *id) {
    Variable *val = malloc(sizeof(Variable));
    memset(val, 0, sizeof(Variable));
    val->identifier = id;
    val->defined = 0;
    return val;
}

Ast *make_variable_ast(char *id) {
    Ast *ast;
    ast = malloc(sizeof(Ast));
    ast->type = VARIABLE_AST;
    ast->val = make_variable(id);
    return ast;
}

Constant *make_int_constant(int x) {
    Constant *cnt = malloc(sizeof(Constant));
    cnt->type = INTEGER_TYPE_CONST;
    cnt->integer_cnt = x;
    return cnt;
}


Constant *make_func_constant_primitive(SchemeFunc f, int argc) {
    Constant *cnt = malloc(sizeof(Constant));
    Function *func = malloc(sizeof(Function));
    func->type = PRIMITIVE_FUNCTION;
    func->argc = argc;
    func->func = f;
    cnt->type = FUNCTION_TYPE_CONST;
    cnt->func = func;
    return cnt;
}

Ast *make_int_ast(int x) {
    Ast *ast;
    ast = malloc(sizeof(Ast));
    ast->type = CONSTANT_AST;
    ast->cnt = make_int_constant(x);
    return ast;
}

void print_constant(Constant *c) {
    switch (c->type) {
        case INTEGER_TYPE_CONST:
            printf("%d", c->integer_cnt);
            break;
        case BOOLEAN_TYPE_CONST:
            printf("#%c", c->bool_cnt == 1 ? 't' : 'f');
            break;
        case FUNCTION_TYPE_CONST:
            printf("<function %p>", c->func);
            break;
    }
}

Function *make_constructive_function(Ast *ast, int argc) {
    Function *func = malloc(sizeof(Function));
    func->type = CONSTRUCTIVE_FUNCTION;
    func->argc = argc;
    func->ast = ast;
    return func;
}
