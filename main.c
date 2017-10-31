#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "linklist.h"
#include "ast.h"
#include "vector.h"
#include "util.h"

void parser(Ast *ast, char *code);

void panic(char *s) {
    puts(s);
    exit(-1);
}

Variable *envs;
int main(void) {
    char *s = "(+ 1 2)";
    Ast *ast;
    parser(ast, s);
    return 0;
}

char *cut_string(char *code, int st, int ed) {
    if (st >= ed) {
        panic("st >= ed is invalid");
    }
    char *new_str = malloc(ed - st + 1);
    memcpy(new_str, code + st, st - ed);
    new_str[st - ed] = '\x00';
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

Ast *handle_string_token(Token *token) {
    char *name = dup_str(token->raw);

}


int _dfs_application_from_token(Ast *ast, Vector *token_tree, int idx) {
    while (idx < token_tree->len) {
        Token *token = vector_get(token_tree, idx);
        Ast *child_ast;
        switch (token->type) {
            case OPEN_BRACKET_TOKEN:
                child_ast = make_apply_ast();
                _dfs_application_from_token(child_ast, token_tree, idx + 1);
                vector_push(ast->ap->asts, child_ast);
                break;
            case CLOSE_BRACKET_TOKEN:
                return idx + 1;
                break;
            case STR_TOKEN:
                child_ast = make_variable_ast(INTEGER_TYPE_VARIABLE, token->raw);
                vector_push(ast->ap->asts, child_ast);
                break;
            default:
                panic("oops. invalid token.");
        }
    }
    panic("invalid syntax.");
}

void create_ast_from_token_tree(Ast *ast, Vector *token_tree) {
    if (token_tree->len == 0) {
        panic("token tree size is zero.");
    }
    Token *head = vector_get(token_tree, 0);
    if (head->type != OPEN_BRACKET_TOKEN) {
        panic("invalid syntax.");
    }
    ast = make_apply_ast();
    _dfs_application_from_token(ast, token_tree, 1);
}

void parser(Ast *ast, char *code) {
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
                st = i;
                break;
            case ')':
                token = create_bracket_token(CLOSE_BRACKET_TOKEN);
                vector_push(token_tree, token);
                st = i;
                break;
            // 明らかに""とか'()とかキャッチできないので
            // TODO
            case ' ':
                tmp = cut_string(code, st, i);
                token = create_str_token(tmp);
                vector_push(token_tree, token);
                st = i;
                break;
            default:
                break;
        }
    }
    create_ast_from_token_tree(ast, token_tree);
}

void run(char *line) {
	Ast *ast;
    parser(ast, line);
}
