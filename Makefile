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
# Address Sanitizer flags
ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer
# Clang Static Analyzer command
SCAN_BUILD = PATH="/opt/homebrew/opt/llvm/bin:$$PATH" scan-build -o bin/scan-build-results
# Analyzer flags directly for Clang (used when scan-build is not available)
ANALYZER_FLAGS = -Xclang -analyze -Xclang -analyzer-checker=core -Xclang -analyzer-checker=unix \
                 -Xclang -analyzer-checker=deadcode -Xclang -analyzer-checker=security

# Source files (excluding test files)
SRC = $(filter-out modules/*_tests.c,$(wildcard modules/*.c))
OBJ = $(patsubst modules/%.c,bin/obj/%.o,$(SRC))

# Test source files
TEST_SRC = $(wildcard modules/*_tests.c)
TEST_OBJ = $(patsubst modules/%.c,bin/obj/%.o,$(TEST_SRC))
TEST_BIN = $(patsubst modules/%_tests.c,bin/%_tests,$(TEST_SRC))

# Main target
all: debug executable

# Debug target
debug: CFLAGS += $(DEBUG_CFLAGS)
debug: lib

# Release target
release: CFLAGS += $(RELEASE_CFLAGS)
release: lib

# Sanitize target with Address Sanitizer
sanitize: CFLAGS += $(DEBUG_CFLAGS) $(ASAN_FLAGS)
sanitize: LDFLAGS += -fsanitize=address
sanitize: lib

# Create bin and bin/obj directories
bin:
	mkdir -p $@/obj

# Library target
lib: bin $(OBJ)
	ar rcs bin/libagerun.a $(OBJ)
	
# Test library target (including all obj files but not test files)
test_lib: CFLAGS += $(DEBUG_CFLAGS)
test_lib: bin $(OBJ) $(TEST_OBJ)
	ar rcs bin/libagerun.a $(OBJ)

# Executable application - build only (always in debug mode)
executable: CFLAGS += $(DEBUG_CFLAGS)
executable: lib bin
	$(CC) $(CFLAGS) -o bin/agerun modules/agerun_executable.c bin/libagerun.a $(LDFLAGS)

# Executable application with Address Sanitizer - build only
executable-sanitize: sanitize bin
	$(CC) $(CFLAGS) -o bin/agerun modules/agerun_executable.c bin/libagerun.a $(LDFLAGS)

# Run the executable (always in debug mode)
run: executable
	cd bin && ./agerun

# Run the executable with Address Sanitizer
run-sanitize: executable-sanitize
	cd bin && ASAN_OPTIONS=detect_leaks=1 ./agerun

# Define test executables without bin/ prefix for use in the bin directory
TEST_BIN_NAMES = $(notdir $(TEST_BIN))

# Build and run tests (always in debug mode)
test: CFLAGS += $(DEBUG_CFLAGS)
test: bin $(TEST_BIN)
	@cd bin && rm -f *.agerun && for test in $(TEST_BIN_NAMES); do \
		echo "Running $$test"; \
		./$$test || echo "ERROR: Test $$test failed with status $$?"; \
	done

# Build and run tests with Address Sanitizer
test-sanitize: sanitize bin $(TEST_BIN)
	@cd bin && rm -f *.agerun && for test in $(TEST_BIN_NAMES); do \
		echo "Running $$test with Address Sanitizer"; \
		ASAN_OPTIONS=detect_leaks=1 ./$$test || echo "ERROR: Test $$test failed with status $$?"; \
	done

# Individual test binaries
bin/%_tests: bin/obj/%_tests.o test_lib
	$(CC) $(CFLAGS) -o $@ $< bin/libagerun.a $(LDFLAGS)

# Compile source files
bin/obj/%.o: modules/%.c | bin
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -rf bin

# Static analysis target
analyze:
	@if command -v /opt/homebrew/opt/llvm/bin/scan-build >/dev/null 2>&1 || command -v scan-build >/dev/null 2>&1; then \
		mkdir -p bin/scan-build-results; \
		$(SCAN_BUILD) $(MAKE) clean lib; \
		echo "Static analysis results are available in bin/scan-build-results"; \
	else \
		echo "scan-build not found, using clang analyzer directly"; \
		for file in $(SRC); do \
			echo "Analyzing $$file..."; \
			$(CC) $(CFLAGS) $(ANALYZER_FLAGS) -c -I./modules $$file; \
		done; \
		echo "Analysis complete. Look for analyzer warnings above."; \
	fi

# Static analysis for tests
analyze-tests:
	@if command -v /opt/homebrew/opt/llvm/bin/scan-build >/dev/null 2>&1 || command -v scan-build >/dev/null 2>&1; then \
		mkdir -p bin/scan-build-results; \
		$(SCAN_BUILD) $(MAKE) clean test_lib; \
		echo "Static analysis results are available in bin/scan-build-results"; \
	else \
		echo "scan-build not found, using clang analyzer directly"; \
		for file in $(SRC) $(TEST_SRC); do \
			echo "Analyzing $$file..."; \
			$(CC) $(CFLAGS) $(ANALYZER_FLAGS) -c -I./modules $$file; \
		done; \
		echo "Analysis complete. Look for analyzer warnings above."; \
	fi

.PHONY: all debug release sanitize clean test test-sanitize executable executable-sanitize run run-sanitize analyze analyze-tests

# Helper to install scan-build
install-scan-build:
	@echo "Installing scan-build via Homebrew..."
	@if command -v brew >/dev/null 2>&1; then \
		brew install llvm; \
		echo "LLVM and scan-build installed. Run 'export PATH="/opt/homebrew/opt/llvm/bin:$$PATH"' to use it."; \
	else \
		echo "Homebrew not found. Please install LLVM and scan-build manually."; \
	fi