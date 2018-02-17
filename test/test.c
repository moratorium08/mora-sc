#include <stdio.h>
#include <string.h>
#include "../vector.h"
#include "../map.h"
#include "../ast.h"
#include "test.h"

// context
int TEST_NEST= 0;
int TEST_ERROR = 0;
int TEST_SUCCESS = 0;

void print_spaces_test(int x) {
    int i = 0;
    for (; i < x; i++) {
        printf(" ");
    } }

void fexpect_int(char *filename, int line, int a, int b) {
    if (a == b) {
        TEST_SUCCESS += 1;
        return;
    }
    print_spaces_test(TEST_NEST);
    printf("%s: %d, expect %d but get %d\n", filename, line, a, b);
    TEST_ERROR += 1;
}

void fexpect_str(char *filename, int line, char *a, char *b) {
    if (strcmp(a, b) == 0) {
        TEST_SUCCESS += 1;
        return;
    }
    print_spaces_test(TEST_NEST);
    printf("%s: %d, expect %s but get %s\n", filename, line, a, b);
    TEST_ERROR += 1;
}

void ferror_msg(char *filename, int line, char *c) {
    printf("%s: %d, error: %s\n", filename, line, c);
    TEST_ERROR += 1;
}

void describe(char *description) {
    puts(description);
    TEST_NEST++;
}

void described() {
    TEST_NEST--;
}

int summary() {
    int ret = TEST_ERROR == 0;
    printf("succeeded: %d/%d\n", TEST_SUCCESS, TEST_ERROR + TEST_SUCCESS);
    TEST_ERROR = 0;
    TEST_SUCCESS = 0;
    described();
    return ret;
}
/*

void test_vector() {
    describe("Vector unit test");

    Vector *v = make_vector(10);
    append_vector(v, "hello");
    append_vector(v, "world");
    char *r0 = vector_get(v, 0);
    char *r1 = vector_get(v, 1);
    expect_str(r0, "hello");
    expect_str(r0, "world");

    char *r2 = vector_pop(v);
    vector_push(v, "nemui");
    vector_push(v, "golang");
    for (int i = 0; i < v->len; i++) {
        printf("%s\n", vector_get(v, i));
    }

    summary();
}

void test_map() {

}

void test_ast() {

}

void run_unit() {
    test_vector();
    test_map();
    test_ast();
}

int main(void) {
    run_unit();
    return 0;
}
*/
