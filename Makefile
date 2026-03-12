INCLUDE := include
LIB := lib
LIB_SRC := lib_src

AUT_LIB := autlib
AUT_LIB_A := lib$(AUT_LIB).a
CFLAGS := -Wall -Wextra -Wpedantic -I$(INCLUDE) $(EXTRA_CFLAGS)
LDFLAGS := $(EXTRA_LDFLAGS) -L$(LIB) -l$(AUT_LIB)
CC := cc
AR := ar
SRC_FILES := $(wildcard */*.c)

DIRS := $(sort $(dir $(SRC_FILES)))
DIRS := $(patsubst %/,%,$(DIRS))
DIRS := $(filter-out $(LIB_SRC) %nope,$(DIRS))

OBJS := $(foreach d,$(DIRS),$(patsubst $(d)/%.c,$(d)/%.o,$(wildcard $(d)/*.c)))

EXES := $(foreach d,$(DIRS),build/$(notdir $(d)))

.PHONY: all clean_objs clean_installed clean_libs clean_all install
all: build/SUBMODULES_INITED $(LIB) $(LIB)/$(AUT_LIB_A) build $(EXES)

$(LIB)/%.o: $(LIB_SRC)/%.c $(wildcard $(INCLUDE)/*.h) Makefile | $(LIB) build/SUBMODULES_INITED
	$(CC) -o $@ -c $< $(CFLAGS)

$(LIB)/$(AUT_LIB_A): $(patsubst $(LIB_SRC)/%.c,$(LIB)/%.o,$(wildcard $(LIB_SRC)/*.c)) $(wildcard $(INCLUDE)/*.h) Makefile | build/SUBMODULES_INITED $(LIB)
	$(AR) rcs $(LIB)/$(AUT_LIB_A) $(filter %.o,$^)


# COMPILATION + LINKING PHASE
#
# compile object files
# depends on respective source file and a header with the same name (if it exists)
.SECONDEXPANSION:
$(OBJS): %.o: %.c $$(wildcard $$(dir $$@)/*.h) Makefile | build/SUBMODULES_INITED
	$(CC) $(CFLAGS) -c -o $@ $< -DAPPLE_AUT_BINDIR='"$(TOOL_BINDIR)"'
#
# link executables
# depends on all respective object files and the lib/*.o object files
.SECONDEXPANSION:
$(EXES): $$(patsubst %.c,%.o,$$(wildcard $$(notdir $$@)/*.c)) $$(LIB)/$$(AUT_LIB_A) | build/SUBMODULES_INITED
	$(CC) $(LDFLAGS) -o $@ $<
#

# INSTALL RULE
INSTALL_PREFIX := $(HOME)/.local
GENERAL_BINDIR := $(INSTALL_PREFIX)/bin
INCLUDE_INSTALLDIR := $(INSTALL_PREFIX)/include
LIB_INSTALLDIR := $(INSTALL_PREFIX)/lib
TOOL_BINDIR = $(GENERAL_BINDIR)/apple-aut-bin
APPLE_AUT_TOOL_BIN := $(GENERAL_BINDIR)/apple-aut
INSTALL_EXES := $(patsubst build/%,$(TOOL_BINDIR)/%,$(EXES))

$(APPLE_AUT_TOOL_BIN): build/apple-aut
	cp -i build/apple-aut $(APPLE_AUT_TOOL_BIN)

$(INSTALL_EXES): $(TOOL_BINDIR)/%: build/% $(TOOL_BINDIR)
	cp $< $@

$(LIB_INSTALLDIR)/$(AUT_LIB_A): $(LIB)/$(AUT_LIB_A) $(LIB_INSTALLDIR)
	cp $(LIB)/$(AUT_LIB_A) $(LIB_INSTALLDIR)/$(AUT_LIB_A)

.PHONY: install_include
install_include: $(INCLUDE_INSTALLDIR)
	rm -rf $(INCLUDE_INSTALLDIR)
	mkdir -p $(INCLUDE_INSTALLDIR)
	cp -r $(INCLUDE)/autlib $(INCLUDE_INSTALLDIR)
	
	
install: $(TOOL_BINDIR) $(LIB_INSTALLDIR)/$(AUT_LIB_A) $(APPLE_AUT_TOOL_BIN) $(INSTALL_EXES) install_include
	rm $(TOOL_BINDIR)/apple-aut

# OTHER RULES
.PHONY: clean clean_installed clean_libs clean_installed_include clean_all
clean_objs:
	rm -f $(filter-out lib/%,$(wildcard */*.o)) build/*
clean_installed: clean_installed_include clean_installed_libs
	rm -f $(INSTALL_EXES) $(APPLE_AUT_TOOL_BIN)
	rm -rf $(TOOL_BINDIR)
clean_libs: $(LIB) clean_installed_libs
	rm -f $(LIB)/*.o $(LIB)/$(AUT_LIB_A)
clean_installed_include:
	rm -rf $(INCLUDE_INSTALLDIR)
clean_installed_libs:
	rm -f $(LIB_INSTALLDIR)/$(AUT_LIB_A)
clean_all: clean_objs clean_installed clean_libs clean_installed_include
	git submodule deinit --all -f
	rm -f build/SUBMODULES_INITED

build:
	mkdir -p build
$(LIB):
	mkdir -p $(LIB)
$(LIB_INSTALLDIR):
	mkdir -p $(LIB_INSTALLDIR)
$(TOOL_BINDIR):
	mkdir -p $(TOOL_BINDIR)
$(INCLUDE_INSTALLDIR): 

build/SUBMODULES_INITED: | build
	git submodule update --init
	touch build/SUBMODULES_INITED
