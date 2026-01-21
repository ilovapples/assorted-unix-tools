INCLUDE := include
LIB := lib
LIB_SRC := lib_src

SHAREDSTUFF_LIB := shared_stuff
SHAREDSTUFF_LIB_A := lib$(SHAREDSTUFF_LIB).a

CFLAGS := -Wall -Wextra -Wpedantic -I$(INCLUDE) $(EXTRA_CFLAGS)
LDFLAGS := $(EXTRA_LDFLAGS) -L$(LIB) -l$(SHAREDSTUFF_LIB)
CC := cc
AR := ar
SRC_FILES := $(wildcard */*.c)

DIRS := $(sort $(dir $(SRC_FILES)))
DIRS := $(patsubst %/,%,$(DIRS))
DIRS := $(filter-out $(LIB_SRC) %nope,$(DIRS))

OBJS := $(foreach d,$(DIRS),$(patsubst $(d)/%.c,$(d)/%.o,$(wildcard $(d)/*.c)))

EXES := $(foreach d,$(DIRS),build/$(notdir $(d)))

.PHONY: all clean clean_exported clean_libs clean_all export
all: build/SUBMODULES_INITED $(LIB) $(LIB)/$(SHAREDSTUFF_LIB_A) build $(EXES)

$(LIB)/%.o: $(LIB_SRC)/%.c $(wildcard $(INCLUDE)/*.h) Makefile | $(LIB) build/SUBMODULES_INITED
	$(CC) -o $@ -c $< $(CFLAGS)

$(LIB)/$(SHAREDSTUFF_LIB_A): $(patsubst $(LIB_SRC)/%.c,$(LIB)/%.o,$(wildcard $(LIB_SRC)/*.c)) $(wildcard $(INCLUDE)/*.h) Makefile | build/SUBMODULES_INITED $(LIB)
	$(AR) rcs $(LIB)/$(SHAREDSTUFF_LIB_A) $(filter %.o,$^)


# COMPILATION + LINKING PHASE
#
# compile object files
# depends on respective source file and a header with the same name (if it exists)
.SECONDEXPANSION:
$(OBJS): %.o: %.c $$(wildcard $$(dir $$@)/*.h) Makefile | build/SUBMODULES_INITED
	$(CC) $(CFLAGS) -c -o $@ $<
#
# link executables
# depends on all respective object files and the lib/*.o object files
.SECONDEXPANSION:
$(EXES): $$(patsubst %.c,%.o,$$(wildcard $$(notdir $$@)/*.c)) $$(LIB)/$$(SHAREDSTUFF_LIB_A) | build/SUBMODULES_INITED
	$(CC) $(LDFLAGS) -o $@ $<
#

# EXPORT RULE
INSTALL_PREFIX := $(HOME)/.local
BINDIR := $(INSTALL_PREFIX)/bin
EXPORT_EXES := $(patsubst build/%,$(BINDIR)/%,$(EXES))

$(EXPORT_EXES): $(BINDIR)/%: build/%
	cp -i $< $@

export: $(LIB)/$(SHAREDSTUFF_LIB_A) $(EXPORT_EXES)

# OTHER RULES
clean:
	rm -f $(patsubst lib/%,,$(wildcard */*.o)) build/*
clean_exported: clean
	rm -f $(EXPORT_EXES)
clean_libs: $(LIB)
	rm -f $(LIB)/*.o $(LIB)/$(SHAREDSTUFF_LIB_A)
clean_all: clean clean_exported clean_libs
	git submodule deinit --all -f
	rm -f build/SUBMODULES_INITED

build:
	mkdir -p build
$(LIB):
	mkdir -p $(LIB)

build/SUBMODULES_INITED: | build
	git submodule update --init
	touch build/SUBMODULES_INITED
