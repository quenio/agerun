# AgeRun Agent Development Guide

Read the guidelines in the CLAUDE.md file, and follow them strictly.

## Build Tool Requirements

### Static Analysis with scan-build

The AgeRun build system uses `scan-build` for static analysis to detect potential bugs and memory issues. This tool is part of the clang-tools package and must be installed on your system.

The following Makefile targets require scan-build:
- `make analyze` - Runs static analysis on the library
- `make analyze-tests` - Runs static analysis on the test files
- `make full-build` - Includes static analysis as part of the full build verification

If you encounter errors like "scan-build: command not found" when running these commands, you need to install clang-tools:

- **macOS**: Install via Homebrew with `brew install llvm`
- **Ubuntu/Debian**: Install with `apt-get update && apt-get install -y clang-tools`

For detailed installation instructions and PATH configuration, see the **"Installing clang-tools (for Static Analysis)"** section in CLAUDE.md.