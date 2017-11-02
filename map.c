#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Map *make_map(int size) {
    Map *m;
    m = (Map *)malloc(sizeof(Map));
    if (m == NULL) {
        return NULL;
    }
    memset(m, 0, sizeof(Map));

    m->items = make_vector(size);
    m->keys = make_vector(size);
    if (m->items == NULL || m->keys == NULL) {
        return NULL;
    }
    m->cur_max = size;
    m->len = 0;
    return m;
}

void map_set(Map *map, char *key, void *item) {
    int i;
    for (i = 0; i < map->len; i++) {
        char *k = vector_get(map->keys, i);
        if (strcmp(k, key) == 0) {
            vector_put(map->items, i, item);
            return;
        }
    }
    vector_push(map->keys, key);
    vector_push(map->items, item);
}

void *map_get(Map *map, char *key) {
    int i;
    for (i = 0; i < map->len; i++) {
        char *k = vector_get(map->keys, i);
        if (strcmp(k, key) == 0) {
            return vector_get(map->items, i);
        }
    }
    return NULL;
}
