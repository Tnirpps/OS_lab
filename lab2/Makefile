all: a.out child_1

a.out: ./src/main.c
	gcc ./src/main.c

child_1: child_1.o strlib.o
	gcc child_1.o strlib.o -o child_1
	
strlib.o: ./src/strlib.c
	gcc -c ./src/strlib.c
	
child_1.o: ./src/child_1.c
	gcc -c ./src/child_1.c

clean:
	rm -rf *.o a.out
