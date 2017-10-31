#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "linklist.h"
#include "ast.h"
#include "vector.h"
#include "util.h"

Ast* parser(char *code);

void panic(char *s) {
    puts(s);
    exit(-1);
}

Variable *envs;
void run(char *line);

int main(void) {
    char *s = "(+ 1 2)";
    run(s);
    return 0;
}

char *cut_string(char *code, int st, int ed) {
    if (st >= ed) {
        panic("st >= ed is invalid");
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
                panic("oops. invalid token.");
        }
    }
    panic("invalid syntax.");
}

Ast * create_ast_from_token_tree(Vector *token_tree) {
    if (token_tree->len == 0) {
        panic("token tree size is zero.");
    }
    Token *head = vector_get(token_tree, 0);
    if (head->type != OPEN_BRACKET_TOKEN) {
        panic("invalid syntax.");
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

Constant *execute(Vector *items) {
    Constant *c = vector_get(items, 0);
    if (c->type != FUNCTION_TYPE_CONST) {
        panic("non-functional value cannot be applied.");
    }
    Function *f = c->func;
    if (f->argc != (items->len - 1)) {
        panic("invalid arguments");
    }
    SchemeFunc raw_func = f->func;
    Constant *ret = (raw_func)(items);
    return ret;
}

Constant* evaluate(Application *ap) {
    int len = ap->asts->len;
    int i;

    Vector *items = make_vector(8);
    for (i = 0; i < len; i++) {
        Ast *ast = vector_get(ap->asts, i);
        switch (ast->type) {
            case VARIABLE_AST:
                // TODO: look up envs, locals
                assert(1);
            case CONSTANT_AST:
                vector_push(items, ast->cnt);
            case APPLY_AST:
                vector_push(items, evaluate(ast->ap));
            default:
                panic("oops maybe not implmented");
        }
    }
    Constant * ret = execute(items);
    return ret;
}

void run(char *line) {
	Ast *ast = parser(line);
    Constant *c = evaluate(ast->ap);
    //print_constant(c);
}
