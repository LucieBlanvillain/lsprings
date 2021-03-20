all: main

main: main.o
	gcc -o main main.o -lX11

main.o: main.c
	gcc -c main.c
