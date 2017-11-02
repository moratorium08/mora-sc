#include "vector.h"

typedef struct Constant Constant;

typedef Constant *(*SchemeFunc)(Vector *);

typedef struct Function {
    int argc;
    SchemeFunc func;
} Function;


typedef enum {
    INTEGER_TYPE_VARIABLE,
    BOOLEAN_TYPE_VARIABLE,
    FUNCTION_TYPE_VARIABLE,
} VariableType;

typedef struct {
    VariableType type;
    int defined; // if defined this is set to 1
    char *identifier;
    union {
        int val;
        struct {
            Function *func;
            Vector *names;
        };
    };
} Variable;

typedef enum {
    INTEGER_TYPE_CONST,
    BOOLEAN_TYPE_CONST,
    FUNCTION_TYPE_CONST,
} ConstantType;

struct Constant {
    ConstantType type;
    union {
        int integer_cnt;
        int bool_cnt;
        Function *func;
    };
};

typedef struct {
    Vector *asts; // Type Vector<Ast *>
} Application;

typedef enum {
    APPLY_AST,
    VARIABLE_AST,
    CONSTANT_AST,
    DEFINE_AST
} ASTType;

typedef struct {
    ASTType type;
    union {
        Variable *val;
        Constant *cnt;
        Application *ap;
    };
} Ast;

Application *make_application();
Ast *make_apply_ast();
Ast *make_variable_ast(char *id);
Ast *make_int_ast(int x);
Ast *make_define_ast();
Constant *make_int_constant(int x);
Constant *make_func_constant(SchemeFunc f, int argc);
void print_constant(Constant *c);
