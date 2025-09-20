INCLUDE := ../include
LIB := ../lib

all: dskusg

global: $(HOME)/.local/bin/dskusg

clean:
	rm dskusg $(HOME)/.local/bin/dskusg

$(HOME)/.local/bin/dskusg: dskusg
	cp dskusg $(HOME)/.local/bin/dskusg

dskusg: dskusg.c $(INCLUDE)/types.h
	gcc -o dskusg dskusg.c -I$(INCLUDE) -L$(LIB)
