# Make-Only Test Execution

## Learning
Always use make targets for building and running tests, never compile directly with gcc/zig or run binaries directly. The Makefile handles dependencies, flags, environment setup, and ensures consistent execution. This is a strict discipline that must be followed even when it seems unnecessary.

## Importance
Direct compilation or execution bypasses critical setup: missing flags, wrong working directory, outdated dependencies, incorrect linking. This leads to mysterious failures and wasted debugging time. Even experienced developers must resist the temptation to run binaries directly.

## Example
```bash
# WRONG: Direct execution (even if it seems to work)
./bin/run-tests/method_creator_tests  # May fail with directory issues
./bin/run-tests/ar_instruction_parser_dlsym_tests  # Bypasses make orchestration

# WRONG: Direct compilation  
gcc -o test method_creator_tests.c -lar  # Missing critical flags and dependencies
zig build-exe test.zig  # Wrong flags, no dependency tracking

# CORRECT: Always use make
make method_creator_tests 2>&1  # Builds and runs with proper setup
make ar_instruction_parser_dlsym_tests 2>&1  # Handles all dlsym test needs

# The Makefile handles:
# 1. Rebuilding changed dependencies
# 2. Correct compilation flags (-DDEBUG, -I./modules, etc.)
# 3. Proper linking order
# 4. Running from correct directory (bin/)
# 5. Memory report generation
# 6. Parallel build isolation

# Even for debugging, use make targets
make method_creator_tests 2>&1 | grep "DEBUG"
make method_creator_tests 2>&1 | tail -50
```

## The Discipline

This is not just a guideline - it's a strict discipline:
1. **No exceptions**: Even if you "know" the binary is up-to-date
2. **Resist shortcuts**: The few seconds saved aren't worth the risk
3. **Interrupt reminders**: Expect to be interrupted if you try direct execution
4. **Consistency matters**: Following this discipline prevents subtle bugs

## Why Make Is Essential

Make provides essential build orchestration:
- **Dependencies**: Automatically rebuilds changed modules
- **Flags**: Consistent compilation flags across all targets
- **Environment**: Runs tests from correct working directory
- **Reports**: Generates memory leak reports automatically
- **Isolation**: Parallel builds use separate directories
- **Linking**: Correct library order and paths
- **Sanitizers**: Proper exclusion of incompatible tests (dlsym)

Never bypass make, even when debugging specific issues.

## Implementation
```bash
# Common make patterns for debugging

# Run specific test with full output
make test_name 2>&1

# Run and filter output
make test_name 2>&1 | grep "pattern"

# Run and check memory report
make test_name 2>&1
cat bin/run-tests/memory_report_test_name.log

# Run with sanitizers
make sanitize-tests  # All tests with ASan
make test_name  # Then check sanitizer output

# Check what make will do
make -n test_name  # Dry run to see commands

# Build individual module
make modules/ar_agent.o  # Rebuilds just this module

# Clean and rebuild
make clean test_name 2>&1
```

## Related Patterns
- [Development Debug Tools](development-debug-tools.md)
- [Build Verification Before Commit](build-verification-before-commit.md)
- [TDD Cycle Completion Verification Pattern](tdd-cycle-completion-verification-pattern.md)
- [Makefile Pattern Rule Management](makefile-pattern-rule-management.md)
- [Test Build Before Run Pattern](test-build-before-run-pattern.md)
- [Integration Test Binary Execution](integration-test-binary-execution.md)
- [Parallel Test Isolation with Process-Based Resource Copying](parallel-test-isolation-process-resources.md) - Advanced pattern for parallel test resource isolation