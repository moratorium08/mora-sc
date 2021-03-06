#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "vector.h"
#include "pair.h"

int get_function_id() {
    return GLOBAL_CURRENT_ID++;
}

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

Ast *make_if_ast() {
    Ast *ast;
    ast = malloc(sizeof(Ast));
    memset(ast, 0, sizeof(Ast));
    ast->type = IF_AST;
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

Ast *make_ast_from_variable(Variable *v) {
    Ast *ast;
    ast = malloc(sizeof(Ast));
    ast->type = VARIABLE_AST;
    ast->val = v;
    return ast;
}

Constant *make_int_constant(int x) {
    Constant *cnt = malloc(sizeof(Constant));
    cnt->type = INTEGER_TYPE_CONST;
    cnt->integer_cnt = x;
    return cnt;
}

Constant *make_boolean_constant(int b) {
    Constant *cnt = malloc(sizeof(Constant));
    cnt->type = BOOLEAN_TYPE_CONST;
    cnt->bool_cnt = b;
    return cnt;
}

Constant *make_func_constant_primitive(SchemeFunc f, int argc) {
    Constant *cnt = malloc(sizeof(Constant));
    Function *func = malloc(sizeof(Function));
    func->env = make_vector(0);
    func->type = PRIMITIVE_FUNCTION;
    func->id = get_function_id();
    func->argc = argc;
    func->func = f;
    cnt->type = FUNCTION_TYPE_CONST;
    cnt->func = func;
    return cnt;
}

Constant *make_pair_constant(Constant *fst, Constant *snd) {
    Constant *cnt = malloc(sizeof(Constant));
    Pair *p = make_pair(fst, snd);
    cnt->type = PAIR_TYPE_CONST;
    cnt->pair = p;
    return cnt;
}

Constant *make_symbol_constant(char *symbol) {
    Constant *cnt = malloc(sizeof(Constant));
    cnt->type = SYMBOL_TYPE_CONST;
    cnt->symbol = symbol;
    return cnt;
}

Constant *_base_list_instance = NULL;
Constant *get_list_base_instance() {
    if (_base_list_instance == NULL) {
        _base_list_instance = malloc(sizeof(Constant));
        _base_list_instance->type = SPECIAL_TYPE_CONST;
        _base_list_instance->special = LIST_TYPE_SPECIAL;
    }
    return _base_list_instance;
}

Ast *make_boolean_ast(int b) {
    Ast *ast;
    ast = malloc(sizeof(Ast));
    ast->type = CONSTANT_AST;
    ast->cnt = make_boolean_constant(b);
    return ast;
}

Ast *make_int_ast(int x) {
    Ast *ast;
    ast = malloc(sizeof(Ast));
    ast->type = CONSTANT_AST;
    ast->cnt = make_int_constant(x);
    return ast;
}
Ast *make_constant_ast(Constant *c) {
    Ast *ast;
    ast = malloc(sizeof(Ast));
    ast->type = CONSTANT_AST;
    ast->cnt = c;
    return ast;
}

int is_list(Constant *c) {
    while (c->type == PAIR_TYPE_CONST) {
        c = c->pair->snd;
    }
    return c->type == SPECIAL_TYPE_CONST && c->special == LIST_TYPE_SPECIAL;
}

void _print_pair(Constant *c) {
    printf("(");
    if (is_list(c)) {
        while (c->type == PAIR_TYPE_CONST) {
            print_constant(c->pair->fst);
            c = c->pair->snd;
            if (c->type == PAIR_TYPE_CONST) {
                printf(" ");
            }
        }
    } else {
        Constant *c1, *c2;
        c1 = c->pair->fst;
        c2 = c->pair->snd;
        print_constant(c1);
        printf(" . ");
        print_constant(c2);
    }
    printf(")");
}

void print_constant(Constant *c) {
    switch (c->type) {
        case INTEGER_TYPE_CONST:
            printf("%d", c->integer_cnt);
            break;
        case BOOLEAN_TYPE_CONST:
            printf("#%c", c->bool_cnt == 1 ? 't' : 'f');
            break;
        case SYMBOL_TYPE_CONST:
            printf("%s", c->symbol);
            break;
        case FUNCTION_TYPE_CONST:
            printf("<function %p>", c->func);
            break;
        case TAIL_TYPE_CONST:
            printf("<tail %p>", c->items);
            break;
        case PAIR_TYPE_CONST:
            _print_pair(c);
            break;
        case SPECIAL_TYPE_CONST:
            printf("()");
            break;
    }
}

Function *make_constructive_function(Ast *ast, Vector* args, Vector *env) {
    Function *func = malloc(sizeof(Function));
    func->type = CONSTRUCTIVE_FUNCTION;
    func->id = get_function_id();
    func->env = copy_vector(env);
    func->argc = args->len;
    func->ast = ast;
    func->names = args;
    return func;
}


Constant *make_func_constant(Function *f) {
    Constant *c = malloc(sizeof(Constant));
    c->type = FUNCTION_TYPE_CONST;
    c->func = f;
    return c;
}

// Vector<Ast *>
Constant *make_tail_constant(Vector *args) {
    Constant *c = malloc(sizeof(Constant));
    c->type = TAIL_TYPE_CONST;
    c->items = args;
    return c;
}

// items ex: (lambda (x y) (+ x y))
Constant *make_lambda_constant(Ast *ast, Vector *args, Vector *env) {
    Constant *c = malloc(sizeof(Constant));
    c->type = FUNCTION_TYPE_CONST;
    Function *func = malloc(sizeof(Function));
    func->type = LAMBDA_FUNCTION;
    func->id = get_function_id();
    func->env = copy_vector(env);
    func->argc = args->len;
    func->lam_ast = ast;
    func->names = args;
    c->func = func;
    return c;
}

void print_spaces(int x) {
    int i = 0;
    for (;i < x; i++) {
        printf(" ");
    }
}

void print_ast(Ast *ast, int indent) {
    int i;
    Variable *v;
    switch(ast->type) {
        case APPLY_AST:
            print_spaces(indent);
            printf("(\n");
            for(i = 0; i < ast->ap->asts->len; i++) {
                print_ast(vector_get(ast->ap->asts, i), indent + 1);
            }
            print_spaces(indent);
            printf(")\n");
            break;
        case VARIABLE_AST:
            v = ast->val;
            print_spaces(indent);
            printf("Val:%s: %d\n", ast->val->identifier, ast->val->type);
            if (v->type == FUNCTION_TYPE_VARIABLE) {
                print_ast(v->func->ast, indent + 1);
            }
            break;
        case CONSTANT_AST:
            print_spaces(indent);
            print_constant(ast->cnt);
            printf("\n");
            break;
        case DEFINE_AST:
            print_spaces(indent);
            printf("[define] \n");
            break;
        case IF_AST:
            print_spaces(indent);
            printf("[if] \n");
            break;
    }
}

