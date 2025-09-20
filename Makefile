INCLUDE := include
LIB := lib
LIB_SRC := lib_src

$(LIB)/shared_stuff.o: $(LIB_SRC)/shared_stuff.c $(INCLUDE)/shared_stuff.h
	gcc -o $(LIB)/shared_stuff.o -c $(LIB_SRC)/shared_stuff.c -I$(INCLUDE)
