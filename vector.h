
#ifndef VECTOR_INCLUDE
#define VECTOR_INCLUDE
typedef struct {
    int len;
    int **body;
    int cur_max;
} Vector;

Vector *make_vector(int size);
int extend_vector(Vector *v, int add_size);
int append_vector(Vector *v, void *item);
void *vector_get(Vector *v, int idx);
void vector_put(Vector *v, int idx, void *item);
void *vector_pop(Vector *v);
void vector_push(Vector *v, void *item);
void delete_vector(Vector *v);
#endif
