INCLUDE := include
LIB := lib
LIB_SRC := lib_src

CFLAGS := -Wall -Wextra -Wpedantic -I$(INCLUDE) $(EXTRA_CFLAGS)
LDFLAGS := $(EXTRA_LDFLAGS)
CC := gcc
SRC_FILES := $(wildcard */*.c)

DIRS := $(sort $(dir $(SRC_FILES)))
DIRS := $(patsubst %/,%,$(DIRS))
DIRS := $(filter-out $(LIB_SRC) %nope,$(DIRS))

OBJS := $(foreach d,$(DIRS),$(patsubst $(d)/%.c,$(d)/%.o,$(wildcard $(d)/*.c)))

EXES := $(foreach d,$(DIRS),build/$(notdir $(d)))

.PHONY: all clean clean_exported export
all: $(LIB)/shared_stuff.o build $(EXES)

$(LIB)/shared_stuff.o: $(LIB) $(LIB_SRC)/shared_stuff.c $(INCLUDE)/shared_stuff.h Makefile
	gcc -o $(LIB)/shared_stuff.o -c $(LIB_SRC)/shared_stuff.c -I$(INCLUDE)


# COMPILATION + LINKING PHASE
#
# compile object files
# depends on respective source file and a header with the same name (if it exists)
$(OBJS): %.o: %.c $(wildcard $(dir $@)/*.h) Makefile
	$(CC) $(CFLAGS) -c -o $@ $<
#
# link executables
# depends on all respective object files and the lib/*.o object files
.SECONDEXPANSION:
$(EXES): $$(patsubst %.c,%.o,$$(wildcard $$(notdir $$@)/*.c)) $$(wildcard $$(LIB)/*.o)
	$(CC) $(LDFLAGS) -o $@ $(filter-out $(LIB)/%.o,$^) $(LIB)/*.o
#

# EXPORT RULE
INSTALL_PREFIX := $(HOME)/.local
BINDIR := $(INSTALL_PREFIX)/bin
EXPORT_EXES := $(patsubst build/%,$(BINDIR)/%,$(EXES))

$(EXPORT_EXES): $(BINDIR)/%: build/%
	cp -i $< $@

export: $(LIB)/shared_stuff.o $(EXPORT_EXES)

# OTHER RULES
clean:
	rm -f */*.o build/*
clean_exported: clean
	rm -f $(EXPORT_EXES)

build:
	mkdir build
$(LIB):
	mkdir $(LIB)
