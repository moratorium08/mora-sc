#include "vector.h"

typedef enum {
    INTEGER_TYPE_VARIABLE,
    BOOLEAN_TYPE_VARIABLE,
} VariableType;


typedef struct {
    VariableType type;
    int defined; // if defined this is set to 1
    int x;
    char *identifier;
} Variable;


typedef enum {
    INTEGER_TYPE_CONST,
    BOOLEAN_TYPE_CONST,
} ConstantType;


typedef struct {
    ConstantType type;
    union {
        int integer_cnt;
        int bool_cnt;
    };
} Constant;

typedef struct {
    Vector *asts; // Type Vector<Ast *>
} Application;


typedef enum {
    APPLY_AST,
    VARIABLE_AST,
    CONSTANT_AST
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
Ast *make_variable_ast(VariableType t, char *id);
