# Default target
.DEFAULT_GOAL := help

# Help target - displays available make targets
help:
	@echo "AgeRun Makefile Targets:"
	@echo ""
	@echo "Build targets:"
	@echo "  make              - Show this help message"
	@echo "  make clean        - Remove all build artifacts"
	@echo ""
	@echo "Test targets:"
	@echo "  make run-tests    - Run all tests"
	@echo "  make sanitize-tests - Run tests with AddressSanitizer + UBSan"
	@echo "  make tsan-tests   - Run tests with ThreadSanitizer"
	@echo "  make <test_name>  - Build and run a single test (e.g. make ar_data_tests)"
	@echo ""
	@echo "Analysis targets:"
	@echo "  make analyze-exec - Run static analysis on executable code"
	@echo "  make analyze-tests - Run static analysis on tests"
	@echo "  make check-naming - Check naming conventions"
	@echo "  make check-docs   - Check documentation validity"
	@echo "  make check-kb-links - Check KB article cross-references"
	@echo "  make check-kb-integration - Verify KB integration quality"
	@echo "  make check-all    - Run all code quality checks"
	@echo "  make build        - Run complete build with all checks and tests"
	@echo ""
	@echo "Run targets:"
	@echo "  make run-exec     - Build and run the executable"
	@echo "  make run-shell    - Build and run the shell scaffold executable"
	@echo "  make sanitize-exec - Build and run executable with sanitizers"
	@echo "  make tsan-exec    - Build and run executable with ThreadSanitizer"
	@echo ""
	@echo "Utility targets:"
	@echo "  make check-logs   - Check build logs for hidden issues"
	@echo "  make add-newline FILE=<file> - Add newline to end of file if missing"
	@echo "  make vendor-llama-cpu - Build/install vendored CPU-only llama.cpp into .deps"
	@echo "  make clean-llama-cpp - Remove vendored llama.cpp build/install artifacts from .deps"
	@echo "  make complete-runtime-ready - Build vendored libllama and download phi-3-mini-q4.gguf if missing"
	@echo "  make download-complete-model - Download phi-3-mini-q4.gguf into models/ if missing"
	@echo "  make complete-model-smoke - Ensure complete() runtime assets are ready and run the embedded libllama smoke with a cold-start timeout"
	@echo "  make complete-performance-validation - Run the documented complete() performance validation subtests"
	@echo "  make print-llama-config - Show vendored llama.cpp source/install paths"

# Output directories for parallel builds
ANALYZE_EXEC_DIR = bin/analyze-exec
ANALYZE_TESTS_DIR = bin/analyze-tests
# Allow RUN_EXEC_DIR to be overridden by environment variable for parallel test isolation
RUN_EXEC_DIR ?= bin/run-exec
RUN_TESTS_DIR = bin/run-tests
SANITIZE_TESTS_DIR = bin/sanitize-tests
SANITIZE_EXEC_DIR = bin/sanitize-exec
TSAN_TESTS_DIR = bin/tsan-tests
TSAN_EXEC_DIR = bin/tsan-exec

# Detect OS for sanitizer compiler selection
UNAME_S := $(shell uname -s)

# Vendored llama.cpp CPU-only dependency paths
LLAMA_SOURCE_DIR ?= llama-cpp
LLAMA_BUILD_DIR ?= .deps/llama.cpp-build
LLAMA_INSTALL_DIR ?= .deps/llama.cpp-install
LLAMA_LICENSE = $(LLAMA_SOURCE_DIR)/LICENSE
COMPLETE_MODEL_DIR ?= models
COMPLETE_MODEL_FILE ?= $(COMPLETE_MODEL_DIR)/phi-3-mini-q4.gguf
COMPLETE_MODEL_LICENSE ?= $(COMPLETE_MODEL_DIR)/phi-3-mini-q4.LICENSE
COMPLETE_MODEL_CARD ?= $(COMPLETE_MODEL_DIR)/phi-3-mini-q4.README.md
COMPLETE_MODEL_URL ?= https://huggingface.co/microsoft/Phi-3-mini-4k-instruct-gguf/resolve/main/Phi-3-mini-4k-instruct-q4.gguf?download=true
COMPLETE_MODEL_LICENSE_URL ?= https://huggingface.co/microsoft/Phi-3-mini-4k-instruct-gguf/raw/main/LICENSE
COMPLETE_MODEL_CARD_URL ?= https://huggingface.co/microsoft/Phi-3-mini-4k-instruct-gguf/raw/main/README.md
COMPLETE_MODEL_SMOKE_TIMEOUT_SECONDS ?= 35
ifeq ($(UNAME_S),Darwin)
LLAMA_SHARED_EXT = dylib
LLAMA_CPU_CMAKE_FLAGS = -DGGML_METAL=OFF
else
LLAMA_SHARED_EXT = so
LLAMA_CPU_CMAKE_FLAGS =
endif
LLAMA_SOURCE_HEADER = $(LLAMA_SOURCE_DIR)/include/llama.h
LLAMA_HEADER = $(LLAMA_INSTALL_DIR)/include/llama.h
LLAMA_LIB = $(LLAMA_INSTALL_DIR)/lib/libllama.$(LLAMA_SHARED_EXT)
LLAMA_AVAILABLE = $(and $(wildcard $(LLAMA_HEADER)),$(wildcard $(LLAMA_LIB)))
LLAMA_CFLAGS =
LLAMA_ZIG_INCLUDE_FLAGS =
ifneq ($(wildcard $(LLAMA_SOURCE_HEADER)),)
LLAMA_CFLAGS += -I$(LLAMA_SOURCE_DIR)/include -I$(LLAMA_SOURCE_DIR)/ggml/include
LLAMA_ZIG_INCLUDE_FLAGS += -I$(LLAMA_SOURCE_DIR)/include -I$(LLAMA_SOURCE_DIR)/ggml/include
endif

# Default compiler
CC = gcc-13

# Toolchain commands
ZIG = zig
CMAKE ?= cmake

# Sanitizer compiler selection based on OS
ifeq ($(UNAME_S),Darwin)
    SANITIZER_CC := clang
else
    SANITIZER_CC := $(CC)
endif

CFLAGS = -Wall -Wextra -Werror -Wpedantic -Wconversion -Wshadow -Wcast-qual \
         -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wstrict-aliasing=2 \
         -Wnull-dereference -Wformat=2 -Wuninitialized -Wpointer-arith \
         -Wunused -Wunused-parameter -Wwrite-strings -std=c11 -I./modules -D_GNU_SOURCE

# On Ubuntu, gcc-13 might need explicit include paths
ifeq ($(shell test -d /usr/include/x86_64-linux-gnu && echo yes),yes)
    CFLAGS += -I/usr/include/x86_64-linux-gnu
endif
# Also check for aarch64 (ARM64) Ubuntu systems
ifeq ($(shell test -d /usr/include/aarch64-linux-gnu && echo yes),yes)
    CFLAGS += -I/usr/include/aarch64-linux-gnu
endif

