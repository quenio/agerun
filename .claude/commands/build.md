Execute comprehensive build verification with minimal output and check for hidden issues.

## Command
```bash
make build 2>&1 && make check-logs
```

## Expected Output

### Success State
```
Building library...
Running all tests...
All 68 tests passed! (1126 assertions)
Build took 1m 24s
========================================
         BUILD SUCCESSFUL! ✓
========================================
Running log checks...
✅ No errors found in logs - CI ready!
```

### Failure States

**Compilation Error:**
```
Building library...
modules/ar_data.c:123: error: expected ';' before '}' token
make: *** [build] Error 1
```

**Test Failure:**
```
Building library...
Running all tests...
FAILED: ar_string_tests
  Expected: "hello"
  Actual: "world"
67 of 68 tests passed
make: *** [build] Error 1
```

**Log Check Failure:**
```
Building library...
Running all tests...
All 68 tests passed!
Build took 1m 24s
========================================
         BUILD SUCCESSFUL! ✓
========================================
Running log checks...
❌ Found 3 errors in logs:
  - ERROR: Method evaluation failed (in test: ar_method_evaluator_tests)
  - ERROR: Assertion failed at line 45
  - ERROR: Memory leak detected
Fix these issues before pushing to CI.
```

## Key Points

- **Always use `2>&1`** to capture stderr for complete error visibility
- **Always follow with `make check-logs`** to catch hidden issues
- Logs are only updated during build, not by check-logs ([details](../../kb/build-logs-relationship-principle.md))
- The build must pass check-logs or CI will fail ([details](../../kb/ci-check-logs-requirement.md))
- Build time varies: ~20s for incremental, ~1m 24s for full build 

The Makefile uses generic parameterized targets to avoid duplication ([details](../../kb/generic-make-targets-pattern.md)). Commands should document expected outputs for clarity ([details](../../kb/command-output-documentation-pattern.md)).

**Important**: The build system runs parallel tests with different compilers (gcc for regular tests, clang for Thread Sanitizer). Each uses isolated build directories to prevent conflicts ([details](../../kb/compiler-output-conflict-pattern.md)).

**Exit Code Handling**: When using popen() to run processes, remember that exit codes are multiplied by 256 ([details](../../kb/exit-code-propagation-popen.md)). Non-critical operations should follow graceful degradation patterns ([details](../../kb/graceful-degradation-pattern.md)).