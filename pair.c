#include <stdlib.h>
#include "pair.h"

Pair *make_pair(void *fst, void *snd) {
    Pair *p = malloc(sizeof(Pair));
    p->fst = fst;
    p->snd = snd;
    return p;
}