# Define Clang-specific flags (will be added when using clang)
CLANG_FLAGS = -Wno-newline-eof

# Define deployment target on Darwin to ensure consistent versioning
ifeq ($(UNAME_S),Darwin)
MACOS_DEPLOYMENT_TARGET = 15.5
DEPLOYMENT_FLAGS = -mmacosx-version-min=$(MACOS_DEPLOYMENT_TARGET)
CFLAGS += $(DEPLOYMENT_FLAGS)
# Determine architecture for Zig target
ARCH := $(shell uname -m)
ifeq ($(ARCH),arm64)
ZIG_TARGET = aarch64-macos.15.5...15.5-none
else
ZIG_TARGET = x86_64-macos.15.5...15.5-none
endif
else
ZIG_TARGET = native
endif

# Define SANITIZER_EXTRA_FLAGS based on OS (for use with sanitizer targets)
ifeq ($(UNAME_S),Darwin)
SANITIZER_EXTRA_FLAGS = $(CLANG_FLAGS) $(DEPLOYMENT_FLAGS)
else
SANITIZER_EXTRA_FLAGS =
endif

# zig test must link libc with an explicit target on macOS (Zig 0.14+). ThreadSanitizer via
# zig test is not supported for Homebrew Zig on Darwin (bundled libtsan fails to compile against the SDK).
ZIG_TEST_LINK_FLAGS = -target $(ZIG_TARGET) -lc
ifeq ($(UNAME_S),Darwin)
ZIG_TEST_TSAN_FLAGS =
else
ZIG_TEST_TSAN_FLAGS = -fsanitize-thread
endif

LDFLAGS = -lm -lc -pthread
CFLAGS += $(LLAMA_CFLAGS)

# Debug build flags
DEBUG_CFLAGS = -g -O0 -DDEBUG
# Release build flags
RELEASE_CFLAGS = -O3 -DNDEBUG
# Address Sanitizer flags
ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer
# Thread Sanitizer flags
TSAN_FLAGS = -fsanitize=thread -fno-omit-frame-pointer
# Undefined Behavior Sanitizer flags
UBSAN_FLAGS = -fsanitize=undefined -fno-omit-frame-pointer
# Combined sanitizer flags (ASan + UBSan - TSan must run separately)
SANITIZER_FLAGS = -fsanitize=address,undefined -fno-omit-frame-pointer
# Clang Static Analyzer command
ifeq ($(UNAME_S),Darwin)
    SCAN_BUILD = PATH="/opt/homebrew/opt/llvm/bin:$$PATH" scan-build
else
    SCAN_BUILD = scan-build
endif

