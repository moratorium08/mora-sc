#include "vector.h"

typedef struct Constant Constant;
typedef struct Ast Ast;

typedef Constant *(*SchemeFunc)(Vector *);

typedef enum {
    PRIMITIVE_FUNCTION,
    CONSTRUCTIVE_FUNCTION
} FunctionType;

typedef struct Function {
    FunctionType type;
    int argc;
    union {
        SchemeFunc func;
        Ast *ast;
    };
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

struct Ast {
    ASTType type;
    union {
        Variable *val;
        Constant *cnt;
        Application *ap;
    };
};

Application *make_application();
Ast *make_apply_ast();
Ast *make_variable_ast(char *id);
Ast *make_int_ast(int x);
Ast *make_define_ast();
Function *make_constructive_function(Ast *ast, int argc);
Constant *make_int_constant(int x);
Constant *make_func_constant_primitive(SchemeFunc f, int argc);
void print_constant(Constant *c);
