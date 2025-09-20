BINC := ../../../BINC

all: stripbs

global: $(HOME)/.local/bin/stripbs

clean:
	rm stripbs $(HOME)/.local/bin/stripbs

$(HOME)/.local/bin/stripbs: stripbs
	cp stripbs $(HOME)/.local/bin/stripbs

stripbs: main.c $(BINC)/types.h
	gcc -o stripbs main.c -I$(BINC)
