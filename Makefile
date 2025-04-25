CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wpedantic -Wconversion -Wshadow -Wcast-qual \
         -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wstrict-aliasing=2 \
         -Wnull-dereference -Wformat=2 -Wuninitialized -Wpointer-arith \
         -Wunused -Wunused-parameter -Wwrite-strings -std=c11 -I./modules
LDFLAGS = -lm

# Debug build flags
DEBUG_CFLAGS = -g -O0 -DDEBUG
# Release build flags
RELEASE_CFLAGS = -O3 -DNDEBUG

# Source files (excluding test files)
SRC = $(filter-out modules/*_tests.c,$(wildcard modules/*.c))
OBJ = $(patsubst modules/%.c,bin/%.o,$(SRC))

# Test source files
TEST_SRC = $(wildcard modules/*_tests.c)
TEST_OBJ = $(patsubst modules/%.c,bin/%.o,$(TEST_SRC))
TEST_BIN = $(patsubst modules/%_tests.c,bin/%_tests,$(TEST_SRC))

# Main target
all: debug executable

# Debug target
debug: CFLAGS += $(DEBUG_CFLAGS)
debug: lib

# Release target
release: CFLAGS += $(RELEASE_CFLAGS)
release: lib

# Create bin directory
bin:
	mkdir -p $@

# Library target
lib: bin $(OBJ)
	ar rcs bin/libagerun.a $(OBJ)
	
# Test library target (including all obj files but not test files)
test_lib: bin $(OBJ) $(TEST_OBJ)
	ar rcs bin/libagerun.a $(OBJ)

# Executable application - build only
executable: lib bin
	$(CC) $(CFLAGS) -o bin/agerun modules/agerun_executable.c bin/libagerun.a $(LDFLAGS)

# Run the executable
run: executable
	cd bin && ./agerun

# Define test executables without bin/ prefix for use in the bin directory
TEST_BIN_NAMES = $(notdir $(TEST_BIN))

# Build and run tests
test: bin $(TEST_BIN)
	@cd bin && rm -f *.agerun && for test in $(TEST_BIN_NAMES); do \
		echo "Running $$test"; \
		./$$test || echo "ERROR: Test $$test failed with status $$?"; \
	done

# Individual test binaries
bin/%_tests: bin/%_tests.o test_lib
	$(CC) $(CFLAGS) -o $@ $< bin/libagerun.a $(LDFLAGS)

# Compile source files
bin/%.o: modules/%.c | bin
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -rf bin

.PHONY: all debug release clean test example run-example