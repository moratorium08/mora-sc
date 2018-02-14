#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "vector.h"

void debug_info(char *s) {
    printf("[INFO]%s\n", s);
}

int calculate_new_chunk_size(int len) {
    if (len == 0) {
        return 2;
    }
    return len;
}

Vector *make_vector(int size) {
    Vector *v;
    v = (Vector *)malloc(sizeof(Vector));
    if (v == NULL) {
        return NULL;
    }
    v->body = malloc(size * sizeof(void *));
    if (v->body == NULL) {
        return NULL;
    }
    v->len = 0;
    v->cur_max = size;
    return v;
}

Vector *copy_vector(Vector *vec) {
    Vector *v = make_vector(vec->len);
    int i = 0;
    for (; i < vec->len; i++) {
        void *item = vector_get(vec, i);
        vector_push(v, item);
    }
    return v;
}

int extend_vector(Vector *v, int add_size) {
    void *new_body = malloc((v->cur_max + add_size) * sizeof(void *));
    if (new_body == NULL) {
        return -1;
    }
    memcpy(new_body, v->body, sizeof(void *) * v->len);
    free(v->body);
    v->body = new_body;
    v->cur_max += add_size;
    return 1;
}

int append_vector(Vector *v, void *item) {
    if (v->len == v->cur_max) {
        int new_chunk_size = calculate_new_chunk_size(v->len);
        extend_vector(v, new_chunk_size);
    }
    v->body[v->len++] = item;
    return 1;
}

void *vector_get(Vector *v, int idx) {
    if (idx >= v->len || (-idx - 1) >= v->len) {
        debug_info("Index out of range");
        assert(1);
    }
    if (idx < 0) {
        return v->body[v->len + idx];
    }
    return v->body[idx];
}

void vector_put(Vector *v, int idx, void *item) {
    if (idx >= v->len || (-idx - 1) >= v->len) {
        debug_info("Index out of range");
        assert(1);
    }
    if (idx < 0) {
        v->body[v->len - idx] = item;
    }
    v->body[idx] = item;
}

void *vector_pop(Vector *v) {
    void *ret = vector_get(v, -1);
    v->len--;
    return ret;
}

void vector_push(Vector *v, void *item) {
    append_vector(v, item);
}

void delete_vector(Vector *v) {
    // TODO
}
