INCLUDE := include
LIB := lib
LIB_SRC := lib_src

MCFLAGS := -Wall -Wextra -Wpedantic -I$(INCLUDE)
MLDFLAGS := -g
CC := gcc

SRC_FILES := $(wildcard */*.c)

DIRS := $(sort $(dir $(SRC_FILES)))
DIRS := $(patsubst %/,%,$(DIRS))
DIRS := $(filter-out $(LIB_SRC),$(DIRS))

OBJS := $(foreach d,$(DIRS),$(patsubst $(d)/%.c,$(d)/%.o,$(wildcard $(d)/*.c)))

EXES := $(foreach d,$(DIRS),build/$(notdir $(d)))


.PHONY: all clean print_info
all: $(LIB)/shared_stuff.o print_info $(EXES)

# compile object files
%.o: %.c
	@echo Compiling $<
	#@mkdir -p $(dir $<)/obj
	$(CC) $(MCFLAGS) -c -o $@ $<

# link executables
build/%: $(patsubst %.c,%.o,$(wildcard $(notdir $@)/*.c))
	@echo Linking $@ from $^
	$(CC) $(MLDFLAGS) -o $@ $^

$(LIB)/shared_stuff.o: $(LIB_SRC)/shared_stuff.c $(INCLUDE)/shared_stuff.h Makefile
	gcc -o $(LIB)/shared_stuff.o -c $(LIB_SRC)/shared_stuff.c -I$(INCLUDE)

clean:
	rm -f $(OBJS) $(EXES)

print_info:
	@echo DIRS : $(DIRS)
	@echo OBJS : $(OBJS)
	@echo EXES : $(EXES)


#
#EXPORTS := $(patsubst %,$(HOME)/.local/bin/%,$(EXES))

#.PHONY: export
#export: $(EXPORTS)
#$(EXPORTS): $(EXES)
#	cp $< $@