# Source files (excluding test files)
ALL_C_FILES = $(wildcard modules/*.c)
# Exclude test files
C_SRC = $(filter-out %_tests.c %_fixture_tests.c,$(ALL_C_FILES))
ZIG_SRC = $(wildcard modules/*.zig)

# Standard object paths (for backward compatibility and default targets)
C_OBJ = $(patsubst modules/%.c,bin/obj/%.o,$(C_SRC))
ZIG_OBJ = $(patsubst modules/%.zig,bin/obj/%.o,$(ZIG_SRC))
OBJ = $(C_OBJ) $(ZIG_OBJ)

# Directory-specific object paths
RUN_TESTS_C_OBJ = $(patsubst modules/%.c,$(RUN_TESTS_DIR)/obj/%.o,$(C_SRC))
RUN_TESTS_ZIG_OBJ = $(patsubst modules/%.zig,$(RUN_TESTS_DIR)/obj/%.o,$(ZIG_SRC))
RUN_TESTS_OBJ = $(RUN_TESTS_C_OBJ) $(RUN_TESTS_ZIG_OBJ)

RUN_EXEC_C_OBJ = $(patsubst modules/%.c,$(RUN_EXEC_DIR)/obj/%.o,$(C_SRC))
RUN_EXEC_ZIG_OBJ = $(patsubst modules/%.zig,$(RUN_EXEC_DIR)/obj/%.o,$(ZIG_SRC))
RUN_EXEC_OBJ = $(RUN_EXEC_C_OBJ) $(RUN_EXEC_ZIG_OBJ)

SANITIZE_TESTS_C_OBJ = $(patsubst modules/%.c,$(SANITIZE_TESTS_DIR)/obj/%.o,$(C_SRC))
SANITIZE_TESTS_ZIG_OBJ = $(patsubst modules/%.zig,$(SANITIZE_TESTS_DIR)/obj/%.o,$(ZIG_SRC))
SANITIZE_TESTS_OBJ = $(SANITIZE_TESTS_C_OBJ) $(SANITIZE_TESTS_ZIG_OBJ)

SANITIZE_EXEC_C_OBJ = $(patsubst modules/%.c,$(SANITIZE_EXEC_DIR)/obj/%.o,$(C_SRC))
SANITIZE_EXEC_ZIG_OBJ = $(patsubst modules/%.zig,$(SANITIZE_EXEC_DIR)/obj/%.o,$(ZIG_SRC))
SANITIZE_EXEC_OBJ = $(SANITIZE_EXEC_C_OBJ) $(SANITIZE_EXEC_ZIG_OBJ)

TSAN_TESTS_C_OBJ = $(patsubst modules/%.c,$(TSAN_TESTS_DIR)/obj/%.o,$(C_SRC))
TSAN_TESTS_ZIG_OBJ = $(patsubst modules/%.zig,$(TSAN_TESTS_DIR)/obj/%.o,$(ZIG_SRC))
TSAN_TESTS_OBJ = $(TSAN_TESTS_C_OBJ) $(TSAN_TESTS_ZIG_OBJ)

TSAN_EXEC_C_OBJ = $(patsubst modules/%.c,$(TSAN_EXEC_DIR)/obj/%.o,$(C_SRC))
TSAN_EXEC_ZIG_OBJ = $(patsubst modules/%.zig,$(TSAN_EXEC_DIR)/obj/%.o,$(ZIG_SRC))
TSAN_EXEC_OBJ = $(TSAN_EXEC_C_OBJ) $(TSAN_EXEC_ZIG_OBJ)

# Test source files
TEST_SRC = $(wildcard modules/*_tests.c)
TEST_OBJ = $(patsubst modules/%.c,bin/obj/%.o,$(TEST_SRC))
TEST_BIN = $(patsubst modules/%_tests.c,bin/%_tests,$(TEST_SRC))

# Directory-specific test paths
RUN_TESTS_TEST_OBJ = $(patsubst modules/%.c,$(RUN_TESTS_DIR)/obj/%.o,$(TEST_SRC))
RUN_TESTS_TEST_BIN = $(patsubst modules/%_tests.c,$(RUN_TESTS_DIR)/%_tests,$(RUN_TEST_TEST_SRC))

SANITIZE_TESTS_TEST_OBJ = $(patsubst modules/%.c,$(SANITIZE_TESTS_DIR)/obj/%.o,$(SANITIZER_TEST_SRC))
SANITIZE_TESTS_TEST_BIN = $(patsubst modules/%_tests.c,$(SANITIZE_TESTS_DIR)/%_tests,$(SANITIZER_TEST_SRC))

TSAN_TESTS_TEST_OBJ = $(patsubst modules/%.c,$(TSAN_TESTS_DIR)/obj/%.o,$(SANITIZER_TEST_SRC))
TSAN_TESTS_TEST_BIN = $(patsubst modules/%_tests.c,$(TSAN_TESTS_DIR)/%_tests,$(SANITIZER_TEST_SRC))

# Method test source files
METHOD_TEST_SRC = $(wildcard methods/*_tests.c)
METHOD_TEST_OBJ = $(patsubst methods/%.c,bin/obj/%.o,$(METHOD_TEST_SRC))
METHOD_TEST_BIN = $(patsubst methods/%_tests.c,bin/%_tests,$(METHOD_TEST_SRC))

# Zig struct module test source files
ZIG_TEST_SRC = $(wildcard modules/*Tests.zig)
ZIG_TEST_BIN = $(patsubst modules/%Tests.zig,bin/%Tests,$(ZIG_TEST_SRC))

# Directory-specific method test paths
RUN_TESTS_METHOD_TEST_OBJ = $(patsubst methods/%.c,$(RUN_TESTS_DIR)/obj/%.o,$(METHOD_TEST_SRC))
RUN_TESTS_METHOD_TEST_BIN = $(patsubst methods/%_tests.c,$(RUN_TESTS_DIR)/%_tests,$(METHOD_TEST_SRC))

SANITIZE_TESTS_METHOD_TEST_OBJ = $(patsubst methods/%.c,$(SANITIZE_TESTS_DIR)/obj/%.o,$(SANITIZER_METHOD_TEST_SRC))
SANITIZE_TESTS_METHOD_TEST_BIN = $(patsubst methods/%_tests.c,$(SANITIZE_TESTS_DIR)/%_tests,$(SANITIZER_METHOD_TEST_SRC))

TSAN_TESTS_METHOD_TEST_OBJ = $(patsubst methods/%.c,$(TSAN_TESTS_DIR)/obj/%.o,$(SANITIZER_METHOD_TEST_SRC))
TSAN_TESTS_METHOD_TEST_BIN = $(patsubst methods/%_tests.c,$(TSAN_TESTS_DIR)/%_tests,$(SANITIZER_METHOD_TEST_SRC))

# Directory-specific Zig test paths
RUN_TESTS_ZIG_TEST_BIN = $(patsubst modules/%Tests.zig,$(RUN_TESTS_DIR)/%Tests,$(ZIG_TEST_SRC))
SANITIZE_TESTS_ZIG_TEST_BIN = $(patsubst modules/%Tests.zig,$(SANITIZE_TESTS_DIR)/%Tests,$(ZIG_TEST_SRC))
TSAN_TESTS_ZIG_TEST_BIN = $(patsubst modules/%Tests.zig,$(TSAN_TESTS_DIR)/%Tests,$(ZIG_TEST_SRC))

# Create directory-specific directories
$(ANALYZE_EXEC_DIR):
	mkdir -p $@/obj $@/scan-build-results

$(ANALYZE_TESTS_DIR):
	mkdir -p $@/obj $@/scan-build-results

$(RUN_TESTS_DIR):
	mkdir -p $@/obj

$(RUN_EXEC_DIR):
	mkdir -p $@/obj

$(SANITIZE_TESTS_DIR):
	mkdir -p $@/obj

$(SANITIZE_EXEC_DIR):
	mkdir -p $@/obj

$(TSAN_TESTS_DIR):
	mkdir -p $@/obj

$(TSAN_EXEC_DIR):
	mkdir -p $@/obj


# Directory-specific library targets
run_tests_lib: $(RUN_TESTS_DIR) $(RUN_TESTS_OBJ) $(RUN_TESTS_TEST_OBJ) $(RUN_TESTS_METHOD_TEST_OBJ)
	ar rcs $(RUN_TESTS_DIR)/libagerun.a $(RUN_TESTS_OBJ)

run_exec_lib: $(RUN_EXEC_DIR) $(RUN_EXEC_OBJ)
	ar rcs $(RUN_EXEC_DIR)/libagerun.a $(RUN_EXEC_OBJ)

sanitize_tests_lib: CC = $(SANITIZER_CC)
sanitize_tests_lib: CFLAGS += $(DEBUG_CFLAGS) $(SANITIZER_FLAGS) $(SANITIZER_EXTRA_FLAGS)
sanitize_tests_lib: LDFLAGS += $(SANITIZER_FLAGS)
sanitize_tests_lib: $(SANITIZE_TESTS_DIR) $(SANITIZE_TESTS_OBJ) $(SANITIZE_TESTS_TEST_OBJ) $(SANITIZE_TESTS_METHOD_TEST_OBJ)
	ar rcs $(SANITIZE_TESTS_DIR)/libagerun.a $(SANITIZE_TESTS_OBJ)

sanitize_exec_lib: CC = $(SANITIZER_CC)
sanitize_exec_lib: CFLAGS += $(DEBUG_CFLAGS) $(SANITIZER_FLAGS) $(SANITIZER_EXTRA_FLAGS)
sanitize_exec_lib: LDFLAGS += $(SANITIZER_FLAGS)
sanitize_exec_lib: $(SANITIZE_EXEC_DIR) $(SANITIZE_EXEC_OBJ)
	ar rcs $(SANITIZE_EXEC_DIR)/libagerun.a $(SANITIZE_EXEC_OBJ)

tsan_tests_lib: CC = $(SANITIZER_CC)
tsan_tests_lib: CFLAGS += $(DEBUG_CFLAGS) $(TSAN_FLAGS) $(SANITIZER_EXTRA_FLAGS)
tsan_tests_lib: LDFLAGS += $(TSAN_FLAGS)
tsan_tests_lib: $(TSAN_TESTS_DIR) $(TSAN_TESTS_OBJ) $(TSAN_TESTS_TEST_OBJ) $(TSAN_TESTS_METHOD_TEST_OBJ)
	ar rcs $(TSAN_TESTS_DIR)/libagerun.a $(TSAN_TESTS_OBJ)

tsan_exec_lib: CC = $(SANITIZER_CC)
tsan_exec_lib: CFLAGS += $(DEBUG_CFLAGS) $(TSAN_FLAGS) $(SANITIZER_EXTRA_FLAGS)
tsan_exec_lib: LDFLAGS += $(TSAN_FLAGS)
tsan_exec_lib: $(TSAN_EXEC_DIR) $(TSAN_EXEC_OBJ)
	ar rcs $(TSAN_EXEC_DIR)/libagerun.a $(TSAN_EXEC_OBJ)

# Build and run the executable (always in debug mode)
run-exec: run_exec_lib
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -o $(RUN_EXEC_DIR)/agerun modules/ar_executable.c $(RUN_EXEC_DIR)/libagerun.a $(LDFLAGS)
	cd $(RUN_EXEC_DIR) && AGERUN_MEMORY_REPORT="memory_report_agerun.log" ./agerun

run-shell: run_exec_lib
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -DAR_SHELL__BUILD_MAIN -o $(RUN_EXEC_DIR)/arsh modules/ar_shell.c $(RUN_EXEC_DIR)/libagerun.a $(LDFLAGS)
	cd $(RUN_EXEC_DIR) && AGERUN_MEMORY_REPORT="memory_report_arsh.log" ./arsh

# Build and run the executable with Address + Undefined Behavior Sanitizers
sanitize-exec: sanitize_exec_lib
	$(SANITIZER_CC) $(CFLAGS) $(DEBUG_CFLAGS) $(SANITIZER_FLAGS) $(SANITIZER_EXTRA_FLAGS) -o $(SANITIZE_EXEC_DIR)/agerun modules/ar_executable.c $(SANITIZE_EXEC_DIR)/libagerun.a $(LDFLAGS) $(SANITIZER_FLAGS)
	cd $(SANITIZE_EXEC_DIR) && AGERUN_MEMORY_REPORT="memory_report_agerun.log" ./agerun

# Build and run the executable with Thread Sanitizer
tsan-exec: tsan_exec_lib
	$(SANITIZER_CC) $(CFLAGS) $(DEBUG_CFLAGS) $(TSAN_FLAGS) $(SANITIZER_EXTRA_FLAGS) -o $(TSAN_EXEC_DIR)/agerun modules/ar_executable.c $(TSAN_EXEC_DIR)/libagerun.a $(LDFLAGS) $(TSAN_FLAGS)
	cd $(TSAN_EXEC_DIR) && AGERUN_MEMORY_REPORT="memory_report_agerun.log" ./agerun

# Define test executables without bin/ prefix for use in the bin directory
# DLSym interception tests remain individually runnable but are excluded from aggregate
# run-tests because they provide their own symbol definitions that conflict with the full library.
RUN_TEST_TEST_SRC = $(filter-out %_dlsym_tests.c,$(TEST_SRC))
RUN_TEST_TEST_BIN_NAMES = $(patsubst modules/%_tests.c,%_tests,$(RUN_TEST_TEST_SRC))
METHOD_TEST_BIN_NAMES = $(notdir $(METHOD_TEST_BIN))
ZIG_TEST_BIN_NAMES = $(notdir $(ZIG_TEST_BIN))
ALL_TEST_BIN_NAMES = $(RUN_TEST_TEST_BIN_NAMES) $(METHOD_TEST_BIN_NAMES) $(ZIG_TEST_BIN_NAMES)

# Filtered test sources for sanitizers (exclude *_dlsym_tests.c)
SANITIZER_TEST_SRC = $(filter-out %_dlsym_tests.c,$(TEST_SRC))
SANITIZER_METHOD_TEST_SRC = $(filter-out %_dlsym_tests.c,$(METHOD_TEST_SRC))

# Filtered test binaries for sanitizers
SANITIZER_TEST_BIN_NAMES = $(patsubst modules/%_tests.c,%_tests,$(SANITIZER_TEST_SRC))
SANITIZER_METHOD_TEST_BIN_NAMES = $(patsubst methods/%_tests.c,%_tests,$(SANITIZER_METHOD_TEST_SRC))
SANITIZER_ALL_TEST_BIN_NAMES = $(SANITIZER_TEST_BIN_NAMES) $(SANITIZER_METHOD_TEST_BIN_NAMES) $(ZIG_TEST_BIN_NAMES)

# Build and run tests (always in debug mode)
run-tests: run_tests_lib
	$(MAKE) $(RUN_TESTS_TEST_BIN) $(RUN_TESTS_METHOD_TEST_BIN)
	$(MAKE) $(RUN_TESTS_ZIG_TEST_BIN)
	@cd $(RUN_TESTS_DIR) && failed=0 && for test in $(ALL_TEST_BIN_NAMES); do \
		rm -f *.agerun; \
		echo "Running test: $$test"; \
		case "$$test" in \
			*Tests) \
				if ! ./$$test; then \
					echo "ERROR: Test $$test failed with status $$?"; \
					failed=1; \
				fi; \
				;; \
			*) \
				if ! AGERUN_MEMORY_REPORT="memory_report_$$test.log" ./$$test; then \
					echo "ERROR: Test $$test failed with status $$?"; \
					failed=1; \
				fi; \
				;; \
		esac; \
	done; \
	if [ $$failed -ne 0 ]; then \
		echo "ERROR: One or more tests failed"; \
		exit 1; \
	fi

# Build and run tests with Address + Undefined Behavior Sanitizers
sanitize-tests:
	$(MAKE) sanitize_tests_lib
	$(MAKE) $(SANITIZE_TESTS_TEST_BIN) $(SANITIZE_TESTS_METHOD_TEST_BIN) CC="$(SANITIZER_CC)" CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS) $(SANITIZER_FLAGS) $(SANITIZER_EXTRA_FLAGS)" LDFLAGS="$(LDFLAGS) $(SANITIZER_FLAGS)"
	$(MAKE) $(SANITIZE_TESTS_ZIG_TEST_BIN)
	@cd $(SANITIZE_TESTS_DIR) && failed=0 && for test in $(SANITIZER_ALL_TEST_BIN_NAMES); do \
		rm -f *.agerun; \
		echo "Running test: $$test with Address + Undefined Behavior Sanitizers"; \
		case "$$test" in \
			*Tests) \
				./$$test; \
				exitcode=$$?; \
				if [ $$exitcode -ne 0 ]; then \
					echo "ERROR: Test $$test failed with status $$exitcode"; \
					failed=1; \
				fi; \
				;; \
			*) \
				AGERUN_MEMORY_REPORT="memory_report_$$test.log" ./$$test; \
				exitcode=$$?; \
				if [ $$exitcode -ne 0 ]; then \
					echo "ERROR: Test $$test failed with status $$exitcode"; \
					failed=1; \
				fi; \
				;; \
		esac; \
	done; \
	if [ $$failed -ne 0 ]; then \
		echo "ERROR: One or more tests failed"; \
		exit 1; \
	fi

# Build and run tests with Thread Sanitizer (must run separately from ASan)
tsan-tests:
	$(MAKE) tsan_tests_lib
	$(MAKE) $(TSAN_TESTS_TEST_BIN) $(TSAN_TESTS_METHOD_TEST_BIN) CC="$(SANITIZER_CC)" CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS) $(TSAN_FLAGS) $(SANITIZER_EXTRA_FLAGS)" LDFLAGS="$(LDFLAGS) $(TSAN_FLAGS)"
	$(MAKE) $(TSAN_TESTS_ZIG_TEST_BIN)
	@cd $(TSAN_TESTS_DIR) && failed=0 && for test in $(SANITIZER_ALL_TEST_BIN_NAMES); do \
		rm -f *.agerun; \
		echo "Running test: $$test with Thread Sanitizer"; \
		case "$$test" in \
			*Tests) \
				./$$test; \
				exitcode=$$?; \
				if [ $$exitcode -ne 0 ]; then \
					echo "ERROR: Test $$test failed with status $$exitcode"; \
					failed=1; \
				fi; \
				;; \
			*) \
				AGERUN_MEMORY_REPORT="memory_report_$$test.log" ./$$test; \
				exitcode=$$?; \
				if [ $$exitcode -ne 0 ]; then \
					echo "ERROR: Test $$test failed with status $$exitcode"; \
					failed=1; \
				fi; \
				;; \
		esac; \
	done; \
	if [ $$failed -ne 0 ]; then \
		echo "ERROR: One or more tests failed"; \
		exit 1; \
	fi

# Individual test binaries (build in run-tests directory for consistency)
# Individual test target (without bin/ prefix for convenience)
%_tests: bin/%_tests
	@# Target completed by dependency

# Build and run individual test with bin/ prefix
bin/%_tests: $(RUN_TESTS_DIR)/obj/%_tests.o run_tests_lib
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -o $(RUN_TESTS_DIR)/$*_tests $< $(RUN_TESTS_DIR)/libagerun.a $(LDFLAGS)
	@echo "Running test: $*_tests"
	@cd $(RUN_TESTS_DIR) && AGERUN_MEMORY_REPORT="memory_report_$*_tests.log" ./$*_tests

# Directory-specific test binaries
$(RUN_TESTS_DIR)/%_tests: $(RUN_TESTS_DIR)/obj/%_tests.o run_tests_lib
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -o $@ $< $(RUN_TESTS_DIR)/libagerun.a $(LDFLAGS)

$(SANITIZE_TESTS_DIR)/%_tests: $(SANITIZE_TESTS_DIR)/obj/%_tests.o sanitize_tests_lib
	$(SANITIZER_CC) $(CFLAGS) $(DEBUG_CFLAGS) $(SANITIZER_FLAGS) $(SANITIZER_EXTRA_FLAGS) -o $@ $< $(SANITIZE_TESTS_DIR)/libagerun.a $(LDFLAGS) $(SANITIZER_FLAGS)

$(TSAN_TESTS_DIR)/%_tests: $(TSAN_TESTS_DIR)/obj/%_tests.o tsan_tests_lib
	$(SANITIZER_CC) $(CFLAGS) $(DEBUG_CFLAGS) $(TSAN_FLAGS) $(SANITIZER_EXTRA_FLAGS) -o $@ $< $(TSAN_TESTS_DIR)/libagerun.a $(LDFLAGS) $(TSAN_FLAGS)

# Individual Zig test targets
%Tests: bin/%Tests
	@# Target completed by dependency

# Build and run individual Zig test with bin/ prefix
bin/%Tests: modules/%Tests.zig
	@echo "Building and running Zig test: $*Tests"
	@cd $(RUN_TESTS_DIR) && $(ZIG) test ../../modules/$*Tests.zig -femit-bin=$*Tests $(ZIG_TEST_LINK_FLAGS)
	@cd $(RUN_TESTS_DIR) && ./$*Tests

# Directory-specific Zig test binaries
$(RUN_TESTS_DIR)/%Tests: modules/%Tests.zig | $(RUN_TESTS_DIR)
	$(ZIG) test $< -femit-bin=$@ $(ZIG_TEST_LINK_FLAGS)

$(SANITIZE_TESTS_DIR)/%Tests: modules/%Tests.zig | $(SANITIZE_TESTS_DIR)
	$(ZIG) test $< -femit-bin=$@ -fsanitize-c $(ZIG_TEST_LINK_FLAGS)

$(TSAN_TESTS_DIR)/%Tests: modules/%Tests.zig | $(TSAN_TESTS_DIR)
	$(ZIG) test $< -femit-bin=$@ $(ZIG_TEST_TSAN_FLAGS) $(ZIG_TEST_LINK_FLAGS)

# Note: Individual test builds use bin/run-tests/ directory via the bin/%_tests target above
# No generic bin/ compilation rules needed for tests

# Directory-specific compilation rules
# Run tests directory
$(RUN_TESTS_DIR)/obj/%.o: modules/%.c | $(RUN_TESTS_DIR)
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -c $< -o $@

$(RUN_TESTS_DIR)/obj/%.o: modules/%.zig | $(RUN_TESTS_DIR)
	$(ZIG) build-obj -O Debug -DDEBUG -D__ZIG__ -target $(ZIG_TARGET) -mcpu=native -fno-stack-check -lc -I./modules $(LLAMA_ZIG_INCLUDE_FLAGS) $< -femit-bin=$@

$(RUN_TESTS_DIR)/obj/%_tests.o: modules/%_tests.c | $(RUN_TESTS_DIR)
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -c $< -o $@

$(RUN_TESTS_DIR)/obj/%_tests.o: methods/%_tests.c | $(RUN_TESTS_DIR)
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -c $< -o $@

# Run exec directory
$(RUN_EXEC_DIR)/obj/%.o: modules/%.c | $(RUN_EXEC_DIR)
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -c $< -o $@

$(RUN_EXEC_DIR)/obj/%.o: modules/%.zig | $(RUN_EXEC_DIR)
	$(ZIG) build-obj -O Debug -DDEBUG -D__ZIG__ -target $(ZIG_TARGET) -mcpu=native -fno-stack-check -lc -I./modules $(LLAMA_ZIG_INCLUDE_FLAGS) $< -femit-bin=$@

# Sanitize tests directory
$(SANITIZE_TESTS_DIR)/obj/%.o: modules/%.c | $(SANITIZE_TESTS_DIR)
	$(SANITIZER_CC) $(CFLAGS) $(DEBUG_CFLAGS) $(SANITIZER_FLAGS) $(SANITIZER_EXTRA_FLAGS) -c $< -o $@

$(SANITIZE_TESTS_DIR)/obj/%.o: modules/%.zig | $(SANITIZE_TESTS_DIR)
	$(ZIG) build-obj -O Debug -DDEBUG -D__ZIG__ -target $(ZIG_TARGET) -mcpu=native -fno-stack-check -lc -I./modules $(LLAMA_ZIG_INCLUDE_FLAGS) $< -femit-bin=$@

$(SANITIZE_TESTS_DIR)/obj/%_tests.o: modules/%_tests.c | $(SANITIZE_TESTS_DIR)
	$(SANITIZER_CC) $(CFLAGS) $(DEBUG_CFLAGS) $(SANITIZER_FLAGS) $(SANITIZER_EXTRA_FLAGS) -c $< -o $@

$(SANITIZE_TESTS_DIR)/obj/%_tests.o: methods/%_tests.c | $(SANITIZE_TESTS_DIR)
	$(SANITIZER_CC) $(CFLAGS) $(DEBUG_CFLAGS) $(SANITIZER_FLAGS) $(SANITIZER_EXTRA_FLAGS) -c $< -o $@

# Sanitize exec directory
$(SANITIZE_EXEC_DIR)/obj/%.o: modules/%.c | $(SANITIZE_EXEC_DIR)
	$(SANITIZER_CC) $(CFLAGS) $(DEBUG_CFLAGS) $(SANITIZER_FLAGS) $(SANITIZER_EXTRA_FLAGS) -c $< -o $@

$(SANITIZE_EXEC_DIR)/obj/%.o: modules/%.zig | $(SANITIZE_EXEC_DIR)
	$(ZIG) build-obj -O Debug -DDEBUG -D__ZIG__ -target $(ZIG_TARGET) -mcpu=native -fno-stack-check -lc -I./modules $(LLAMA_ZIG_INCLUDE_FLAGS) $< -femit-bin=$@

# TSan tests directory
$(TSAN_TESTS_DIR)/obj/%.o: modules/%.c | $(TSAN_TESTS_DIR)
	$(SANITIZER_CC) $(CFLAGS) $(DEBUG_CFLAGS) $(TSAN_FLAGS) $(SANITIZER_EXTRA_FLAGS) -c $< -o $@

$(TSAN_TESTS_DIR)/obj/%.o: modules/%.zig | $(TSAN_TESTS_DIR)
	$(ZIG) build-obj -O Debug -DDEBUG -D__ZIG__ -target $(ZIG_TARGET) -mcpu=native -fno-stack-check -lc -I./modules $(LLAMA_ZIG_INCLUDE_FLAGS) $< -femit-bin=$@

$(TSAN_TESTS_DIR)/obj/%_tests.o: modules/%_tests.c | $(TSAN_TESTS_DIR)
	$(SANITIZER_CC) $(CFLAGS) $(DEBUG_CFLAGS) $(TSAN_FLAGS) $(SANITIZER_EXTRA_FLAGS) -c $< -o $@

$(TSAN_TESTS_DIR)/obj/%_tests.o: methods/%_tests.c | $(TSAN_TESTS_DIR)
	$(SANITIZER_CC) $(CFLAGS) $(DEBUG_CFLAGS) $(TSAN_FLAGS) $(SANITIZER_EXTRA_FLAGS) -c $< -o $@

# TSan exec directory
$(TSAN_EXEC_DIR)/obj/%.o: modules/%.c | $(TSAN_EXEC_DIR)
	$(SANITIZER_CC) $(CFLAGS) $(DEBUG_CFLAGS) $(TSAN_FLAGS) $(SANITIZER_EXTRA_FLAGS) -c $< -o $@

$(TSAN_EXEC_DIR)/obj/%.o: modules/%.zig | $(TSAN_EXEC_DIR)
	$(ZIG) build-obj -O Debug -DDEBUG -D__ZIG__ -target $(ZIG_TARGET) -mcpu=native -fno-stack-check -lc -I./modules $(LLAMA_ZIG_INCLUDE_FLAGS) $< -femit-bin=$@

# Clean vendored llama.cpp build/install artifacts
clean-llama-cpp:
	rm -rf "$(LLAMA_BUILD_DIR)" "$(LLAMA_INSTALL_DIR)"

# Clean target
clean:
	rm -rf bin logs

# Static analysis target
analyze-exec: $(ANALYZE_EXEC_DIR)
	@if command -v /opt/homebrew/opt/llvm/bin/scan-build >/dev/null 2>&1 || command -v scan-build >/dev/null 2>&1; then \
		rm -rf $(ANALYZE_EXEC_DIR)/scan-build-results/*; \
		echo "Running scan-build on source files with $(CC)..."; \
		total_bugs=0; \
		rm -f $(ANALYZE_EXEC_DIR)/scan-build-analyze.log; \
		rm -f $(ANALYZE_EXEC_DIR)/scan-build-warnings.log; \
		for file in $(C_SRC); do \
			echo "Analyzing $$file..."; \
			$(SCAN_BUILD) -o $(ANALYZE_EXEC_DIR)/scan-build-results --status-bugs --use-cc=$(CC) $(CC) $(CFLAGS) $(DEBUG_CFLAGS) -c -I./modules $$file -o $(ANALYZE_EXEC_DIR)/obj/$$(basename $$file .c).o 2>&1 | tee $(ANALYZE_EXEC_DIR)/scan-build-temp.log; \
			if grep -q "scan-build: [0-9]* bug" $(ANALYZE_EXEC_DIR)/scan-build-temp.log && ! grep -q "scan-build: 0 bugs found" $(ANALYZE_EXEC_DIR)/scan-build-temp.log; then \
				file_bugs=$$(grep "scan-build: [0-9]* bug" $(ANALYZE_EXEC_DIR)/scan-build-temp.log | tail -1 | sed 's/.*scan-build: \([0-9]*\) bug.*/\1/'); \
				echo "  ✗ $$file_bugs bugs found in $$file"; \
				echo "=== Warnings in $$file ===" >> $(ANALYZE_EXEC_DIR)/scan-build-warnings.log; \
				grep -E "(warning:|error:|note:)" $(ANALYZE_EXEC_DIR)/scan-build-temp.log >> $(ANALYZE_EXEC_DIR)/scan-build-warnings.log 2>/dev/null || true; \
				grep -B2 -A2 "warning:" $(ANALYZE_EXEC_DIR)/scan-build-temp.log | grep -v "^--$$" >> $(ANALYZE_EXEC_DIR)/scan-build-warnings.log 2>/dev/null || true; \
				echo >> $(ANALYZE_EXEC_DIR)/scan-build-warnings.log; \
				total_bugs=$$((total_bugs + file_bugs)); \
			fi; \
			cat $(ANALYZE_EXEC_DIR)/scan-build-temp.log >> $(ANALYZE_EXEC_DIR)/scan-build-analyze.log; \
		done; \
		if [ $$total_bugs -gt 0 ]; then \
			echo "Static analysis FAILED: $$total_bugs total bugs found"; \
			echo "View detailed reports in: $(ANALYZE_EXEC_DIR)/scan-build-results/"; \
			if [ -f $(ANALYZE_EXEC_DIR)/scan-build-warnings.log ]; then \
				echo "=== Static Analysis Warnings ==="; \
				cat $(ANALYZE_EXEC_DIR)/scan-build-warnings.log | head -50; \
				if [ $$(wc -l < $(ANALYZE_EXEC_DIR)/scan-build-warnings.log) -gt 50 ]; then \
					echo "... (truncated, see full output in $(ANALYZE_EXEC_DIR)/scan-build-warnings.log)"; \
				fi; \
			fi; \
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
analyze-tests: $(ANALYZE_TESTS_DIR)
	@if command -v /opt/homebrew/opt/llvm/bin/scan-build >/dev/null 2>&1 || command -v scan-build >/dev/null 2>&1; then \
		rm -rf $(ANALYZE_TESTS_DIR)/scan-build-results/*; \
		echo "Running scan-build on test files with $(CC)..."; \
		total_bugs=0; \
		rm -f $(ANALYZE_TESTS_DIR)/scan-build-analyze-tests.log; \
		rm -f $(ANALYZE_TESTS_DIR)/scan-build-warnings.log; \
		for file in $(C_SRC) $(TEST_SRC) $(METHOD_TEST_SRC); do \
			echo "Analyzing $$file..."; \
			$(SCAN_BUILD) -o $(ANALYZE_TESTS_DIR)/scan-build-results --status-bugs --use-cc=$(CC) $(CC) $(CFLAGS) $(DEBUG_CFLAGS) -c -I./modules $$file -o $(ANALYZE_TESTS_DIR)/obj/$$(basename $$file .c).o 2>&1 | tee $(ANALYZE_TESTS_DIR)/scan-build-temp-tests.log; \
			if grep -q "scan-build: [0-9]* bug" $(ANALYZE_TESTS_DIR)/scan-build-temp-tests.log && ! grep -q "scan-build: 0 bugs found" $(ANALYZE_TESTS_DIR)/scan-build-temp-tests.log; then \
				file_bugs=$$(grep "scan-build: [0-9]* bug" $(ANALYZE_TESTS_DIR)/scan-build-temp-tests.log | tail -1 | sed 's/.*scan-build: \([0-9]*\) bug.*/\1/'); \
				echo "  ✗ $$file_bugs bugs found in $$file"; \
				echo "=== Warnings in $$file ===" >> $(ANALYZE_TESTS_DIR)/scan-build-warnings.log; \
				grep -E "(warning:|error:|note:)" $(ANALYZE_TESTS_DIR)/scan-build-temp-tests.log >> $(ANALYZE_TESTS_DIR)/scan-build-warnings.log 2>/dev/null || true; \
				grep -B2 -A2 "warning:" $(ANALYZE_TESTS_DIR)/scan-build-temp-tests.log | grep -v "^--$$" >> $(ANALYZE_TESTS_DIR)/scan-build-warnings.log 2>/dev/null || true; \
				echo >> $(ANALYZE_TESTS_DIR)/scan-build-warnings.log; \
				total_bugs=$$((total_bugs + file_bugs)); \
			fi; \
			cat $(ANALYZE_TESTS_DIR)/scan-build-temp-tests.log >> $(ANALYZE_TESTS_DIR)/scan-build-analyze-tests.log; \
		done; \
		if [ $$total_bugs -gt 0 ]; then \
			echo "Static analysis FAILED: $$total_bugs total bugs found in tests"; \
			echo "View detailed reports in: $(ANALYZE_TESTS_DIR)/scan-build-results/"; \
			if [ -f $(ANALYZE_TESTS_DIR)/scan-build-warnings.log ]; then \
				echo "=== Static Analysis Warnings ==="; \
				cat $(ANALYZE_TESTS_DIR)/scan-build-warnings.log | head -50; \
				if [ $$(wc -l < $(ANALYZE_TESTS_DIR)/scan-build-warnings.log) -gt 50 ]; then \
					echo "... (truncated, see full output in $(ANALYZE_TESTS_DIR)/scan-build-warnings.log)"; \
				fi; \
			fi; \
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

# Check naming conventions across all source files
check-naming:
	@if [ -x ./scripts/check-naming-conventions.sh ]; then \
		./scripts/check-naming-conventions.sh; \
	else \
		echo "ERROR: scripts/check-naming-conventions.sh not found or not executable"; \
		echo "Make sure the script exists and has execute permissions"; \
		exit 1; \
	fi

# Check documentation validity (file references, module names, function/type refs)
check-docs:
	@if command -v python3 >/dev/null 2>&1; then \
		python3 ./scripts/check_docs.py; \
	else \
		echo "ERROR: python3 not found - required for documentation validation"; \
		echo "Install python3 to run check_docs.py"; \
		exit 1; \
	fi

# Run all code quality checks (naming conventions and documentation)
check-all: check-naming check-docs
	@echo ""
	@echo "All code quality checks completed!"

# Run build script (static analysis, all tests, sanitizers, doc validation)
build: install-scan-build
	@if [ -x ./scripts/build.sh ]; then \
		./scripts/build.sh; \
	else \
		echo "ERROR: scripts/build.sh not found or not executable"; \
		echo "Make sure the script exists and has execute permissions"; \
		exit 1; \
	fi

# Check build logs for hidden issues
check-logs:
	@if [ -x ./scripts/check_logs.py ]; then \
		python3 ./scripts/check_logs.py; \
	else \
		echo "ERROR: scripts/check_logs.py not found or not executable"; \
		exit 1; \
	fi

# Add newline to end of file if missing
add-newline:
	@if [ -z "$(FILE)" ]; then \
		echo "ERROR: Please specify a file using FILE=<filename>"; \
		echo "Usage: make add-newline FILE=<filename>"; \
		exit 1; \
	fi
	@if [ -x ./scripts/add-newline.sh ]; then \
		./scripts/add-newline.sh "$(FILE)"; \
	else \
		echo "ERROR: scripts/add-newline.sh not found or not executable"; \
		echo "Make sure the script exists and has execute permissions"; \
		exit 1; \
	fi

.PHONY: help clean clean-llama-cpp build add-newline check-naming check-docs check-all analyze-exec analyze-tests run-exec run-tests sanitize-exec sanitize-tests tsan-exec tsan-tests install-scan-build print-src print-obj print-llama-config vendor-llama-cpu complete-runtime-ready download-complete-model complete-model-smoke complete-performance-validation

# Debug targets
print-src:
	@echo "C_SRC = $(C_SRC)"
	@echo "Number of C_SRC files: $(words $(C_SRC))"
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


$(COMPLETE_MODEL_FILE):
	@mkdir -p "$(COMPLETE_MODEL_DIR)"
	curl -L --fail --continue-at - --output "$@" "$(COMPLETE_MODEL_URL)"

$(COMPLETE_MODEL_LICENSE):
	@mkdir -p "$(COMPLETE_MODEL_DIR)"
	curl -L --fail --output "$@" "$(COMPLETE_MODEL_LICENSE_URL)"

$(COMPLETE_MODEL_CARD):
	@mkdir -p "$(COMPLETE_MODEL_DIR)"
	curl -L --fail --output "$@" "$(COMPLETE_MODEL_CARD_URL)"

# Download the real phi-3 completion model and its accompanying license/card files
# Usage: make download-complete-model
download-complete-model: $(COMPLETE_MODEL_FILE) $(COMPLETE_MODEL_LICENSE) $(COMPLETE_MODEL_CARD)

# Ensure complete() runtime assets are ready for embedded local completion tests
# Usage: make complete-runtime-ready
complete-runtime-ready: vendor-llama-cpu download-complete-model
	@# Target completed by dependency

# Download/build complete() runtime assets if needed and run the embedded libllama smoke test
# Usage: make complete-model-smoke
complete-model-smoke: complete-runtime-ready
	$(MAKE) $(RUN_TESTS_DIR)/ar_local_completion_tests 2>&1
	@cd $(RUN_TESTS_DIR) && \
	AGERUN_MEMORY_REPORT="memory_report_ar_local_completion_tests.log" \
	AGERUN_LOCAL_COMPLETION_SUBTEST=real_phi3_model_smoke \
	python3 -c 'code = """import os, signal, subprocess, sys\ntimeout = int(sys.argv[1])\nenv = dict(os.environ)\nproc = subprocess.Popen([\"./ar_local_completion_tests\"], start_new_session=True, env=env)\ntry:\n    sys.exit(proc.wait(timeout=timeout))\nexcept subprocess.TimeoutExpired:\n    os.killpg(proc.pid, signal.SIGKILL)\n    print(f\"ERROR: complete-model-smoke exceeded {timeout}s cold-start budget\", file=sys.stderr)\n    sys.exit(124)\n"""; exec(code)' "$(COMPLETE_MODEL_SMOKE_TIMEOUT_SECONDS)"

# Run the documented complete() performance validation subtests
# Usage: make complete-performance-validation
complete-performance-validation: complete-runtime-ready
	$(MAKE) $(RUN_TESTS_DIR)/ar_local_completion_tests 2>&1
	$(MAKE) $(RUN_TESTS_DIR)/ar_complete_instruction_evaluator_tests 2>&1
	@cd $(RUN_TESTS_DIR) && \
	AGERUN_MEMORY_REPORT="memory_report_ar_local_completion_tests.log" \
	AGERUN_LOCAL_COMPLETION_SUBTEST=real_phi3_fixture_set_warm_run_support \
	./ar_local_completion_tests
	@cd $(RUN_TESTS_DIR) && \
	python3 -c 'code = """import os, re, subprocess, sys\nn = 20\nsuccess = 0\nunder = 0\nelapsed_values = []\nfor index in range(n):\n    env = dict(os.environ)\n    env["AGERUN_MEMORY_REPORT"] = "memory_report_ar_complete_instruction_evaluator_tests.log"\n    env["AGERUN_COMPLETE_EVALUATOR_SUBTEST"] = "performance_cold_fixture"\n    env["AGERUN_COMPLETE_EVALUATOR_FIXTURE_INDEX"] = str(index)\n    proc = subprocess.run(["./ar_complete_instruction_evaluator_tests"], capture_output=True, text=True, env=env)\n    sys.stdout.write(proc.stdout)\n    sys.stderr.write(proc.stderr)\n    if proc.returncode != 0:\n        sys.exit(proc.returncode)\n    match = re.search(r"Cold evaluator fixture (\\d+) elapsed=(\\d+) ms success=(yes|no) under_limit=(yes|no)", proc.stdout)\n    if not match:\n        print(f"ERROR: could not parse cold evaluator output for fixture {index+1}", file=sys.stderr)\n        sys.exit(1)\n    elapsed = int(match.group(2))\n    elapsed_values.append(elapsed)\n    if match.group(3) == "yes":\n        success += 1\n    if match.group(4) == "yes":\n        under += 1\navg = sum(elapsed_values) // len(elapsed_values)\nprint(f"Cold evaluator summary: fixtures={n} success={success} under_30000ms={under} avg={avg} ms max={max(elapsed_values)} ms")\nif success < 18 or under < 18:\n    sys.exit(1)\n"""; exec(code)'
	@cd $(RUN_TESTS_DIR) && \
	AGERUN_MEMORY_REPORT="memory_report_ar_complete_instruction_evaluator_tests.log" \
	AGERUN_COMPLETE_EVALUATOR_SUBTEST=performance_warm_fixture_set \
	./ar_complete_instruction_evaluator_tests

# Show vendored llama.cpp configuration
print-llama-config:
	@echo "CMAKE=$(CMAKE)"
	@echo "LLAMA_SOURCE_DIR=$(LLAMA_SOURCE_DIR)"
	@echo "LLAMA_BUILD_DIR=$(LLAMA_BUILD_DIR)"
	@echo "LLAMA_INSTALL_DIR=$(LLAMA_INSTALL_DIR)"
	@echo "LLAMA_SOURCE_HEADER=$(LLAMA_SOURCE_HEADER)"
	@echo "LLAMA_HEADER=$(LLAMA_HEADER)"
	@echo "LLAMA_LIB=$(LLAMA_LIB)"
	@echo "LLAMA_LICENSE=$(LLAMA_LICENSE)"
	@if [ -n "$(LLAMA_AVAILABLE)" ]; then \
		echo "LLAMA_AVAILABLE=yes"; \
	else \
		echo "LLAMA_AVAILABLE=no"; \
	fi

# Build/install vendored CPU-only llama.cpp
vendor-llama-cpu:
	@if [ ! -f "$(LLAMA_SOURCE_DIR)/CMakeLists.txt" ]; then \
		echo "ERROR: vendored llama.cpp source not found at $(LLAMA_SOURCE_DIR)"; \
		echo "Add the pinned upstream source tree under $(LLAMA_SOURCE_DIR) before running this target"; \
		exit 1; \
	fi
	@if [ ! -f "$(LLAMA_LICENSE)" ]; then \
		echo "ERROR: upstream llama.cpp license file missing at $(LLAMA_LICENSE)"; \
		echo "Keep the upstream MIT license text in the vendored source tree to preserve redistribution compliance"; \
		exit 1; \
	fi
	@if ! command -v "$(CMAKE)" >/dev/null 2>&1; then \
		echo "ERROR: cmake is required for make vendor-llama-cpu"; \
		echo "Set CMAKE=/absolute/path/to/cmake or install cmake before building the vendored dependency"; \
		exit 1; \
	fi
	$(CMAKE) -S "$(LLAMA_SOURCE_DIR)" -B "$(LLAMA_BUILD_DIR)" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$(abspath $(LLAMA_INSTALL_DIR))" -DBUILD_SHARED_LIBS=ON $(LLAMA_CPU_CMAKE_FLAGS)
	$(CMAKE) --build "$(LLAMA_BUILD_DIR)" --config Release
	$(CMAKE) --install "$(LLAMA_BUILD_DIR)" --config Release
	@if [ "$(UNAME_S)" = "Darwin" ]; then \
		for dylib in "$(LLAMA_INSTALL_DIR)"/lib/*.dylib; do \
			install_name_tool -add_rpath @loader_path "$$dylib" 2>/dev/null || true; \
			install_name_tool -add_rpath @loader_path/../lib "$$dylib" 2>/dev/null || true; \
		done; \
		for exe in "$(LLAMA_INSTALL_DIR)"/bin/*; do \
			if [ -f "$$exe" ]; then \
				install_name_tool -add_rpath @loader_path/../lib "$$exe" 2>/dev/null || true; \
			fi; \
		done; \
	fi

# Fix command documentation structure to match comprehensive standards
# Usage: make fix-commands
fix-commands:
	@python3 scripts/fix_commands.py

# Check KB article cross-references and links
# Usage: make check-kb-links
check-kb-links:
	@python3 scripts/check_kb_links.py

# Verify KB integration quality (cross-refs, command updates, etc)
# Usage: make check-kb-integration
check-kb-integration:
	@python3 scripts/check_kb_integration.py
