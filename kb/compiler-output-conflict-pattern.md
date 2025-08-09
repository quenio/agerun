# Compiler Output Conflict Pattern

## Learning
When running parallel builds with different compilers (gcc and clang), sharing the same build output directories causes linker errors like "invalid control bits in libagerun.a" because object files from different compilers are incompatible.

## Importance
- Parallel test execution with different sanitizers requires different compilers
- Mixing object files from gcc and clang creates corrupted archives
- Build system must isolate compiler outputs to prevent conflicts
- Critical for CI/CD pipelines running multiple test configurations

## Example
```c
// Problem: Shared build directory causes conflicts
// Thread Sanitizer tests (clang) writing to same directory as regular tests (gcc)

// Error manifestation:
// ld: archive member 'ar_build_instruction_evaluator.o' not a mach-o file in 'bin/run-tests/libagerun.a'
// ld: archive member invalid control bits in 'bin/run-exec/libagerun.a'

// Solution: Isolate build directories per test run
struct ar_executable_fixture_s {
    char temp_build_dir[256];  /* Unique build directory per test run */
    bool initialized;
};

static void _init_temp_build_dir(ar_executable_fixture_t *mut_fixture) {
    if (!mut_fixture->initialized) {
        pid_t pid = getpid();
        snprintf(mut_fixture->temp_build_dir, sizeof(mut_fixture->temp_build_dir), 
                 "/tmp/agerun_test_%d_build", (int)pid);
        
        // Create isolated build directory with obj subdirectory
        char mkdir_cmd[512];
        snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s/obj", mut_fixture->temp_build_dir);
        system(mkdir_cmd);
        
        mut_fixture->initialized = true;
    }
}

// Use isolated directory in build commands
FILE* ar_executable_fixture__build_and_run(const ar_executable_fixture_t *ref_fixture,
                                           const char *ref_methods_dir) {
    char build_cmd[2048];
    snprintf(build_cmd, sizeof(build_cmd),
        "cd ../.. && "
        "AGERUN_METHODS_DIR=%s "
        "RUN_EXEC_DIR=%s "  // Use fixture's isolated build directory
        "make run-exec 2>&1",
        ref_methods_dir, ref_fixture->temp_build_dir);
    
    return popen(build_cmd, "r");
}
```

## Generalization
Preventing compiler output conflicts:
1. **Identify Compiler Mixing**: Check if tests use different compilers (gcc vs clang)
2. **Isolate Build Directories**: Each compiler environment gets unique output directory
3. **Use Process IDs**: Create directories with PID to ensure uniqueness
4. **Environment Variables**: Pass isolated directories via environment variables
5. **Clean Up**: Remove temporary directories after test completion
6. **Archive Separation**: Never mix .o files from different compilers in same .a file

## Implementation
```bash
# Makefile pattern for isolated builds
tsan-tests: CC = clang
tsan-tests: BUILD_DIR = bin/tsan-tests
tsan-tests:
	mkdir -p $(BUILD_DIR)/obj
	$(MAKE) BUILD_DIR=$(BUILD_DIR) run-tests

sanitize-tests: CC = gcc-13  
sanitize-tests: BUILD_DIR = bin/sanitize-tests
sanitize-tests:
	mkdir -p $(BUILD_DIR)/obj
	$(MAKE) BUILD_DIR=$(BUILD_DIR) run-tests

# Test fixture pattern
export BUILD_DIR="/tmp/test_$$_build"
mkdir -p "$BUILD_DIR/obj"
make BUILD_DIR="$BUILD_DIR" test-target
rm -rf "$BUILD_DIR"
```

## Related Patterns
- [Parallel Test Isolation Process Resources](parallel-test-isolation-process-resources.md)
- [Makefile Pattern Rule Management](makefile-pattern-rule-management.md)
- [Test Working Directory Verification](test-working-directory-verification.md)