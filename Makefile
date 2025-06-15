# Detect OS for sanitizer compiler selection
UNAME_S := $(shell uname -s)

# Default compiler
CC = gcc-13

# Sanitizer compiler selection based on OS
ifeq ($(UNAME_S),Darwin)
    SANITIZER_CC = clang
else
    SANITIZER_CC = $(CC)
endif

CFLAGS = -Wall -Wextra -Werror -Wpedantic -Wconversion -Wshadow -Wcast-qual \
         -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wstrict-aliasing=2 \
         -Wnull-dereference -Wformat=2 -Wuninitialized -Wpointer-arith \
         -Wunused -Wunused-parameter -Wwrite-strings -std=c11 -I./modules -D_GNU_SOURCE
LDFLAGS = -lm

# Debug build flags
DEBUG_CFLAGS = -g -O0 -DDEBUG
# Release build flags
RELEASE_CFLAGS = -O3 -DNDEBUG
# Address Sanitizer flags
ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer
# Clang Static Analyzer command
ifeq ($(UNAME_S),Darwin)
    SCAN_BUILD = PATH="/opt/homebrew/opt/llvm/bin:$$PATH" scan-build -o bin/scan-build-results
else
    SCAN_BUILD = scan-build -o bin/scan-build-results
endif

# Source files (excluding test files)
ALL_C_FILES = $(wildcard modules/*.c)
SRC = $(filter-out %_tests.c %_fixture_tests.c,$(ALL_C_FILES))
OBJ = $(patsubst modules/%.c,bin/obj/%.o,$(SRC))

# Test source files
TEST_SRC = $(wildcard modules/*_tests.c)
TEST_OBJ = $(patsubst modules/%.c,bin/obj/%.o,$(TEST_SRC))
TEST_BIN = $(patsubst modules/%_tests.c,bin/%_tests,$(TEST_SRC))

# Method test source files
METHOD_TEST_SRC = $(wildcard methods/*_tests.c)
METHOD_TEST_OBJ = $(patsubst methods/%.c,bin/obj/%.o,$(METHOD_TEST_SRC))
METHOD_TEST_BIN = $(patsubst methods/%_tests.c,bin/%_tests,$(METHOD_TEST_SRC))

# Main target
all: debug executable

# Debug target
debug: CFLAGS += $(DEBUG_CFLAGS)
debug: lib

# Release target
release: CFLAGS += $(RELEASE_CFLAGS)
release: lib

# Sanitize target with Address Sanitizer
sanitize: CC = $(SANITIZER_CC)
sanitize: CFLAGS += $(DEBUG_CFLAGS) $(ASAN_FLAGS)
sanitize: LDFLAGS += $(ASAN_FLAGS)
sanitize: lib

# Create bin and bin/obj directories
bin:
	mkdir -p $@/obj

# Library target
lib: bin $(OBJ)
	ar rcs bin/libagerun.a $(OBJ)
	
# Test library target (including all obj files but not test files)
test_lib: CFLAGS += $(DEBUG_CFLAGS)
test_lib: bin $(OBJ) $(TEST_OBJ) $(METHOD_TEST_OBJ)
	ar rcs bin/libagerun.a $(OBJ)

# Executable application - build only
executable: lib bin
	$(CC) $(CFLAGS) -o bin/agerun modules/agerun_executable.c bin/libagerun.a $(LDFLAGS)

# Executable application with Address Sanitizer - build only
executable-sanitize: clean
	$(MAKE) sanitize
	$(SANITIZER_CC) $(CFLAGS) $(DEBUG_CFLAGS) $(ASAN_FLAGS) -o bin/agerun modules/agerun_executable.c bin/libagerun.a $(LDFLAGS) $(ASAN_FLAGS)

# Run the executable (always in debug mode)
run: debug executable
	cd bin && AGERUN_MEMORY_REPORT="memory_report_agerun.log" ./agerun

# Run the executable with Address Sanitizer
run-sanitize: executable-sanitize
	cd bin && AGERUN_MEMORY_REPORT="memory_report_agerun.log" ./agerun

# Define test executables without bin/ prefix for use in the bin directory
TEST_BIN_NAMES = $(notdir $(TEST_BIN))
METHOD_TEST_BIN_NAMES = $(notdir $(METHOD_TEST_BIN))
ALL_TEST_BIN_NAMES = $(TEST_BIN_NAMES) $(METHOD_TEST_BIN_NAMES)

# Build and run tests (always in debug mode)
test: clean debug
	$(MAKE) test_lib
	$(MAKE) $(TEST_BIN) $(METHOD_TEST_BIN)
	@cd bin && for test in $(ALL_TEST_BIN_NAMES); do \
		rm -f *.agerun; \
		echo "Running $$test"; \
		AGERUN_MEMORY_REPORT="memory_report_$$test.log" ./$$test || echo "ERROR: Test $$test failed with status $$?"; \
	done

# Build and run tests with Address Sanitizer
test-sanitize: clean
	$(MAKE) sanitize
	$(MAKE) test_lib CC="$(SANITIZER_CC)" CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS) $(ASAN_FLAGS)" LDFLAGS="$(LDFLAGS) $(ASAN_FLAGS)"
	$(MAKE) $(TEST_BIN) $(METHOD_TEST_BIN) CC="$(SANITIZER_CC)" CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS) $(ASAN_FLAGS)" LDFLAGS="$(LDFLAGS) $(ASAN_FLAGS)"
	@cd bin && for test in $(ALL_TEST_BIN_NAMES); do \
		rm -f *.agerun; \
		echo "Running $$test with Address Sanitizer"; \
		AGERUN_MEMORY_REPORT="memory_report_$$test.log" ./$$test || echo "ERROR: Test $$test failed with status $$?"; \
	done

# Individual test binaries
bin/%_tests: bin/obj/%_tests.o test_lib
	$(CC) $(CFLAGS) -o $@ $< bin/libagerun.a $(LDFLAGS)
	@echo "Running test: $@"
	@cd bin && AGERUN_MEMORY_REPORT="memory_report_$(notdir $@).log" ./$(notdir $@)

# Compile source files (always with debug for test files)
bin/obj/%_tests.o: modules/%_tests.c | bin
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -c $< -o $@

# Compile method test files (always with debug)
bin/obj/%_tests.o: methods/%_tests.c | bin
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -c $< -o $@

# Compile regular source files
bin/obj/%.o: modules/%.c | bin
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -rf bin

# Static analysis target
analyze: install-scan-build
	@if command -v /opt/homebrew/opt/llvm/bin/scan-build >/dev/null 2>&1 || command -v scan-build >/dev/null 2>&1; then \
		mkdir -p bin/scan-build-results bin/obj; \
		rm -rf bin/scan-build-results/*; \
		echo "Running scan-build on source files with $(CC)..."; \
		total_bugs=0; \
		rm -f bin/scan-build-analyze.log; \
		for file in $(SRC); do \
			echo "Analyzing $$file..."; \
			$(SCAN_BUILD) --status-bugs --use-cc=$(CC) $(CC) -c -I./modules $$file -o bin/obj/$$(basename $$file .c).o 2>&1 | tee bin/scan-build-temp.log; \
			if grep -q "scan-build: [0-9]* bug" bin/scan-build-temp.log && ! grep -q "scan-build: 0 bugs found" bin/scan-build-temp.log; then \
				file_bugs=$$(grep "scan-build: [0-9]* bug" bin/scan-build-temp.log | tail -1 | sed 's/.*scan-build: \([0-9]*\) bug.*/\1/'); \
				echo "  ✗ $$file_bugs bugs found in $$file"; \
				total_bugs=$$((total_bugs + file_bugs)); \
			fi; \
			cat bin/scan-build-temp.log >> bin/scan-build-analyze.log; \
		done; \
		if [ $$total_bugs -gt 0 ]; then \
			echo "Static analysis FAILED: $$total_bugs total bugs found"; \
			echo "View detailed reports in: bin/scan-build-results/"; \
			exit 1; \
		else \
			echo "Static analysis passed: no bugs found"; \
		fi; \
	else \
		echo "ERROR: scan-build not found. Static analysis requires Clang Static Analyzer."; \
		echo "On macOS: brew install llvm"; \
		echo "On Ubuntu: sudo apt-get install clang-tools"; \
		echo "On other systems: install clang-tools or llvm package"; \
		exit 1; \
	fi

# Static analysis for tests
analyze-tests: install-scan-build
	@if command -v /opt/homebrew/opt/llvm/bin/scan-build >/dev/null 2>&1 || command -v scan-build >/dev/null 2>&1; then \
		mkdir -p bin/scan-build-results bin/obj; \
		rm -rf bin/scan-build-results/*; \
		echo "Running scan-build on test files with $(CC)..."; \
		total_bugs=0; \
		rm -f bin/scan-build-analyze-tests.log; \
		for file in $(SRC) $(TEST_SRC) $(METHOD_TEST_SRC); do \
			echo "Analyzing $$file..."; \
			$(SCAN_BUILD) --status-bugs --use-cc=$(CC) $(CC) -c -I./modules $$file -o bin/obj/$$(basename $$file .c).o 2>&1 | tee bin/scan-build-temp-tests.log; \
			if grep -q "scan-build: [0-9]* bug" bin/scan-build-temp-tests.log && ! grep -q "scan-build: 0 bugs found" bin/scan-build-temp-tests.log; then \
				file_bugs=$$(grep "scan-build: [0-9]* bug" bin/scan-build-temp-tests.log | tail -1 | sed 's/.*scan-build: \([0-9]*\) bug.*/\1/'); \
				echo "  ✗ $$file_bugs bugs found in $$file"; \
				total_bugs=$$((total_bugs + file_bugs)); \
			fi; \
			cat bin/scan-build-temp-tests.log >> bin/scan-build-analyze-tests.log; \
		done; \
		if [ $$total_bugs -gt 0 ]; then \
			echo "Static analysis FAILED: $$total_bugs total bugs found in tests"; \
			echo "View detailed reports in: bin/scan-build-results/"; \
			exit 1; \
		else \
			echo "Static analysis passed: no bugs found in tests"; \
		fi; \
	else \
		echo "ERROR: scan-build not found. Static analysis requires Clang Static Analyzer."; \
		echo "On macOS: brew install llvm"; \
		echo "On Ubuntu: sudo apt-get install clang-tools"; \
		echo "On other systems: install clang-tools or llvm package"; \
		exit 1; \
	fi

.PHONY: all debug release sanitize clean test test-sanitize executable executable-sanitize run run-sanitize analyze analyze-tests

# Debug targets
print-src:
	@echo "SRC = $(SRC)"
	@echo "Number of SRC files: $(words $(SRC))"
print-obj:
	@echo "OBJ = $(OBJ)"
	@echo "Number of OBJ files: $(words $(OBJ))"

# Helper to install scan-build based on OS
install-scan-build:
	@if command -v /opt/homebrew/opt/llvm/bin/scan-build >/dev/null 2>&1 || command -v scan-build >/dev/null 2>&1; then \
		echo "scan-build is already installed."; \
	else \
		echo "Installing clang-tools for static analysis..."; \
		if [ "$(UNAME_S)" = "Darwin" ]; then \
			if command -v brew >/dev/null 2>&1; then \
				echo "Updating Homebrew..."; \
				brew update || { echo "Failed to update Homebrew"; exit 1; }; \
				echo "Installing LLVM via Homebrew..."; \
				brew install llvm || { echo "Failed to install LLVM"; exit 1; }; \
				echo ""; \
				echo "LLVM installed successfully!"; \
				echo "To use scan-build, add LLVM to your PATH:"; \
				echo "  export PATH=\"/opt/homebrew/opt/llvm/bin:\$$PATH\""; \
				echo ""; \
				echo "After adding to PATH, run 'make analyze' again."; \
			else \
				echo "Homebrew not found. Please install Homebrew first:"; \
				echo "  /bin/bash -c \"\$$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""; \
				exit 1; \
			fi; \
	elif [ -f /etc/os-release ] && (grep -q "ubuntu\|debian" /etc/os-release 2>/dev/null); then \
		if command -v apt-get >/dev/null 2>&1; then \
			echo "Installing clang-tools via apt-get..."; \
			echo "This requires sudo access."; \
			sudo apt-get update && sudo apt-get install -y clang-tools || { echo "Failed to install clang-tools"; exit 1; }; \
			echo ""; \
			echo "clang-tools installed successfully!"; \
			echo "scan-build should now be available in your PATH."; \
		else \
			echo "apt-get not found on Ubuntu/Debian system."; \
			exit 1; \
		fi; \
	else \
		echo "Unable to auto-install. Please install manually:"; \
		echo "- On macOS: brew install llvm"; \
		echo "- On Ubuntu/Debian: sudo apt-get install clang-tools"; \
		echo "- On other systems: install clang-tools or llvm package"; \
		exit 1; \
	fi; \
fi
