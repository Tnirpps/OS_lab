all: node main

main: main.o zmq_tools.o
	gcc -pthread main.o zmq_tools.o -o main -lzmq
	
node: node.o zmq_tools.o
	gcc node.o zmq_tools.o -o node -lzmq

main.o: main.c
	gcc -c main.c

node.o: node.c
	gcc -c node.c

zmq_tools.o: zmq_tools.c
	gcc -c zmq_tools.c

clean:
	rm -rf *.o main node

