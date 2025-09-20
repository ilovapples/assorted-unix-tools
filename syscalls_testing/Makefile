CFLAGS := -std=c99 -Wall -Wextra -pedantic 
all: build/main

clean:
	rm build/* obj/*

build/main: obj/main.o obj/term_set.o obj/sysio.o
	gcc -o build/main obj/main.o obj/term_set.o obj/sysio.o

obj/main.o: main.c term_set.h sysio.h
	gcc -o obj/main.o -c main.c $(CFLAGS)

obj/term_set.o: term_set.c term_set.h sysio.h
	gcc -o obj/term_set.o -c term_set.c $(CFLAGS)

obj/sysio.o: sysio.c sysio.h
	gcc -o obj/sysio.o -c sysio.c $(CFLAGS)
