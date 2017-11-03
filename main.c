#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "linklist.h"
#include "ast.h"
#include "vector.h"
#include "map.h"
#include "util.h"

Ast* parser(char *code);
Constant* evaluate(Application *ap);
int start_scope(Vector *tuples);
void end_scope();

int ERROR_FLAG = 0;

void error(char *s) {
    puts(s);
    ERROR_FLAG = 1;
}
void reset_error() {
    ERROR_FLAG = 0;
}

Map *global_variables; // type: Map<Ast*>
Vector *local_variables; // type: Vector<Map<Ast*>>
void run(char *line);

void print_globals() {
    int i;
    int n = global_variables->len;
    for (i = 0; i < n; i++) {
        char *key = vector_get(global_variables->keys, i);
        printf("** %s **\n", key);
        print_ast(map_get(global_variables, key), 0);
    }


}

int main(void) {
    global_variables = make_map(100);
    local_variables = make_vector(100);
    //char *s = "(define x 3)";
    char *s = "(define x 2)";
    run(s);
    s = "(let ((x 3)) (quotient (* (+ x 1) 2) 3))";
    run(s);
    s = "(* x (let ((x 5)) x))";
    run(s);
    s = "(define (f x) (+ x 2))";
    run(s);
    s = "(f 5)";
    run(s);
    return 0;
}

char *cut_string(char *code, int st, int ed) {
    if (st >= ed) {
        error("st >= ed is invalid");
    }
    char *new_str = malloc(ed - st);
    memcpy(new_str, code + st, ed - st - 1);
    new_str[ed - st - 1] = '\x00';
    return new_str;
}

typedef enum {
    STR_TOKEN,
    OPEN_BRACKET_TOKEN,
    CLOSE_BRACKET_TOKEN,
} TokenType;


typedef struct {
    TokenType type;
    union {
        char *raw;
    };
} Token;

Token *create_str_token(char *raw) {
    Token *token = malloc(sizeof(Token));
    token->type = STR_TOKEN;
    char *cpy = malloc(strlen(raw) + 1);
    strcpy(cpy, raw);
    token->raw = cpy;
    return token;
}

Token *create_bracket_token(TokenType tt) {
    Token *token = malloc(sizeof(Token));
    token->type = tt;
    return token;
}

int is_number(char *s) {
    int n = strlen(s);
    int i;
    int flag = 1;
    for (i = 0; i < n; i++) {
        if (i == 0 && s[i] == '-') continue;
        switch (s[i]) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                continue;
            default:
                flag = 0;
                break;
        }
    }
    return flag;
}

Ast *handle_string_token(Token *token) {
    if (is_number(token->raw) == 1) {
        // TODO: オーバーフローを全く考慮していない
        // そもそもSchemeは規格として多倍長
        Ast *ast = make_int_ast(atoi(token->raw));
        return ast;
    }
    if (strcmp("define", token->raw) == 0) {
        Ast *ast = make_define_ast();
        return ast;
    }
    char *name = dup_str(token->raw);
    Ast *ast = make_variable_ast(token->raw);
    return ast;
}


int _dfs_application_from_token(Ast *ast, Vector *token_tree, int idx) {
    while (idx < token_tree->len) {
        Token *token = vector_get(token_tree, idx);
        Ast *child_ast;
        switch (token->type) {
            case OPEN_BRACKET_TOKEN:
                child_ast = make_apply_ast();
                idx = _dfs_application_from_token(child_ast, token_tree, idx + 1);
                vector_push(ast->ap->asts, child_ast);
                break;
            case CLOSE_BRACKET_TOKEN:
                return idx + 1;
            case STR_TOKEN:
                child_ast = handle_string_token(token);
                vector_push(ast->ap->asts, child_ast);
                idx++;
                break;
            default:
                error("oops. invalid token.");
                return -1;
        }
    }
    error("invalid syntax.");
    return -1;
}

