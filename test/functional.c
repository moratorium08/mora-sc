#include <stdio.h>
#include "test.h"
#include "../core.h"

int main(void) {
    setup();
    run("(define x 2)");
    run("x");
    run("1");
    run("(let ((x 3)) (quotient (* (+ x 1) 2) 3))");
    run("(* x (let ((x 5)) x))");
    run("((lambda (x y)(+ x y)) 1 2)");
    run("(define (f x) (+ x 2))");
    run("(f 5)");
    // https://sites.google.com/site/isutbe2017/s/sos2
    puts("** S on S test **\n");
    run("10");
    run("(+ (- (* (* 1 2) 3) (* 4 5)) (+ (* 6 7) (* 8 9)))");
    run("(define a 100)");
    run("a");
    run("(define b (+ 10 1))");
    run("b");
    run("(if (< a b) 10 20)");
    run("(quote (+ 1 2))");
    run("(define f (lambda (x) (+ x 1)))");
    run("(f 10)");
    run("(define g (lambda (x y) (+ x y)))");
    run("(g 3 5)");
    run("(define h (lambda (x) (lambda (y) (+ x (* a y)))))");
    run("((h 9) 9)");
    run("(define i (lambda (x) (if (< x a) a x)))");
    run("(i (f 100))");
    run("(let ((a 1)) (i 10))");
    run("(let ((a 10) (b (f a)) (c (- b 1))) (let ((a 10) (b (f a)) (c (- b 1))) (+ (+ a b) c)))");
    run("a");
    run("(define l (cons 1 (cons 2 (cons 3 (list)))))");
    run("l");
    run("(car (cdr l))");
    puts("\n ** S on S test end");
    // scope test
    run("(define a 0)");
    run("(define (f x) (+ x a))");
    run("(let ((a 1)) (f 0))");
    run("(define (fact x) (if (< 0 x) (* x (fact (- x 1))) 1))");
    run("(fact 4)");
    // run("(define (f x) (f x))");
    // run("(f 1)");
    run("(define (fact2 x y) (if (< x 1) 1 (if (< x 2) y (fact2 (- x 1) (modulo (* x y) 65537)))))");
    run("(fact2 10000 1)");
    run("(define (fact3 x) (if (< 0 x) (modulo (* x (fact3 (- x 1))) 65537) 1))");
    run("(fact3 1000)");
    // run("(fact3 10000)"); -> overflow
    run("(cons 1 2)");
    run("'(+ '(+ 1) 2)");
}
