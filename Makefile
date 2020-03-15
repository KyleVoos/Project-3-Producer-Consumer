all: main.c buffer.h
	gcc -o p3 -pthread main.c buffer.h

test: main.c buffer.h
	make all
	./p3 20 3 2