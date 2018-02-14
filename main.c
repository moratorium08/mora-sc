#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "ast.h"
#include "vector.h"
#include "map.h"
#include "util.h"


typedef struct {
    int evaluating_func_id; // 現在のContextにおいて評価中の関数のid
    int is_tail; // Contextが末尾文脈か
    int env_size; // 関数適用を開始したときのenvの大きさ（末尾再帰のとき、ここまで戻る）
} Context;

Ast* parser(char *code);
Constant* evaluate(Application *ap, Vector *env, Context c);
Constant* eval_ast(Ast *ast, Vector* env, Context c);
int start_scope(Vector *tuples, Vector *env, Context ctx);
void end_scope(Vector *env);

int ERROR_FLAG = 0;

void error(char *s) {
    puts(s);
    ERROR_FLAG = 1;
}
void reset_error() {
    ERROR_FLAG = 0;
}

Map *global_variables; // type: Map<Ast*>
// Vector *local_variables; // type: Vector<Map<Ast*>>
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
    // local_variables = make_vector(100);
    //char *s = "(define x 3)";
    run("(define x 2)");
    run("x");
    run("1");
    run("(let ((x 3)) (quotient (* (+ x 1) 2) 3))");
    run("(* x (let ((x 5)) x))");
    run("((lambda (x y)(+ x y)) 1 2)");
    run("(define (f x) (+ x 2))");
    run("(f 5)");
    run("(+ (- (* (* 1 2) 3) (* 4 5)) (+ (* 6 7) (* 8 9)))");
    run("(define a 100)");
    run("a");
    run("(define b (+ 10 1))");
    run("b");
    run("(if (< a b) 10 20)");
    run("(define f (lambda (x) (+ x 1)))");
    run("(f 10)");
    run("(define g (lambda (x y) (+ x y)))");
    run("(g 3 5)");
    run("(define h (lambda (x) (lambda (y) (+ x (* a y)))))");
    run("((h 9) 9)");
    run("(define i (lambda (x) (if (< x a) a x)))");
    run("(i (f 100))");
    run("(let ((a 10) (b (f a)) (c (- b 1))) (let ((a 10) (b (f a)) (c (- b 1))) (+ (+ a b) c)))");
    run("a");
    // scope test
    run("(define a 0)");
    run("(define (f x) (+ x a))");
    run("(let ((a 1)) (f 0))");
    run("(define (fact x) (if (< 0 x) (* x (fact (- x 1))) 1))");
    run("(fact 4)");
    // run("(define (f x) (f x))");
    // run("(f 1)");
    run("(define (fact2 x y) (if (< x 1) 1 (if (< x 2) y (fact2 (- x 1) (modulo (* x y) 65537)))))");
    run("(fact2 1000 1)");
    run("(define (fact3 x) (if (< 0 x) (modulo (* x (fact3 (- x 1))) 65537) 1))");
    run("(fact3 1000)");
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
        if (i == 0 && s[i] == '-' && n > 1) continue;
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

int is_boolean(char *s) {
    int n = strlen(s);
    if (n != 2) {
        return 0;
    }
    return s[0] == '#' && (s[1] == 't' || s[1] == 'f');
}

