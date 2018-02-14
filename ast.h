#include "vector.h"

typedef struct Constant Constant;
typedef struct Ast Ast;

typedef Constant *(*SchemeFunc)(Vector *);

typedef enum {
    PRIMITIVE_FUNCTION,
    CONSTRUCTIVE_FUNCTION,
    LAMBDA_FUNCTION
} FunctionType;

// 環境が足りていないがはてさて
typedef struct Function {
    FunctionType type;
    int argc;
    Vector *env; // Vector<Map<Ast*>>
    union {
        SchemeFunc func;
        struct {
            Ast *ast;
            Vector *names; //Vector<Variable*>
        };
        struct {
            Ast *lam_ast;
            Vector *lam_names; // Vector<Variable*>
        };
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
        Function *func;
    };
} Variable;

typedef enum {
    INTEGER_TYPE_CONST,
    BOOLEAN_TYPE_CONST,
    FUNCTION_TYPE_CONST
} ConstantType;

struct Constant {
    ConstantType type;
    union {
        // Type: Integer
        int integer_cnt;
        // Type: Boolean
        int bool_cnt;
        // Type: Function
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
Ast *make_constant_ast(Constant *c);
Ast *make_ast_from_variable(Variable *v);
Function *make_constructive_function(Ast *ast, Vector * args, Vector *env);
Constant *make_int_constant(int x);
Constant *make_func_constant(Function *f);
Constant *make_func_constant_primitive(SchemeFunc f, int argc);
Constant *make_lambda_constant(Ast *ast, Vector *args, Vector *env);
void print_constant(Constant *c);
void print_ast(Ast *ast, int indent);