Ast * create_ast_from_token_tree(Vector *token_tree) {
    if (token_tree->len == 0) {
        error("token tree size is zero.");
        return NULL;
    }
    Token *head = vector_get(token_tree, 0);
    if (head->type != OPEN_BRACKET_TOKEN) {
        error("invalid syntax.");
        return NULL;
    }
    Ast *ast = make_apply_ast();
    _dfs_application_from_token(ast, token_tree, 1);
    return ast;
}

Ast * parser(char *code) {
    int n = strlen(code);
    int i = 0;
    int st = 0;

    // 1. Build Parsed Token Tree
    // 2. Then Recursively create AST
    Vector *token_tree = make_vector(256);
    for (; i < n; i++) {
        char c = code[i];
        char *tmp;
        Token *token;

        switch(c) {
            case '(':
                token = create_bracket_token(OPEN_BRACKET_TOKEN);
                vector_push(token_tree, token);
                st = i+1;
                break;
            case ')':
                if (st < i) {
                    tmp = cut_string(code, st, i+1);
                    token = create_str_token(tmp);
                    vector_push(token_tree, token);
                }
                token = create_bracket_token(CLOSE_BRACKET_TOKEN);
                vector_push(token_tree, token);
                st = i+1;
                break;
            // 明らかに""とか'()とかキャッチできないので
            // TODO
            case ' ':
                if (st < i) {
                    tmp = cut_string(code, st, i+1);
                    token = create_str_token(tmp);
                    vector_push(token_tree, token);
                }
                st = i+1;
                break;
            default:
                break;
        }
    }
    return create_ast_from_token_tree(token_tree);
}

// this is so dirty...
Vector *_zip_vectors(Vector *names, Vector *items) {
    int n = names->len;
    int i;
    if (items->len - 1 != names->len) {
        error("invalid arguments");
        return NULL;
    }
    Vector *v = make_vector(names->len);
    for (i = 0; i < n; i++) {
        Ast *ast = make_apply_ast();
        Vector *tup = ast->ap->asts;

        Variable *val = vector_get(names, i);
        Ast *val_ast = make_ast_from_variable(val);
        Constant *item = vector_get(items, i + 1);
        Ast *item_ast = make_constant_ast(item);
        vector_push(tup, val_ast);
        vector_push(tup, item_ast);

        vector_push(v, ast);
    }
    return v;
}

// Vector <Constant *>
Constant *execute(Vector *items) {
    Constant *c = vector_get(items, 0);
    if (c->type != FUNCTION_TYPE_CONST) {
        error("non-functional value cannot be applied.");
        return NULL;
    }
    Function *f = c->func;
    if (f->argc != (items->len - 1)) {
        error("invalid arguments");
        return NULL;
    }
    if (f->type == PRIMITIVE_FUNCTION) {
        SchemeFunc raw_func = f->func;
        Constant *ret = (raw_func)(items);
        return ret;
    } else if (f->type == CONSTRUCTIVE_FUNCTION) {
        if (f->ast->type != APPLY_AST) {
            error("non-functionla value cannot be applied.");
            return NULL;
        }
        Vector *arg_names = f->names;
        int i;
        Vector *tuples = _zip_vectors(arg_names, items);

        start_scope(tuples);
        Constant *ret;
        ret = evaluate(f->ast->ap);
        end_scope();

        return ret;
    }
    error("not implmented");
    return NULL;
}