Ast *handle_string_token(Token *token) {
    if (is_number(token->raw) == 1) {
        // TODO: オーバーフローを全く考慮していない
        // そもそもSchemeは規格として多倍長
        Ast *ast = make_int_ast(atoi(token->raw));
        return ast;
    }
    if (is_boolean(token->raw)) {
        Ast *ast = make_boolean_ast(token->raw[1] == 't');
        return ast;
    }
    if (strcmp("define", token->raw) == 0) {
        Ast *ast = make_define_ast();
        return ast;
    }
    if (strcmp("if", token->raw) == 0) {
        Ast *ast = make_if_ast();
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
    if (head->type == STR_TOKEN && token_tree->len == 1) {
        return handle_string_token(head);
    }
    else if (head->type != OPEN_BRACKET_TOKEN) {
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
    if (st != n) {
        char *tmp = cut_string(code, st, n+1);
        Token *token = create_str_token(tmp);
        vector_push(token_tree, token);
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

void set_env(Vector *env, char *name, Ast *ast) {
    int i = env->len - 1;
    for (; i >= 0; i--) {
        Map *m = vector_get(env, i);
        Ast *ret = map_get(m, name);
    }
}

// Vector <Constant *>
Constant *execute(Vector *items, Context ctx) {
    Constant *c = vector_get(items, 0);
    if (c->type != FUNCTION_TYPE_CONST) {
        error("non-functional value cannot be applied.");
        return NULL;
    }
    Function *f = c->func;
    Vector *env = f->env;
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

        Vector *tuples = _zip_vectors(arg_names, items);
        Constant *ret;

        int flag = 1;
        if (ctx.is_tail == 1 && ctx.evaluating_func_id == f->id) {
            int i;
            flag = 0;
            // printf("hoge%d\n", env->len);
            for (i = 0; i < (env->len - ctx.env_size + 1); i++) {
                vector_pop(env);
            }
        }
        start_scope(tuples, env, ctx);
        ctx.is_tail = 1;
        ctx.evaluating_func_id = f->id;
        ctx.env_size = env->len;
        ret = evaluate(f->ast->ap, env, ctx);
        if (flag) {
            end_scope(env);
        }

        return ret;
    } else if (f->type == LAMBDA_FUNCTION) {
        if (f->lam_ast->type != APPLY_AST) {
            error("non-functionla value cannot be applied.");
            return NULL;
        }
        Vector *arg_names = f->lam_names;
        Vector *tuples = _zip_vectors(arg_names, items);

        start_scope(tuples, env, ctx);
        Constant *ret;
        ret = evaluate(f->lam_ast->ap, env, ctx);
        end_scope(env);
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
Constant *builtin_modulo(Vector *items) {
    if (items->len != 3) {
        error("modulo: invalid arguments.");
    }
    Constant *c1 = vector_get(items, 1);
    Constant *c2 = vector_get(items, 2);
    if (c1->type != INTEGER_TYPE_CONST || c2->type != INTEGER_TYPE_CONST) {
        error("modulo: arguments must have Integer Type");
    }
    return make_int_constant(c1->integer_cnt % c2->integer_cnt);
}
Constant *builtin_lower(Vector *items) {
    if (items->len != 3) {
        error("<: invalid arguments.");
    }
    Constant *c1 = vector_get(items, 1);
    Constant *c2 = vector_get(items, 2);
    if (c1->type != INTEGER_TYPE_CONST || c2->type != INTEGER_TYPE_CONST) {
        error("<: arguments must have Integer Type");
    }
    return make_boolean_constant(c1->integer_cnt < c2->integer_cnt);
}

Constant *lookup_variable(Variable *v, Vector *env, Context ctx) {
    // lookup let scopes
    int i = env->len - 1;
    for (; i >= 0; i--) {
        Map *m = vector_get(env, i);
        Ast *ast = map_get(m, v->identifier);
        if (ast != NULL) {
            switch (ast->type) {
                case VARIABLE_AST:
                    if (ast->val->type != FUNCTION_TYPE_VARIABLE) {
                        error("not implemented");
                        return NULL;
                    }
                    return make_func_constant(ast->val->func);
                case APPLY_AST:
                    ctx.is_tail = 0;
                case CONSTANT_AST:
                case DEFINE_AST:
                case IF_AST:
                    return eval_ast(ast, env, ctx);
            }
        }
    }
    // global variable
    Ast *gv = map_get(global_variables, v->identifier);
    if (gv != NULL) {
        switch (gv->type) {
            case VARIABLE_AST:
                if (gv->val->type != FUNCTION_TYPE_VARIABLE) {
                    error("not implemented");
                    return NULL;
                }
                return make_func_constant(gv->val->func);
            case APPLY_AST:
                ctx.is_tail = 0;
            case CONSTANT_AST:
            case DEFINE_AST:
            case IF_AST:
                return eval_ast(gv, env, ctx);
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
    if (strcmp(v->identifier, "modulo") == 0) {
        return make_func_constant_primitive(&builtin_modulo, 2);
    }
    if (strcmp(v->identifier, "<") == 0) {
        return make_func_constant_primitive(&builtin_lower, 2);
    }
    else {
        char error_msg[1000];
        sprintf(error_msg, "Oops no variable: %s", v->identifier);
        error(error_msg);
        return NULL;
    }
}


// tuples: Vector<Ast *>
// This Ast must have APPLY_AST type
int start_scope(Vector *tuples, Vector *env, Context ctx) {
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

        int tmp = ctx.is_tail;
        ctx.is_tail = 0;
        Constant *c = eval_ast(val, env, ctx);
        ctx.is_tail = tmp;
        map_set(new_scope, name->val->identifier, make_constant_ast(c));
    }
    vector_push(env, new_scope);
    return 1;
}
void end_scope(Vector *env) {
    vector_pop(env);
}

Constant* if_eval(Application *ap, Vector *env, Context ctx) {
    int len = ap->asts->len;
    if (len < 3 || 4 < len) {
        error("Syntax Error: if has 3 or 4 args");
        return NULL;
    }
    Ast *cond = vector_get(ap->asts, 1);
    Ast *fst = vector_get(ap->asts, 2);
    Ast *snd = vector_get(ap->asts, 3);

    int tmp = ctx.is_tail;
    ctx.is_tail = 0;
    Constant *c = eval_ast(cond, env, ctx);
    ctx.is_tail = tmp;

    if (c->type != BOOLEAN_TYPE_CONST || c->bool_cnt) {
        return eval_ast(fst, env, ctx);
    } else {
        return eval_ast(snd, env, ctx);
    }
}

Constant* eval_ast(Ast *ast, Vector* env, Context ctx) {
    Constant *ret;
    switch (ast->type) {
        case VARIABLE_AST:
            ret = lookup_variable(ast->val, env, ctx);
            break;
        case CONSTANT_AST:
            ret = ast->cnt;
            break;
        case APPLY_AST:
            ret = evaluate(ast->ap, env, ctx);
            break;
        case IF_AST:
            ret = if_eval(ast->ap, env, ctx);
            break;
        case DEFINE_AST:
            error("define cannot be in such place");
            return NULL;
    }
    return ret;
}

Constant* evaluate(Application *ap, Vector *env, Context ctx) {
    int len = ap->asts->len;
    int i;
    Ast *top = vector_get(ap->asts, 0);
    if (top->type == IF_AST) {
        return if_eval(ap, env, ctx);
    }
    if (top->type == VARIABLE_AST && strcmp(top->val->identifier, "lambda") == 0) {
        if (ap->asts->len < 3) {
            error("lambda must have not less than 3 args");
            return NULL;
        }
        Ast *args_ast = vector_get(ap->asts, 1);
        if (args_ast->type != APPLY_AST) {
            error("illegal lambda");
            return NULL;
        }

        // Vector<Variable*>
        Vector *names = make_vector(args_ast->ap->asts->len);
        for (i = 0; i < args_ast->ap->asts->len; i++) {
            Ast *v_ast = vector_get(args_ast->ap->asts, i);
            if (v_ast->type != VARIABLE_AST) {
                error("illegal lambda");
                return NULL;
            }
            vector_push(names, v_ast->val);
        }

        Ast *ast = vector_get(ap->asts, 2);
        Constant *ret = make_lambda_constant(ast, names, env);
        return ret;
    }
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

        start_scope(tuples, env, ctx);
        Constant *ret;
        int tmp = ctx.is_tail;
        ctx.is_tail = 0;
        for (i = 2; i < ap->asts->len; i++) {
            // letは最後の実行については末尾文脈
            if ((i + 1) == ap->asts->len) ctx.is_tail = tmp;

            Ast *ast = vector_get(ap->asts, i);
            ret = eval_ast(ast, env, ctx);
        }
        end_scope(env);
        return ret;
    }

    Vector *items = make_vector(8);
    for (i = 0; i < len; i++) {
        // 末尾文脈ではない
        Ast *ast = vector_get(ap->asts, i);
        int tmp = ctx.is_tail;
        ctx.is_tail = 0;
        switch (ast->type) {
            case VARIABLE_AST:
                // TODO: look up envs, locals
                vector_push(items, lookup_variable(ast->val, env, ctx));
                break;
            case CONSTANT_AST:
                vector_push(items, ast->cnt);
                break;
            case APPLY_AST:
                vector_push(items, evaluate(ast->ap, env, ctx));
                break;
            case IF_AST:
                vector_push(items, if_eval(ast->ap, env, ctx));
                break;
            default:
                error("oops maybe not implmented");
                return NULL;
        }
        ctx.is_tail = tmp;
    }
    Constant * ret = execute(items, ctx);
    return ret;
}

void run(char *line) {
    printf("Evaluating: %s\n", line);
	Ast *ast = parser(line);
    //print_ast(ast, 0);

    Vector *top_env = make_vector(1);
    Context ctx = {-1, 1, 0};
    if (ast->type == VARIABLE_AST) {
        Constant *c = lookup_variable(ast->val, top_env, ctx);
        print_constant(c);
        puts("");
        return;
    } else if (ast->type == CONSTANT_AST) {
        print_constant(ast->cnt);
        puts("");
        return;
    }
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
            v->func = make_constructive_function(content, args, top_env);
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
        Constant *c = evaluate(ast->ap, top_env, ctx);
        print_constant(c);
        puts("");
    }
}
