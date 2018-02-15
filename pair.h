#ifndef PAIR_INCLUDE
#define PAIR_INCLUDE

typedef struct {
    void * fst;
    void * snd;
} Pair;

Pair *make_pair(void *fst, void *snd);
#endif