Constant *builtin_add(Vector *items) {
    if (items->len != 3) {
        error("+: invalid arguments.");
    }
    Constant *c1 = vector_get(items, 1);
    Constant *c2 = vector_get(items, 2);
    if (c1->type != INTEGER_TYPE_CONST || c2->type != INTEGER_TYPE_CONST) {
        error("+: arguments must have Integer Type");
    }
    return make_int_constant(c1->integer_cnt + c2->integer_cnt);
}
Constant *builtin_sub(Vector *items) {
    if (items->len != 3) {
        error("-: invalid arguments.");
        return NULL;
    }
    Constant *c1 = vector_get(items, 1);
    Constant *c2 = vector_get(items, 2);
    if (c1->type != INTEGER_TYPE_CONST || c2->type != INTEGER_TYPE_CONST) {
        error("-: arguments must have Integer Type");
        return NULL;
    }
    return make_int_constant(c1->integer_cnt - c2->integer_cnt);
}
Constant *builtin_mul(Vector *items) {
    if (items->len != 3) {
        error("*: invalid arguments.");
    }
    Constant *c1 = vector_get(items, 1);
    Constant *c2 = vector_get(items, 2);
    if (c1->type != INTEGER_TYPE_CONST || c2->type != INTEGER_TYPE_CONST) {
        error("*: arguments must have Integer Type");
    }
    return make_int_constant(c1->integer_cnt * c2->integer_cnt);
}
Constant *builtin_quotient(Vector *items) {
    if (items->len != 3) {
        error("quotient: invalid arguments.");
    }
    Constant *c1 = vector_get(items, 1);
    Constant *c2 = vector_get(items, 2);
    if (c1->type != INTEGER_TYPE_CONST || c2->type != INTEGER_TYPE_CONST) {
        error("quotient: arguments must have Integer Type");
    }
    return make_int_constant(c1->integer_cnt / c2->integer_cnt);
}

Constant *lookup_variable(Variable *v) {
    // lookup let scopes
    int i = local_variables->len - 1;
    for (; i >= 0; i--) {
        Map *m = vector_get(local_variables, i);
        Ast *ast = map_get(m, v->identifier);
        if (ast != NULL) {
            switch (ast->type) {
                case APPLY_AST:
                    return evaluate(ast->ap);
                case CONSTANT_AST:
                    return ast->cnt;
                case VARIABLE_AST:
                    if (ast->val->type != FUNCTION_TYPE_VARIABLE) {
                        error("not implemented");
                        return NULL;
                    }
                    return make_func_constant(ast->val->func);
                case DEFINE_AST:
                    error("not implemented");
                    return NULL;
            }
        }
    }
    // global variable
    Ast *gv = map_get(global_variables, v->identifier);
    if (gv != NULL) {
        switch (gv->type) {
            case APPLY_AST:
                return evaluate(gv->ap);
            case CONSTANT_AST:
                return gv->cnt;
            case VARIABLE_AST:
                if (gv->val->type != FUNCTION_TYPE_VARIABLE) {
                    error("not implemented");
                    return NULL;
                }
                return make_func_constant(gv->val->func);
            case DEFINE_AST:
                error("not implemented");
                return NULL;
        }
    }

    if (strcmp(v->identifier, "+") == 0) {
        return make_func_constant_primitive(&builtin_add, 2);
    }
    if (strcmp(v->identifier, "-") == 0) {
        return make_func_constant_primitive(&builtin_sub, 2);
    }
    if (strcmp(v->identifier, "*") == 0) {
        return make_func_constant_primitive(&builtin_mul, 2);
    }
    if (strcmp(v->identifier, "quotient") == 0) {
        return make_func_constant_primitive(&builtin_quotient, 2);
    }
    else {
        error("Ooops");
        return NULL;
    }
}

// tuples: Vector<Ast *>
// This Ast must have APPLY_AST type
int start_scope(Vector *tuples) {
    Map *new_scope = make_map(tuples->len);
    int i;
    for (i = 0; i < tuples->len; i++) {
        Ast *tuple = vector_get(tuples, i);
        if (tuple->type != APPLY_AST || tuple->ap->asts->len != 2) {
            error("illegal let");
            return -1;
        }
        Ast *name = vector_get(tuple->ap->asts, 0);
        Ast *val = vector_get(tuple->ap->asts, 1);
        if (name->type != VARIABLE_AST) {
            error("illegal let");
            return -1;
        }

        Constant *c;
        switch(val->type) {
            case CONSTANT_AST:
                c = val->cnt;
                break;
            case VARIABLE_AST:
                c = lookup_variable(val->val);
                break;
            case APPLY_AST:
                c = evaluate(val->ap);
                break;
            case DEFINE_AST:
                error("illegal let");
                return -1;
        }
        map_set(new_scope, name->val->identifier, make_constant_ast(c));
    }
    vector_push(local_variables, new_scope);
    return 1;
}
void end_scope() {
    vector_pop(local_variables);
}

