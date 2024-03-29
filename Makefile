export CC

CFLAGS=-W -Wall -Wextra -Werror -std=c11
MAKE_ARGS=
LIBS=hashtable/hashtable.a vector/vector.a
TARGET=interpreter

MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
MKFILE_DIR := $(dir $(MKFILE_PATH))
LIBS_FULLPATH := $(addprefix lib/, $(LIBS))

SRCS := $(wildcard source/*.c)
HDRS := $(wildcard source/*.h)
OBJS := $(patsubst source/%.c,bin/%.o,$(SRCS))

ifdef WINDOWS
CFLAGS += -DWINDOWS
endif

.PHONY: all
all: debug

.PHONY: debug
debug: CFLAGS += -g -DDEBUG
debug: MAKE_ARGS += debug
debug: libs
debug: $(TARGET)

.PHONY: release
release: CFLAGS += -O2 -Wno-array-bounds
release: libs
release: $(TARGET)

.PHONY: libs
libs:
	$(MAKE) -C lib $(MAKE_ARGS)

$(TARGET): $(OBJS) $(HDRS) Makefile
	$(CC) $(CFLAGS) $(OBJS) $(LIBS_FULLPATH) -o $(MKFILE_DIR)$(TARGET)

bin/%.o: source/%.c $(HDRS) Makefile | bin
	$(CC) $(CFLAGS) -c $< -o $@ -I lib

bin:
	mkdir -p $@

.PHONY: clean
clean:
	$(MAKE) -C lib clean
	rm -rf bin
	rm -f $(TARGET)
	rm -f $(TARGET).exe
