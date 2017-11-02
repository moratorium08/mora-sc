#include "vector.h"

// そのうちちゃんと実装する
// とりあえずはO(n)でもいいや
typedef struct {
    // char **keys;
    // void **items;
    int cur_max;
    int len;
    Vector *items;
    Vector *keys;
} Map;

Map *make_map(int size);
void map_set(Map *map, char *key, void *item);
void *map_get(Map *map, char *key);
