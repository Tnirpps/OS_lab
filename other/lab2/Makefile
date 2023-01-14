all: a.out child

a.out: main.c
	gcc main.c

child: child.o
	gcc child.o -o child

child.o: child.c
	gcc -c child.c

clean:
	rm -rf *.o a.out

