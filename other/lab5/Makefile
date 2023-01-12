FLAGS = -O2
# can be usefull =>  export LD_LIBRARY_PATH=/home/.....:$LD_LIBRARY_PATH

# a1.out static1
# a2.out static2
#  b.out dynamic

all: static1 static2 dynamic

static1: dynamic1 static_main
	gcc $(FLAGS) main.o -L. -lDyn1 -o a1.out

static2: dynamic2 static_main
	gcc $(FLAGS) main.o -L. -lDyn2 -o a2.out

static_main: main.c
	gcc $(FLAGS) -c main.c

dynamic: main_dyn dynamic1 dynamic2
	gcc $(FLAGS) main_dyn.o -o b.out -ldl

main_dyn: main_dyn.c
	gcc $(FLAGS) -c main_dyn.c -ldl

# dynamic gen 

dynamic1: realisation.h realisation1.c
	gcc $(FLAGS) -shared -o libDyn1.so -fPIC realisation1.c

dynamic2: realisation.h realisation2.c
	gcc $(FLAGS) -shared -o libDyn2.so -fPIC realisation2.c

clean:
	rm -rf *.o *.so a1.out a2.out b.out

