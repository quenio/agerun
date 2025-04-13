CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c11 -I./include
LDFLAGS = -lm

# Debug build flags
DEBUG_CFLAGS = -g -O0 -DDEBUG
# Release build flags
RELEASE_CFLAGS = -O3 -DNDEBUG

# Source files
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

# Test source files
TEST_SRC = $(wildcard tests/*.c)
TEST_OBJ = $(TEST_SRC:.c=.o)
TEST_BIN = $(TEST_SRC:.c=)

# Main target
all: debug

# Debug target
debug: CFLAGS += $(DEBUG_CFLAGS)
debug: lib

# Release target
release: CFLAGS += $(RELEASE_CFLAGS)
release: lib

# Library target
lib: $(OBJ)
	ar rcs libagerun.a $(OBJ)

# Example application
example: lib
	$(CC) $(CFLAGS) -o example examples/example.c libagerun.a $(LDFLAGS)

# Build and run tests
test: $(TEST_BIN)
	@for test in $(TEST_BIN); do \
		echo "Running $$test"; \
		./$$test; \
	done

# Individual test binaries
tests/%: tests/%.o lib
	$(CC) $(CFLAGS) -o $@ $< libagerun.a $(LDFLAGS)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(OBJ) $(TEST_OBJ) $(TEST_BIN) libagerun.a example

.PHONY: all debug release clean test