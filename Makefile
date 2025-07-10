.PHONY: run all

all:
	gcc main.c -lraylib -lX11 -lGL -lm -I./include -o game

run: all
	./game

test:
	gcc test.c -lGL -lraylib -lX11 -lode && ./a.out
