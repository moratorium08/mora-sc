#ifndef CORE_INCLUDE
#define CORE_INCLUDE

#include "ast.h"
#include "map.h"
int ERROR_FLAG;
void setup();
Ast* parser(char *code);
Constant* evaluate(Application *ap, Vector *env, Context c);
Constant* eval_ast(Ast *ast, Vector* env, Context c);
int start_scope(Vector *tuples, Vector *env, Context ctx);
void end_scope(Vector *env);
Map *global_variables; // type: Map<Ast*>
// Vector *local_variables; // type: Vector<Map<Ast*>>
Constant *run(char *line);
#endif