Constant* evaluate(Application *ap) {
    int len = ap->asts->len;
    int i;
    Ast *top = vector_get(ap->asts, 0);
    if (top->type == VARIABLE_AST && strcmp(top->val->identifier, "let") == 0) {
        // handling let
        if (ap->asts->len < 2) {
            error("let must have not less than 2 args");
            return NULL;
        }
        Ast *tuple_ast = vector_get(ap->asts, 1);
        if (tuple_ast->type != APPLY_AST) {
            error("illegal let");
            return NULL;
        }
        Vector *tuples = tuple_ast->ap->asts;

        start_scope(tuples);
        Constant *ret;
        for (i = 2; i < ap->asts->len; i++) {
            Ast *ast = vector_get(ap->asts, i);
            switch (ast->type) {
                case VARIABLE_AST:
                    ret = lookup_variable(ast->val);
                    break;
                case CONSTANT_AST:
                    ret = ast->cnt;
                    break;
                case APPLY_AST:
                    ret = evaluate(ast->ap);
                    break;
                case DEFINE_AST:
                    error("define cannot be in such place");
                    return NULL;
            }
        }
        end_scope();
        return ret;
    }

    Vector *items = make_vector(8);
    for (i = 0; i < len; i++) {
        Ast *ast = vector_get(ap->asts, i);
        switch (ast->type) {
            case VARIABLE_AST:
                // TODO: look up envs, locals
                vector_push(items, lookup_variable(ast->val));
                break;
            case CONSTANT_AST:
                vector_push(items, ast->cnt);
                break;
            case APPLY_AST:
                vector_push(items, evaluate(ast->ap));
                break;
            default:
                error("oops maybe not implmented");
                return NULL;
        }
    }
    Constant * ret = execute(items);
    return ret;
}

void run(char *line) {
    printf("Evaluating: %s\n", line);
	Ast *ast = parser(line);
    Ast *top = vector_get(ast->ap->asts, 0);
    if (top->type == DEFINE_AST) {
        Ast *def_ast = vector_get(ast->ap->asts, 1);
        if (def_ast->type == APPLY_AST) {

            // Vector<Variable *>
            Vector *args = make_vector(def_ast->ap->asts->len - 1);
            int i;
            // (f x1 x2 .. xn) => iterate 1 to n
            for (i = 1; i < def_ast->ap->asts->len; i++) {
                Ast *ast = vector_get(def_ast->ap->asts, i);
                if (ast->type != VARIABLE_AST) {
                    error("illegal define source expression");
                    return;
                }
                vector_push(args, ast->val);
            }
            Ast *func_name_ast = vector_get(def_ast->ap->asts, 0);
            if (func_name_ast->type != VARIABLE_AST) {
                error("illegal define source expression");
                return;
            }

            Variable *v = func_name_ast->val;
            v->type = FUNCTION_TYPE_VARIABLE;

            Ast *content = vector_get(ast->ap->asts, 2);
            v->func = make_constructive_function(content, args);
            Ast *val_ast = make_ast_from_variable(v);

            map_set(global_variables, v->identifier, val_ast);
        } else if (def_ast->type == VARIABLE_AST) {
            if (ast->ap->asts->len != 3) {
                error("define args are too much");
                return;
            }
            map_set(global_variables, def_ast->val->identifier, vector_get(ast->ap->asts, 2));
        } else {
            error("illegal define statement");
            return;
        }
    }
    else {
        Constant *c = evaluate(ast->ap);
        print_constant(c);
        puts("");
    }
}
