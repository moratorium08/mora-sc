#include <stdio.h>
#include <string.h>
#include "test.h"
#include "../core.h"
#define expect_c_int(c, x) fexpect_c_int(__FILE__, __LINE__, c, x)


void fexpect_c_int(char *filename, int line, Constant *c, int x) {
    if (c->type != INTEGER_TYPE_CONST) {
        char buf[1024];
        ferror_msg(filename, line, "type error: expect int but not");
        return;
    }
    expect_int(x, c->integer_cnt);
}

int main(void) {
    setup();
    Constant *c;
    run("(define x 2)");
    expect_c_int(run("x"), 2);
    expect_c_int(run("1"), 1);

    run("(let ((x 3)) (quotient (* (+ x 1) 2) 3))");
    expect_c_int(run("(* x (let ((x 5)) x))"), 10);
    expect_c_int(run("((lambda (x y)(+ x y)) 1 2)"), 3);
    run("(define (f x) (+ x 2))");
    expect_c_int(run("(f 5)"), 7);
    // https://sites.google.com/site/isutbe2017/s/sos2
    describe("** S on S test **\n");
    expect_c_int(run("10"), 10);
    expect_c_int(run("(+ (- (* (* 1 2) 3) (* 4 5)) (+ (* 6 7) (* 8 9)))"), 100);
    run("(define a 100)");
    expect_c_int(run("a"), 100);
    run("(define b (+ 10 1))");
    expect_c_int(run("b"), 11);
    expect_c_int(run("(if (< a b) 10 20)"), 20);

    c = run("(quote (+ 1 2))");
    if (!is_list(c)) {
        error_msg("expect type list.");
    } else {
        Constant *fst = c->pair->fst;
        Constant *snd = c->pair->snd;
        if (strcmp(fst->symbol, "+") != 0) {
            char buf[1024];
            sprintf(buf, "expect + but %s.", fst->symbol);
            error_msg(buf);
        } else  {
            expect_c_int(snd->pair->fst, 1);
        }
    }

    run("(define f (lambda (x) (+ x 1)))");
    run("(f 10)");
    run("(define g (lambda (x y) (+ x y)))");
    expect_c_int(run("(g 3 5)"), 8);
    run("(define h (lambda (x) (lambda (y) (+ x (* a y)))))");
    expect_c_int(run("((h 9) 9)"), 909);
    run("(define i (lambda (x) (if (< x a) a x)))");
    expect_c_int(run("(i (f 100))"), 101);
    expect_c_int(run("(let ((a 1)) (i 10))"), 100);
    expect_c_int(run("(let ((a 10) (b (f a)) (c (- b 1))) (let ((a 10) (b (f a)) (c (- b 1))) (+ (+ a b) c)))"), 121);
    expect_c_int(run("a"), 100);

    run("(define l (cons 1 (cons 2 (cons 3 (list)))))");
    c = run("l");
    // c = run("(list)");
    print_constant(c);
    if (!is_list(c)) {
        error_msg("expect type list");
    } else {
        Constant *top = c->pair->fst;
        expect_c_int(top, 1);
        if (is_list(c->pair->snd)) {
            error_msg("expect type list");
        }
    }
    expect_c_int(run("(car (cdr l))"), 2);

    // scope test
    run("(define a 0)");
    run("(define (f x) (+ x a))");
    expect_c_int(run("(let ((a 1)) (f 0))"), 0);
    run("(define (fact x) (if (< 0 x) (* x (fact (- x 1))) 1))");
    expect_c_int(run("(fact 4)"), 24);
    run("(define (fact2 x y) (if (< x 1) 1 (if (< x 2) y (fact2 (- x 1) (modulo (* x y) 65537)))))");
    expect_c_int(run("(fact2 10000 1)"), 2296);
    run("(define (fact3 x) (if (< 0 x) (modulo (* x (fact3 (- x 1))) 65537) 1))");
    expect_c_int(run("(fact3 1000)"), 17173);
    run("(cons 1 2)");

    run("'(+ '(+ 1) 2)");
    return summary();
}
