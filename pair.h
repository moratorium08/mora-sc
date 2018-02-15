#ifndef VECTOR_INCLUDE
#define VECTOR_INCLUDE

typedef struct {
    void * fst;
    void * snd;
} Pair;

Pair *make_pair(void *fst, void *snd);
#endif
