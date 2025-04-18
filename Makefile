CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wpedantic -Wconversion -Wshadow -Wcast-qual \
         -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wstrict-aliasing=2 \
         -Wnull-dereference -Wformat=2 -Wuninitialized -Wpointer-arith \
         -Wunused -Wunused-parameter -Wwrite-strings -std=c11 -I./src
LDFLAGS = -lm

# Debug build flags
DEBUG_CFLAGS = -g -O0 -DDEBUG
# Release build flags
RELEASE_CFLAGS = -O3 -DNDEBUG

# Source files
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c,bin/%.o,$(SRC))

# Test source files
TEST_SRC = $(wildcard tests/*.c)
TEST_OBJ = $(patsubst tests/%.c,bin/tests/%.o,$(TEST_SRC))
TEST_BIN = $(patsubst tests/%.c,bin/tests/%,$(TEST_SRC))

# Main target
all: debug

# Debug target
debug: CFLAGS += $(DEBUG_CFLAGS)
debug: lib

# Release target
release: CFLAGS += $(RELEASE_CFLAGS)
release: lib

# Create bin directories
bin bin/tests:
	mkdir -p $@

# Library target
lib: bin $(OBJ)
	ar rcs bin/libagerun.a $(OBJ)

# Example application - build and run
example: lib bin
	$(CC) $(CFLAGS) -o bin/example examples/example.c bin/libagerun.a $(LDFLAGS)
	./bin/example

# Build and run tests
test: bin/tests $(TEST_BIN)
	@for test in $(TEST_BIN); do \
		echo "Running $$test"; \
		./$$test; \
	done

# Individual test binaries
bin/tests/%: bin/tests/%.o lib
	$(CC) $(CFLAGS) -o $@ $< bin/libagerun.a $(LDFLAGS)

# Compile source files
bin/%.o: src/%.c | bin
	$(CC) $(CFLAGS) -c $< -o $@

bin/tests/%.o: tests/%.c | bin/tests
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -rf bin

.PHONY: all debug release clean test