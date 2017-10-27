typedef enum VariableType {
    INTEGER_TYPE,
    BOOLEAN_TYPE,
} VariableType;

typedef struct Variable {
    VariableType type;
    int x;
} Variable;

typedef struct Operation {

} Operation;

typedef struct LinkListAst {
    struct Ast *ast;
    struct LinkListAst *next;
} LinkListAst;


typedef struct Application {
    Operation *op;
    LinkListAst *linklist;
} Application;


typedef enum ASTType{
    OPERATION_AST,
    APPLY_AST,
    PRIMITIVE_AST,
} ASTType;


typedef struct Ast {
    ASTType type;
    union {
        Variable *val;
        Operation *op;
        Application *ap;
    };
} Ast;
