INCLUDE := include
LIB := lib
LIB_SRC := lib_src
BUILD := build

AUT_LIB := autlib
AUT_LIB_A := lib$(AUT_LIB).a
CFLAGS := -Wall -Wextra -Wpedantic -D_POSIX_C_SOURCE=200809L -D_ISO2X_SOURCE=1 -D_DEFAULT_SOURCE -I$(INCLUDE) $(EXTRA_CFLAGS)
LDFLAGS := $(EXTRA_LDFLAGS) -L$(LIB) -l$(AUT_LIB)
CC := cc
AR := ar
SRC_FILES := $(wildcard */*.c)

DIRS := $(sort $(dir $(SRC_FILES)))
DIRS := $(patsubst %/,%,$(DIRS))
DIRS := $(filter-out $(LIB_SRC) %nope,$(DIRS))

OBJS := $(foreach d,$(DIRS),$(patsubst $(d)/%.c,$(d)/%.o,$(wildcard $(d)/*.c)))

EXES := $(foreach d,$(DIRS),$(BUILD)/$(notdir $(d)))

.PHONY: all clean_objs clean_installed clean_libs clean_all install
all: $(BUILD)/SUBMODULES_INITED $(LIB) $(LIB)/$(AUT_LIB_A) $(BUILD) $(EXES)

$(LIB)/%.o: $(LIB_SRC)/%.c $(wildcard $(INCLUDE)/*.h) Makefile | $(LIB) $(BUILD)/SUBMODULES_INITED
	$(CC) -o $@ -c $< $(CFLAGS)

$(LIB)/$(AUT_LIB_A): $(patsubst $(LIB_SRC)/%.c,$(LIB)/%.o,$(wildcard $(LIB_SRC)/*.c)) $(wildcard $(INCLUDE)/*.h) Makefile | $(BUILD)/SUBMODULES_INITED $(LIB)
	$(AR) rcs $(LIB)/$(AUT_LIB_A) $(filter %.o,$^)


# COMPILATION + LINKING PHASE
#
# compile object files
# depends on respective source file and a header with the same name (if it exists)
.SECONDEXPANSION:
$(OBJS): %.o: %.c $$(wildcard $$(dir $$@)/*.h) Makefile | $(BUILD)/SUBMODULES_INITED
	$(CC) $(CFLAGS) -c -o $@ $< -DAPPLE_AUT_BINDIR='"$(TOOL_BINDIR)"'
#
# link executables
# depends on all respective object files and the lib/*.o object files
.SECONDEXPANSION:
$(EXES): $$(patsubst %.c,%.o,$$(wildcard $$(notdir $$@)/*.c)) $$(LIB)/$$(AUT_LIB_A) | $(BUILD)/SUBMODULES_INITED
	$(CC) -o $@ $< $(LDFLAGS)

# INSTALL RULE
INSTALL_PREFIX := $(HOME)/.local
GENERAL_BINDIR := $(INSTALL_PREFIX)/bin
INCLUDE_INSTALLDIR := $(INSTALL_PREFIX)/include
AUTLIB_INCLUDE_INSTALLDIR := $(INCLUDE_INSTALLDIR)/autlib
LIB_INSTALLDIR := $(INSTALL_PREFIX)/lib
TOOL_BINDIR = $(GENERAL_BINDIR)/apple-aut-bin
APPLE_AUT_TOOL_BIN := $(GENERAL_BINDIR)/apple-aut
INSTALL_EXES := $(patsubst $(BUILD)/%,$(TOOL_BINDIR)/%,$(EXES))
SHELL_COMP_INSTALLDIR := $(HOME)/.local/share/shell-completions

DIRS_TO_BE_GENERATED := $(BUILD) $(LIB) $(LIB_INSTALLDIR) $(TOOL_BINDIR) $(INCLUDE_INSTALLDIR) $(SHELL_COMP_INSTALLDIR)

$(APPLE_AUT_TOOL_BIN): $(BUILD)/apple-aut
	cp $(BUILD)/apple-aut $(APPLE_AUT_TOOL_BIN)

$(INSTALL_EXES): $(TOOL_BINDIR)/%: $(BUILD)/% $(TOOL_BINDIR)
	cp $< $@

$(LIB_INSTALLDIR)/$(AUT_LIB_A): $(LIB)/$(AUT_LIB_A) $(LIB_INSTALLDIR)
	cp $(LIB)/$(AUT_LIB_A) $(LIB_INSTALLDIR)/$(AUT_LIB_A)

.PHONY: install_include
install_include: $(INCLUDE_INSTALLDIR)
	rm -rf $(AUTLIB_INCLUDE_INSTALLDIR)
	cp -r $(INCLUDE)/autlib $(INCLUDE_INSTALLDIR)

.PHONY: install_shell_comp
install_shell_comp: $(SHELL_COMP_INSTALLDIR)
	cp ./apple-aut-shell-completions.sh $(SHELL_COMP_INSTALLDIR)/apple-aut.sh
	
install: $(TOOL_BINDIR) $(LIB_INSTALLDIR)/$(AUT_LIB_A) $(APPLE_AUT_TOOL_BIN) $(INSTALL_EXES) install_include
	rm $(TOOL_BINDIR)/apple-aut

# CLEAN RULES
.PHONY: clean_local clean_objs clean_libs clean_submodules
clean_local: clean_submodules clean_objs clean_libs
clean_objs:
	rm -f $(filter-out lib/%,$(wildcard */*.o)) $(BUILD)/*
clean_libs: $(LIB)
	rm -f $(LIB)/*.o $(LIB)/$(AUT_LIB_A)
clean_submodules:
	git submodule deinit --all -f
	rm -f $(BUILD)/SUBMODULES_INITED

.PHONY: clean_installed clean_installed_include clean_installed_libs
clean_installed: clean_installed_include clean_installed_libs
	rm -f $(INSTALL_EXES) $(APPLE_AUT_TOOL_BIN)
	rm -rf $(TOOL_BINDIR)
clean_installed_include:
	rm -rf $(AUTLIB_INCLUDE_INSTALLDIR)
clean_installed_libs:
	rm -f $(LIB_INSTALLDIR)/$(AUT_LIB_A)

.PHONY: clean_all
clean_all: clean_local clean_installed


# OTHER RULES

$(DIRS_TO_BE_GENERATED):
	mkdir -p $@

$(BUILD)/SUBMODULES_INITED: | $(BUILD)
	git submodule update --init
	touch $(BUILD)/SUBMODULES_INITED
