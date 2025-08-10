# Parallel Test Isolation with Process-Based Resource Copying

## Learning
Race conditions in parallel test execution can be eliminated by creating isolated environments using process IDs for unique temporary directories and copying shared resources per test process, preventing tests from interfering with each other's state.

## Importance
Parallel test execution dramatically speeds up CI/CD pipelines, but shared resources between tests create race conditions that cause intermittent failures. This pattern provides complete isolation without sacrificing parallel execution speed.

## Example
```c
// Helper function for isolated test execution
static FILE* _build_and_run_executable(void) {
    char build_cmd[1024];
    pid_t pid = getpid();
    
    // Create unique environment per process
    snprintf(build_cmd, sizeof(build_cmd), 
        "cd ../.. && "
        "rm -rf /tmp/agerun_test_%d 2>/dev/null && "
        "mkdir -p /tmp/agerun_test_%d/obj && "
        "mkdir -p /tmp/agerun_test_%d/methods && "
        "cp methods/* /tmp/agerun_test_%d/methods/ && "
        "AGERUN_METHODS_DIR=/tmp/agerun_test_%d/methods "
        "RUN_EXEC_DIR=/tmp/agerun_test_%d make run-exec 2>&1", 
        (int)pid, (int)pid, (int)pid, (int)pid, (int)pid, (int)pid);
    return popen(build_cmd, "r");
}

// Cleanup function
static void _cleanup_build_directory(void) {
    char cleanup_cmd[256];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), 
        "rm -rf /tmp/agerun_test_%d 2>/dev/null", 
        (int)getpid());
    system(cleanup_cmd);
}
```

## Generalization
Pattern for parallel test isolation:
1. **Use Process ID**: Create unique directories with `getpid()` to ensure no collisions
2. **Copy Resources**: Each test gets its own copy of shared files/directories
3. **Environment Variables**: Configure applications to use isolated paths
4. **Build Override**: Use environment variables to redirect build outputs
5. **Cleanup**: Remove temporary resources after test completion

## Implementation
```bash
# Key environment variables for isolation
export RESOURCE_DIR="/tmp/test_$(getpid())/resources"
export BUILD_DIR="/tmp/test_$(getpid())/build"
export OUTPUT_DIR="/tmp/test_$(getpid())/output"

# Setup isolated environment
mkdir -p "$BUILD_DIR/obj"
cp -r shared_resources/* "$RESOURCE_DIR/"

# Run with isolation
APPLICATION_CONFIG="$RESOURCE_DIR/config" \
OUTPUT_PATH="$OUTPUT_DIR" \
make test-target

# Cleanup
rm -rf "/tmp/test_$(getpid())"
```

Root cause addressed: Tests that modify shared resources (files, directories, configuration) create race conditions when multiple tests run simultaneously. Solution: Give each test process its own complete copy of all resources it might modify.

## Race Condition Scenarios Solved
- **File Modification**: Tests renaming/moving files affect other tests
- **Configuration Changes**: Tests modifying shared config files
- **Build Output Conflicts**: Multiple tests writing to same directories
- **State Persistence**: Tests leaving state that affects subsequent tests

## Knowledge Application Gap
Despite having [Evidence-Based Debugging](evidence-based-debugging.md) guidance emphasizing "gather hard evidence" and "don't assume," the systematic approach was abandoned in favor of quick fixes. This reveals the gap between having documented patterns and applying them under pressure.

**Why existing guidance wasn't followed:**
- **Urgency bias**: Pressure to fix quickly led to assumption-driven debugging
- **Evidence avoidance**: Made assumptions about race conditions instead of examining actual error messages and test outputs systematically  
- **Pattern blindness**: Focus on symptoms (test failures) instead of root cause investigation
- **Missing consultation**: Didn't reference existing debugging articles during active problem-solving

**Critical failure**: Repeatedly theorized about Makefile issues instead of examining concrete evidence like build output, directory states, and actual error messages. The user had to repeatedly demand "hard evidence" to redirect from assumption-based to evidence-based debugging.

**Prevention**: Before debugging complex issues, explicitly review relevant KB articles and commit to following documented patterns.

## Related Patterns
- [Integration Test Binary Execution](integration-test-binary-execution.md)
- [Test File Cleanup Pattern](test-file-cleanup-pattern.md)
- [Test Isolation Through Commenting](test-isolation-through-commenting.md)
- [Make-Only Test Execution](make-only-test-execution.md)
- [Test Working Directory Verification](test-working-directory-verification.md)
- [Evidence-Based Debugging](evidence-based-debugging.md) - Should have been consulted early
- [User Feedback Debugging Pattern](user-feedback-debugging-pattern.md) - Emphasizes accepting challenges to assumptions
- [Compiler Output Conflict Pattern](compiler-output-conflict-pattern.md) - Specific compiler mixing issues
- [Test Fixture Module Creation Pattern](test-fixture-module-creation-pattern.md) - Proper fixture design