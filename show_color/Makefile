INCLUDE := ../include
LIB := ../lib

all: main

clean:
	rm main

main: main.c $(INCLUDE)/types.h
	gcc -o main main.c -I$(INCLUDE) -L$(LIB)
