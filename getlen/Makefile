SOURCE_FILE := $(wildcard *.c)
OUT_FILE := $(patsubst %.c, %, $(SOURCE_FILE))

$(OUT_FILE): $(SOURCE_FILE)
	cc -o $(OUT_FILE) $(SOURCE_FILE)

.PHONY: export
export: $(OUT_FILE)
	cp $(OUT_FILE) $(HOME)/.local/bin
