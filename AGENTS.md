# AgeRun Agent Development Guide

Read the guidelines in the CLAUDE.md file, and follow them strictly.

## Build Tool Requirements

### Static Analysis with scan-build

The AgeRun build system uses `scan-build` for static analysis to detect potential bugs and memory issues. This tool is part of the clang-tools package and must be installed on your system.

## Available Make Targets

### Build Targets
- `make` - Show help message with all available targets
- `make clean` - Remove all build artifacts
- `make build` - Run complete build with all checks and tests

### Test Targets
- `make run-tests` - Run all tests
- `make sanitize-tests` - Run tests with AddressSanitizer + UBSan
- `make tsan-tests` - Run tests with ThreadSanitizer
- `make <test_name>` - Build and run a single test (e.g. make ar_data_tests)

### Analysis Targets (require scan-build)
- `make analyze-exec` - Run static analysis on executable code
- `make analyze-tests` - Run static analysis on tests
- `make check-naming` - Check naming conventions
- `make check-docs` - Check documentation validity
- `make check-all` - Run all code quality checks

### Run Targets
- `make run-exec` - Build and run the executable
- `make sanitize-exec` - Build and run executable with sanitizers
- `make tsan-exec` - Build and run executable with ThreadSanitizer

### Utility Targets
- `make add-newline FILE=<file>` - Add newline to end of file if missing

The following targets require scan-build:
- `make analyze-exec` - Runs static analysis on executable code
- `make analyze-tests` - Runs static analysis on test files
- `make build` - Includes static analysis as part of the build verification

If you encounter errors like "scan-build: command not found" when running these commands, you need to install clang-tools:

- **macOS**: Install via Homebrew with `brew install llvm`
- **Ubuntu/Debian**: Install with `apt-get update && apt-get install -y clang-tools`

For detailed installation instructions and PATH configuration, see the **"Installing clang-tools (for Static Analysis)"** section in CLAUDE.md.