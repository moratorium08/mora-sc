all: build

test: functional unit

functional: util.o map.o vector.o ast.o core.o pair.o functional.o test.o
	cd build && gcc -o t-functional util.o map.o vector.o ast.o core.o pair.o test.o functional.o
	./build/t-functional

unit: unit-pair

unit-pair: pair.o test.o test-pair.o
	cd build && gcc -o unit-pair test.o pair.o test-pair.o
	./build/unit-pair

build: main.o map.o util.o ast.o vector.o core.o pair.o
	cd build && gcc -o ../mora-sc main.o util.o map.o vector.o ast.o core.o pair.o

main.o:
	gcc -o build/main.o -c main.c

util.o:
	gcc -o build/util.o -c util.c

map.o:
	gcc -o build/map.o -c map.c

vector.o:
	gcc -o build/vector.o -c vector.c

ast.o:
	gcc -o build/ast.o -c ast.c

core.o:
	gcc -o build/core.o -c core.c

pair.o:
	gcc -o build/pair.o -c pair.c

functional.o:
	gcc -o build/functional.o -c test/functional.c

test-pair.o:
	gcc -o build/test-pair.o -c test/test-pair.c

test.o:
	gcc -o build/test.o -c test/test.c
