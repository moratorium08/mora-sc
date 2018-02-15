#include "vector.h"

typedef struct Constant Constant;
typedef struct Ast Ast;

typedef Constant *(*SchemeFunc)(Vector *);

typedef enum {
    PRIMITIVE_FUNCTION,
    CONSTRUCTIVE_FUNCTION,
    LAMBDA_FUNCTION
} FunctionType;

// 末尾再帰のためfunctionに一意なidを与える
int GLOBAL_CURRENT_ID;

typedef struct Function {
    FunctionType type;
    int argc;
    Vector *env; // Vector<Map<Ast*>>
    int id; // unique func id
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

typedef struct {
    int evaluating_func_id; // 現在のContextにおいて評価中の関数のid
    int is_tail; // Contextが末尾文脈か
    int env_size; // 関数適用を開始したときのenvの大きさ（末尾再帰のとき、ここまで戻る）
} Context;

typedef enum {
    INTEGER_TYPE_CONST,
    BOOLEAN_TYPE_CONST,
    FUNCTION_TYPE_CONST,
    PAIR_TYPE_CONST,
    TAIL_TYPE_CONST, // for tail recursion
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
        // Type: Pair
        // Type Tail
        Vector *items;
    };
};

typedef struct {
    Vector *asts; // Type Vector<Ast *>
} Application;

typedef enum {
    APPLY_AST,
    VARIABLE_AST,
    CONSTANT_AST,
    DEFINE_AST,
    IF_AST
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
Ast *make_boolean_ast(int b);
Ast *make_define_ast();
Ast *make_if_ast();
Ast *make_constant_ast(Constant *c);
Ast *make_ast_from_variable(Variable *v);
Function *make_constructive_function(Ast *ast, Vector * args, Vector *env);
Constant *make_int_constant(int x);
Constant *make_boolean_constant(int b);
Constant *make_func_constant(Function *f);
Constant *make_func_constant_primitive(SchemeFunc f, int argc);
Constant *make_lambda_constant(Ast *ast, Vector *args, Vector *env);
Constant *make_tail_constant(Vector *args); // Vector<Ast *>
void print_constant(Constant *c);
void print_ast(Ast *ast, int indent);
